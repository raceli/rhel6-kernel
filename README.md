RHEL6 Kernel - v2.6.32.x
============

Detailed changes between RHEL kernel releases

* Sun Jul 27 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.29.2.el6]
- [kernel] futex: Fix errors in nested key ref-counting (Denys Vlasenko) [1094457 1094458] {CVE-2014-0205}
- [net] vxlan: fix NULL pointer dereference (Jiri Benc) [1114549 1096351] {CVE-2014-3535}

* Sun Jul 27 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.29.1.el6]
- [mm] hugetlb: ensure hugepage access is denied if hugepages are not supported (Gustavo Duarte) [1118782 1086450]
- [security] keys: Increase root_maxkeys and root_maxbytes sizes (Steve Dickson) [1115542 1113607]
- [fs] lockd: Ensure that nlmclnt_block resets block->b_status after a server reboot (Steve Dickson) [1110180 959006]
- [net] filter: add vlan tag access (Jiri Benc) [1108526 1082097]
- [net] filter: add XOR operation (Jiri Benc) [1108526 1082097]
- [net] filter: add SKF_AD_RXHASH and SKF_AD_CPU (Jiri Benc) [1108526 1082097]
- [net] filter: Socket filter ancilliary data access for skb->dev->type (Jiri Benc) [1108526 1082097]
- [net] filter: Add SKF_AD_QUEUE instruction (Jiri Benc) [1108526 1082097]
- [net] filter: ingress socket filter by mark (Jiri Benc) [1108526 1082097]
- [netdrv] bonding: look for bridge IPs in arp monitoring (Veaceslav Falico) [1102794 704190]
- [s390] af_iucv: wrong mapping of sent and confirmed skbs (Hendrik Brueckner) [1112390 1102248]
- [s390] af_iucv: recvmsg problem for SOCK_STREAM sockets (Hendrik Brueckner) [1112390 1102248]
- [s390] af_iucv: fix recvmsg by replacing skb_pull() function (Hendrik Brueckner) [1112390 1102248]
- [s390] kernel: avoid page table walk on user space access (Hendrik Brueckner) [1111194 1099146]
- [s390] qeth: postpone freeing of qdio memory (Hendrik Brueckner) [1112134 1094379]
- [s390] qeth: Fix retry logic in hardsetup (Hendrik Brueckner) [1112134 1094379]
- [s390] qeth: Recognize return codes of ccw_device_set_online (Hendrik Brueckner) [1112134 1094379]
- [s390] qdio: remove API wrappers (Hendrik Brueckner) [1112134 1094379]
- [scsi] Ensure medium access timeout counter resets (David Jeffery) [1117153 1036884]
- [scsi] Fix error handling when no ULD is attached (David Jeffery) [1117153 1036884]
- [scsi] Handle disk devices which can not process medium access commands (David Jeffery) [1117153 1036884]
- [fs] nfs: Fix calls to drop_nlink() (Steve Dickson) [1099607 1093819]
- [mm] swap: do not skip lowest_bit in scan_swap_map() scan loop (Rafael Aquini) [1099728 1060886]
- [mm] swap: fix shmem swapping when more than 8 areas (Rafael Aquini) [1099728 1060886]
- [mm] swap: fix swapon size off-by-one (Rafael Aquini) [1099728 1060886]
- [md] avoid deadlock when dirty buffers during md_stop (Jes Sorensen) [1121541 994724]
- [x86] hyperv: bypass the timer_irq_works() check (Jason Wang) [1112226 1040349]

* Wed Jul 23 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.28.1.el6]
- [kernel] auditsc: audit_krule mask accesses need bounds checking (Denys Vlasenko) [1102704 1102705] {CVE-2014-3917}
- [net] ipv4: fix route cache rebuilds (Jiri Pirko) [1113824 1111631]
- [fs] nfsd: notify_change needs elevated write count (Mateusz Guzik) [1110177 1105057]
- [fs] nfsv4: close needs to handle NFS4ERR_ADMIN_REVOKED (Dave Wysochanski) [1096397 1082127]
- [fs] pipe: skip file_update_time on frozen fs (Eric Sandeen) [1114405 1093077]
- [fs] nfs: Fail the truncate() if the lock/open stateid is invalid (Steve Dickson) [1090613 1075123]
- [fs] nfs: Servers should only check SETATTR stateid open mode on size change (Steve Dickson) [1090613 1075123]
- [fs] nfs: Fail data server I/O if stateid represents a lost lock (Steve Dickson) [1090613 1075123]
- [fs] nfs: Fix the return value of nfs4_select_rw_stateid (Steve Dickson) [1090613 1075123]
- [fs] nfs: Use the open stateid if the delegation has the wrong mode (Steve Dickson) [1090613 1075123]
- [fs] nfs: nfs4_stateid_is_current should return 'true' for an invalid stateid (Steve Dickson) [1090613 1075123]
- [fs] nfs: fix error return in nfs4_select_rw_stateid (Steve Dickson) [1090613 1075123]
- [fs] nfs: Document the recover_lost_locks kernel parameter (Jeff Layton) [1089359 963785]
- [fs] nfs: Don't try to recover NFSv4 locks when they are lost (Jeff Layton) [1089359 963785]
- [fs] nfs: Fix handling of partially delegated locks (Jeff Layton) [1120074 959788]
- [fs] nfs: Convert the nfs4_lock_state->ls_flags to a bit field (Jeff Layton) [1120074 959788]
- [x86] Optimize switch_mm() for multi-threaded workloads (Rik van Riel) [1115821 991518]
- [netdrv] pppol2tp: fail when socket option level is not SOL_PPPOL2TP [1119461 1119462] {CVE-2014-4943}
- [kernel] utrace: force IRET path after utrace_finish_vfork() (Oleg Nesterov) [1115932 1115933] {CVE-2014-4699}

* Thu Jul 17 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.27.1.el6]
- [scsi] fix performance regression due to inverted blk_get_queue return (Mike Snitzer) [1117582 1098658]
- [net] openvswitch: fix use-after-free bug in netns (Flavio Leitner) [1120651 1100127]

* Mon Jul 14 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.26.1.el6]
- [net] gro: fix deliver of trunk packets to VLAN interfaces (Marcelo Ricardo Leitner) [1116231 1112324]

* Mon Jul 07 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.25.1.el6]
- [net] sctp: Fix sk_ack_backlog wrap-around problem (Daniel Borkmann) [1113969 1085932] {CVE-2014-4667}

* Wed Jun 25 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.24.1.el6]
- [alsa] aloop: Close races at restarting the stream (Jaroslav Kysela) [1112492 1078592]
- [alsa] aloop: Export snd_pcm_constraint_mask64() (Jaroslav Kysela) [1112492 1078592]
- [alsa] pcm: Warn when buffer preallocation fails (Jaroslav Kysela) [1112492 1078592]
- [alsa] aloop: Add SNDRV_PCM_STATE_PAUSED case in wait_for_avail function (Jaroslav Kysela) [1112492 1078592]
- [alsa] jack: Unregister input device at disconnection (Jaroslav Kysela) [1112492 1078592]
- [alsa] aloop: Optimize module name check (Jaroslav Kysela) [1112492 1078592]
- [alsa] pcm: Add fallthru comments (Jaroslav Kysela) [1112492 1078592]
- [alsa] aloop: Fix Oops while PM resume (Jaroslav Kysela) [1112492 1078592]
- [alsa] aloop: add locking to timer access (Jaroslav Kysela) [1112492 1078592]

http://j.mp/1suvdKv

Latest available is [linux-2.6.32-431.23.3](https://github.com/cybernet/rhel6-kernel/releases/tag/2.6.32-431.29.2)
