/*
 *  linux/fs/ext4/fsync.c
 *
 *  Copyright (C) 1993  Stephen Tweedie (sct@redhat.com)
 *  from
 *  Copyright (C) 1992  Remy Card (card@masi.ibp.fr)
 *                      Laboratoire MASI - Institut Blaise Pascal
 *                      Universite Pierre et Marie Curie (Paris VI)
 *  from
 *  linux/fs/minix/truncate.c   Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  ext4fs fsync primitive
 *
 *  Big-endian to little-endian byte-swapping/bitmaps by
 *        David S. Miller (davem@caip.rutgers.edu), 1995
 *
 *  Removed unnecessary code duplication for little endian machines
 *  and excessive __inline__s.
 *        Andi Kleen, 1997
 *
 * Major simplications and cleanup - we only need to do the metadata, because
 * we can depend on generic_block_fdatasync() to sync the data blocks.
 */

#include <linux/time.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <linux/jbd2.h>
#include <linux/blkdev.h>

#include "ext4.h"
#include "ext4_jbd2.h"

#include <trace/events/ext4.h>

/*
 * akpm: A new design for ext4_sync_file().
 *
 * This is only called from sys_fsync(), sys_fdatasync() and sys_msync().
 * There cannot be a transaction open by this task.
 * Another task could have dirtied this inode.  Its data can be in any
 * state in the journalling system.
 *
 * What we do is just kick off a commit and wait on it.  This will snapshot the
 * inode to disk.
 *
 * i_mutex lock is held when entering and exiting this function
 */

int ext4_sync_file(struct file *file, struct dentry *dentry, int datasync)
{
	struct inode *inode = dentry->d_inode;
	struct ext4_inode_info *ei = EXT4_I(inode);
	journal_t *journal = EXT4_SB(inode->i_sb)->s_journal;
	int ret, err = 0;
	tid_t commit_tid;
	tid_t flush_tid;
	bool needs_barrier = false;

	J_ASSERT(ext4_journal_current_handle() == NULL);

	trace_ext4_sync_file(file, dentry, datasync);
	percpu_counter_inc(&EXT4_SB(inode->i_sb)->s_fsync_counter);

	if (inode->i_sb->s_flags & MS_RDONLY) {
		/* Make shure that we read updated s_mount_flags value */
		smp_rmb();
		if (EXT4_SB(inode->i_sb)->s_mount_flags & EXT4_MF_FS_ABORTED)
			return -EROFS;
		return 0;
	}

	ret = flush_aio_dio_completed_IO(inode);
	if (ret < 0)
		return ret;

	if (!journal)
		return simple_fsync(file, dentry, datasync);

	/*
	 * data=writeback,ordered:
	 *  The caller's filemap_fdatawrite()/wait will sync the data.
	 *  Metadata is in the journal, we wait for proper transaction to
	 *  commit here.
	 *
	 * data=journal:
	 *  filemap_fdatawrite won't do anything (the buffers are clean).
	 *  ext4_force_commit will write the file data into the journal and
	 *  will wait on that.
	 *  filemap_fdatawait() will encounter a ton of newly-dirtied pages
	 *  (they were dirtied by commit).  But that's OK - the blocks are
	 *  safe in-journal, which is all fsync() needs to ensure.
	 */
	if (ext4_should_journal_data(inode))
		return ext4_force_commit(inode->i_sb);

	flush_tid = journal->j_commit_sequence;

	commit_tid = datasync ? ei->i_datasync_tid : ei->i_sync_tid;
	if (journal->j_flags & JBD2_BARRIER &&
	    !jbd2_trans_will_send_data_barrier(journal, commit_tid))
		needs_barrier = true;
	ret = jbd2_complete_transaction(journal, commit_tid);

	/* Even if we had to wait for commit completion, it does not mean a flush has been
	 * issued after data demanded by this fsync were written back. Commit could be in state
	 * after it is already done, but not yet in state where we should not wait.
	 */
	if (needs_barrier && !tid_gt(journal->j_commit_sequence, flush_tid))
		err = blkdev_issue_flush(inode->i_sb->s_bdev, NULL);
	if (!ret)
		ret = err;
	return ret;
}

int ext4_sync_files(struct file **files, unsigned int *flags, unsigned int nr_files)
{
	struct super_block *sb;
	journal_t *journal;
	int err = 0, err2 = 0, i = 0, j = 0;
	int force_commit = 0, datawriteback = 0;
	tid_t commit_tid = 0;
	tid_t flush_tid;
	int fdsync_cnt = 0, fsync_cnt = 0;
	int need_barrier = 0;
	struct user_beancounter *ub;

	J_ASSERT(ext4_journal_current_handle() == NULL);
	if (!nr_files)
		return 0;

	sb = files[0]->f_mapping->host->i_sb;
	journal = EXT4_SB(sb)->s_journal;
	ub = get_exec_ub();
	if (sb->s_flags & MS_RDONLY) {
		/* Make shure that we read updated s_mount_flags value */
		smp_rmb();
		if (EXT4_SB(sb)->s_mount_flags & EXT4_MF_FS_ABORTED)
			return -EROFS;
		return 0;
	}
	for (i = 0; i < nr_files; i++) {
		struct address_space * mapping = files[i]->f_mapping;
		struct inode *inode = mapping->host;
		unsigned int datasync = flags[i];

		BUG_ON(sb != inode->i_sb);
		trace_ext4_sync_file(files[i], files[i]->f_path.dentry, flags[i]);

		if (datasync) {
			ub_percpu_inc(ub, fdsync);
			fdsync_cnt++;
		} else {
			ub_percpu_inc(ub, fsync);
			fsync_cnt++;
		}

		if (!mapping->nrpages)
			continue;

		err = filemap_fdatawrite(mapping);
		if (err)
			break;
	}
	/*
	 * Even if the above returned error, the pages may be
	 * written partially (e.g. -ENOSPC), so we wait for it.
	 * But the -EIO is special case, it may indicate the worst
	 * thing (e.g. bug) happened, so we avoid waiting for it.
	 */
	if (err == -EIO)
		goto out;

	for (j = 0; j < i; j++) {
		struct address_space * mapping = files[j]->f_mapping;
		struct inode *inode = mapping->host;
		struct ext4_inode_info *ei = EXT4_I(inode);
		unsigned int datasync = flags[j];
		tid_t tid;

		if (mapping->nrpages) {
			err2 = filemap_fdatawait(mapping);
			if (!err || err2 == -EIO)
				err = err2;
		}

		mutex_lock(&inode->i_mutex);
		err2 = flush_aio_dio_completed_IO(inode);
		if (!err || err2 == -EIO)
			err = err2;
		force_commit  |= ext4_should_journal_data(inode);
		datawriteback |= ext4_should_writeback_data(inode);
		tid = datasync ? ei->i_datasync_tid : ei->i_sync_tid;
		mutex_unlock(&inode->i_mutex);
		trace_ext4_sync_files_iterate(files[j]->f_path.dentry, tid, datasync);
		if (j == 0 || !tid_geq(commit_tid, tid))
			commit_tid = tid;
	}

	/* Ext4 specific stuff starts here */
	if (!journal) {
		for (j = 0; i < i; j++) {
			err2 = simple_fsync(files[j], files[j]->f_path.dentry, flags[j]);
			if (!err)
				err = err2;
			j++;
		}
	} else if (force_commit) {
		/* data=journal:
		 *  filemap_fdatawrite won't do anything (the buffers are clean).
		 *  ext4_force_commit will write the file data into the journal and
		 *  will wait on that.
		 *  filemap_fdatawait() will encounter a ton of newly-dirtied pages
		 *  (they were dirtied by commit).  But that's OK - the blocks are
		 *  safe in-journal, which is all fsync() needs to ensure.
		 */
		err2 = ext4_force_commit(sb);
	} else {
		/*
		 * data=writeback,ordered:
		 * The caller's filemap_fdatawrite()/wait will sync the data.
		 * Metadata is in the journal, we wait for proper transaction to
		 * commit here.
		 */
		flush_tid = journal->j_commit_sequence;
		if (jbd2_log_start_commit(journal, commit_tid)) {
			/*
			 * When the journal is on a different device than the
			 * fs data disk, we need to issue the barrier in
			 * writeback mode.  (In ordered mode, the jbd2 layer
			 * will take care of issuing the barrier.  In
			 * data=journal, all of the data blocks are written to
			 * the journal device.)
			 */
			if (datawriteback &&
			    (journal->j_fs_dev != journal->j_dev) &&
			    (journal->j_flags & JBD2_BARRIER)) {
				err2 = blkdev_issue_flush(sb->s_bdev, NULL);
				if (!err)
					err = err2;
			}
			err2 = jbd2_log_wait_commit(journal, commit_tid);
			/* Even if we had to wait for commit completion, it does
			 * not mean a flush has been issued after data demanded
			 * by this fsync were written back. Commit could be in
			 * state  after it is already done, but not yet in state
			 * where we should not wait.
			 */
		}
		if ((journal->j_flags & JBD2_BARRIER) &&
		    !tid_gt(journal->j_commit_sequence, flush_tid)) {
			int err3;
			need_barrier = 1;
			err3 = blkdev_issue_flush(sb->s_bdev, NULL);
			if (!err)
				err = err3;
		}
	}
out:
	trace_ext4_sync_files_exit(files[0]->f_path.dentry, commit_tid, need_barrier);
	ub_percpu_add(ub, fdsync_done, fdsync_cnt);
	ub_percpu_add(ub, fsync_done, fsync_cnt);
	percpu_counter_add(&EXT4_SB(sb)->s_fsync_counter, i);
	if (!err || err2 == -EIO)
		err = err2;
	return err;
}
