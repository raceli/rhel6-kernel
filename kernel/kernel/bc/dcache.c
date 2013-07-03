#include <linux/slab.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/highmem.h>

#include <bc/beancounter.h>
#include <bc/vmpages.h>
#include <bc/dcache.h>
#include <bc/kmem.h>

static struct rb_root bc_dcache_root = RB_ROOT;

static unsigned int dcache_charge_size(int name_len)
{
	return dentry_cache->objuse + inode_cachep->objuse +
		(name_len > DNAME_INLINE_LEN ? name_len : 0);
}

int ub_dcache_shrink(struct user_beancounter *ub,
		unsigned long size, gfp_t gfp_mask)
{
	int count, pruned;

	if (!(gfp_mask & __GFP_FS))
		return -EBUSY;

	count = DIV_ROUND_UP(size, dcache_charge_size(0));
	spin_lock(&dcache_lock);
	pruned = __shrink_dcache_ub(ub, count, ub_dcache_lru_popup);
	spin_unlock(&dcache_lock);
	if (!pruned)
		return -ENOMEM;

	return 0;
}

static void __ub_dcache_insert(struct user_beancounter *ub)
{
	struct rb_node **new = &(bc_dcache_root.rb_node), *parent = NULL;

	while (*new) {
		struct user_beancounter *this;

		this = container_of(*new, struct user_beancounter, dc_node);
		parent = *new;
		if (ub->dc_time <= this->dc_time)
			new = &((*new)->rb_left);
		else
			new = &((*new)->rb_right);
	}

	rb_link_node(&ub->dc_node, parent, new);
	rb_insert_color(&ub->dc_node, &bc_dcache_root);
}

void ub_dcache_insert(struct user_beancounter *ub, unsigned int d_time)
{
	if (ub->ub_dentry_unused <= ub->ub_dcache_threshold) {
		if (list_empty(&ub->ub_dentry_lru))
			ub->dc_time = d_time;
		return;
	}

	if (RB_EMPTY_NODE(&ub->dc_node))
		__ub_dcache_insert(ub);
}

static inline void __ub_dcache_remove(struct user_beancounter *ub)
{
	rb_erase(&ub->dc_node, &bc_dcache_root);
}

static inline void ub_dcache_remove(struct user_beancounter *ub)
{
	if (!RB_EMPTY_NODE(&ub->dc_node)) {
		__ub_dcache_remove(ub);
		RB_CLEAR_NODE(&ub->dc_node);
	}
}

bool ub_dcache_shrinkable(void)
{
       return !RB_EMPTY_ROOT(&bc_dcache_root) || !ub_dcache_lru_popup;
}

struct user_beancounter *ub_dcache_next(void)
{
	struct rb_node *n;
	struct user_beancounter *bc;
	struct dentry *de;

again:
	/* FIXME, keep the leftmost pointer */
	n = bc_dcache_root.rb_node;
	if (!n)
		return NULL;

	while (n->rb_left != NULL)
		n = n->rb_left;

	bc = container_of(n, struct user_beancounter, dc_node);
	if (bc->ub_dentry_unused <= bc->ub_dcache_threshold) {
		ub_dcache_remove(bc);
		goto again;
	}

	de = list_entry(bc->ub_dentry_lru.prev, struct dentry, d_bclru);

	if (de->d_lru_time > bc->dc_time) {
		__ub_dcache_remove(bc);
		bc->dc_time = de->d_lru_time;
		__ub_dcache_insert(bc);
		goto again;
	}

	if (!get_beancounter_rcu(bc)) {
		ub_dcache_remove(bc);
		goto again;
	}

	bc->dc_shrink_ts = dcache_update_time();
	return bc;
}

static int __ub_dcache_charge(struct user_beancounter *ub,
		unsigned long size, gfp_t gfp_mask, int strict)
{
	int ret;

	ret = ub_kmem_charge(ub, size, gfp_mask);
	if (unlikely(ret))
		goto no_kmem;

	ret = charge_beancounter_fast(ub, UB_DCACHESIZE, size, strict);
	if (unlikely(ret))
		goto no_dcache;

	return 0;

no_dcache:
	ub_kmem_uncharge(ub, size);
no_kmem:
	return ret;
}

static void __ub_dcache_uncharge(struct user_beancounter *ub,
		unsigned long size)
{
	uncharge_beancounter_fast(ub, UB_DCACHESIZE, size);
	ub_kmem_uncharge(ub, size);
}

int ub_dcache_charge(struct user_beancounter *ub, int name_len)
{
	int size, shrink;

	if (ub_check_ram_limits(ub, GFP_KERNEL))
		return -ENOMEM;

	size = dcache_charge_size(name_len);
	do {
		if (!__ub_dcache_charge(ub, size,
					GFP_KERNEL|__GFP_NOWARN|__GFP_SOFT_UBC,
					UB_SOFT | UB_TEST))
			return 0;

		shrink = max(size, ub->ub_parms[UB_DCACHESIZE].max_precharge);
	} while (!ub_dcache_shrink(ub, shrink, GFP_KERNEL));

	spin_lock_irq(&ub->ub_lock);
	ub->ub_parms[UB_DCACHESIZE].failcnt++;
	spin_unlock_irq(&ub->ub_lock);

	return -ENOMEM;
}

void ub_dcache_uncharge(struct user_beancounter *ub, int name_len)
{
	unsigned int size;

	size = dcache_charge_size(name_len);
	__ub_dcache_uncharge(ub, size);
}

static unsigned long recharge_subtree(struct dentry *d, struct user_beancounter *ub,
		struct user_beancounter *cub)
{
	struct dentry *orig_root;
	unsigned long size = 0;

	orig_root = d;

	while (1) {
		if (d->d_ub != cub) {
			if (!(d->d_flags & DCACHE_BCTOP)) {
				printk("%s %s %d %d %d %p %p %p %p\n", __func__,
						d->d_name.name,
						d->d_ub->ub_uid,
						ub->ub_uid,
						cub->ub_uid,
						d, d->d_ub, ub, cub);
				WARN_ON(1);
			}
			goto skip_subtree;
		} else if (d->d_ub == ub)
			goto skip_recharge;

		if (!list_empty(&d->d_lru)) {
			ub_dcache_insert(ub, d->d_lru_time);
			list_move(&d->d_bclru, &ub->ub_dentry_lru);
			cub->ub_dentry_unused--;
			ub->ub_dentry_unused++;
		}

		d->d_ub = ub;
skip_recharge:
		size += dcache_charge_size(d->d_name.len);

		if (!list_empty(&d->d_subdirs)) {
			d = list_entry(d->d_subdirs.next,
					struct dentry, d_u.d_child);
			continue;
		}
skip_subtree:
		if (d == orig_root)
			break;
		while (d == list_entry(d->d_parent->d_subdirs.prev,
					struct dentry, d_u.d_child)) {
			d = d->d_parent;
			if (d == orig_root)
				goto out;
		}
		d = list_entry(d->d_u.d_child.next,
				struct dentry, d_u.d_child);
	}
out:
	return size;
}

unsigned long ub_dcache_get_size(struct dentry *dentry)
{
	unsigned long size;

	spin_lock(&dcache_lock);
	size = recharge_subtree(dentry, dentry->d_ub, dentry->d_ub);
	spin_unlock(&dcache_lock);

	return size;
}

void ub_dcache_set_owner(struct dentry *root, struct user_beancounter *ub)
{
	struct user_beancounter *cub;
	unsigned long size;

	spin_lock(&dcache_lock);

	cub = root->d_ub;
	if (ub != cub) {
		size = recharge_subtree(root, ub, cub);
		__ub_dcache_uncharge(cub, size);
		__ub_dcache_charge(ub, size, GFP_ATOMIC | __GFP_NOFAIL, UB_FORCE);
	}

	if (root->d_flags & DCACHE_BCTOP) {
		list_del(&root->d_bclru);
	} else {
		spin_lock(&root->d_lock);
		root->d_flags |= DCACHE_BCTOP;
		spin_unlock(&root->d_lock);
	}

	if (!list_empty(&root->d_lru)) {
		list_del_init(&root->d_lru);
		list_del(&root->d_bclru);
		root->d_sb->s_nr_dentry_unused--;
		cub->ub_dentry_unused--;
		dentry_stat.nr_unused--;
	}

	list_add_tail(&root->d_bclru, &ub->ub_dentry_top);

	spin_unlock(&dcache_lock);
}
EXPORT_SYMBOL(ub_dcache_set_owner);

void ub_dcache_change_owner(struct dentry *dentry, struct user_beancounter *ub)
{
	struct user_beancounter *cub = dentry->d_ub;
	long size;

	size = recharge_subtree(dentry, ub, cub);
	__ub_dcache_uncharge(cub, size);
	__ub_dcache_charge(ub, size, GFP_ATOMIC | __GFP_NOFAIL, UB_FORCE);
}

#define UB_DCACHE_BATCH 32

int ub_dcache_reclaim(struct user_beancounter *ub,
		unsigned long numerator, unsigned long denominator)
{
	unsigned long flags, batch;
	int ret = 1;

	if (ub->ub_dentry_unused <= ub->ub_dcache_threshold)
		return 0;

	spin_lock_irqsave(&ub->ub_lock, flags);
	batch = ub->ub_dentry_unused * numerator / denominator;
	/*
	 * Don't rise reclaimer's hopes if progress not guaranteed.
	 * It's better to kill someone instead of getting stuck forever.
	 */
	if (!batch)
		ret = 0;
	batch = ub->ub_dentry_batch = batch + ub->ub_dentry_batch;
	if (batch < UB_DCACHE_BATCH)
		batch = 0;
	else
		ub->ub_dentry_batch = 0;
	spin_unlock_irqrestore(&ub->ub_lock, flags);

	if (batch) {
		spin_lock(&dcache_lock);
		ret = __shrink_dcache_ub(ub, batch, ub_dcache_lru_popup);
		spin_unlock(&dcache_lock);
	}
	return ret;
}

/* under dcache_lock and dentry->d_lock */
void ub_dcache_clear_owner(struct dentry *dentry)
{
	struct user_beancounter *ub, *cub;
	long size;

	BUG_ON(!list_empty(&dentry->d_subdirs));
	BUG_ON(!(dentry->d_flags & DCACHE_BCTOP));

	cub = dentry->d_ub;
	ub = IS_ROOT(dentry) ? get_ub0() : dentry->d_parent->d_ub;
	dentry->d_ub = ub;

	size = dcache_charge_size(dentry->d_name.len);
	__ub_dcache_uncharge(cub, size);
	__ub_dcache_charge(ub, size, GFP_ATOMIC|__GFP_NOFAIL, UB_FORCE);

	dentry->d_flags &= ~DCACHE_BCTOP;

	list_del(&dentry->d_bclru);
}

void ub_dcache_unuse(struct user_beancounter *cub)
{
	struct dentry *dentry, *tmp;
	struct user_beancounter *ub;
	long size;

	spin_lock(&dcache_lock);
	ub_dcache_remove(cub);
	list_for_each_entry_safe(dentry, tmp, &cub->ub_dentry_top, d_bclru) {
		/* umount in progress */
		if (!atomic_read(&dentry->d_sb->s_active))
			continue;

		BUG_ON(dentry->d_ub != cub);
		ub = IS_ROOT(dentry) ? get_ub0() : dentry->d_parent->d_ub;

		size = recharge_subtree(dentry, ub, cub);
		__ub_dcache_uncharge(cub, size);
		__ub_dcache_charge(ub, size, GFP_ATOMIC|__GFP_NOFAIL, UB_FORCE);

		spin_lock(&dentry->d_lock);
		BUG_ON(!(dentry->d_flags & DCACHE_BCTOP));
		dentry->d_flags &= ~DCACHE_BCTOP;
		spin_unlock(&dentry->d_lock);

		list_del(&dentry->d_bclru);
	}
	spin_unlock(&dcache_lock);

	/* wait for concurrent umounts */
	while (!list_empty(&cub->ub_dentry_top))
		schedule_timeout_uninterruptible(1);

	BUG_ON(!list_empty(&cub->ub_dentry_lru));
}

void ub_update_threshold(struct user_beancounter *ub)
{
	unsigned long dsize, res;

	dsize = min(ub->ub_parms[UB_PHYSPAGES].limit,
		    totalram_pages - totalhigh_pages);

	res = dsize * ub_dcache_thres_ratio;
	if (unlikely(res < dsize))
		res = (dsize / 100) * ub_dcache_thres_ratio;
	else
		res /= 100;

	ub->ub_dcache_threshold = (res << PAGE_SHIFT) / dcache_charge_size(0);
}

static __init int ub_set_ub0_thres(void)
{
	printk("Dcache charge unit %u\n", dcache_charge_size(0));
	ub_update_threshold(get_ub0());
	return 0;
}
late_initcall(ub_set_ub0_thres);
