RHEL6 Kernel - v2.6.32.x
============

Detailed changes between RHEL kernel releases

* Fri Jun 06 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.20.3.el6]
- [kernel] futex: Make lookup_pi_state more robust (Jerome Marchand) [1104516 1104517] {CVE-2014-3153}
- [kernel] futex: Always cleanup owner tid in unlock_pi (Jerome Marchand) [1104516 1104517] {CVE-2014-3153}
- [kernel] futex: Validate atomic acquisition in futex_lock_pi_atomic() (Jerome Marchand) [1104516 1104517] {CVE-2014-3153}
- [kernel] futex: prevent requeue pi on same futex (Jerome Marchand) [1104516 1104517] {CVE-2014-3153}
- [fs] autofs4: fix device ioctl mount lookup (Ian Kent) [1069630 999708]
- [fs] vfs: introduce kern_path_mountpoint() (Ian Kent) [1069630 999708]
- [fs] vfs: rename user_path_umountat() to user_path_mountpoint_at() (Ian Kent) [1069630 999708]
- [fs] vfs: massage umount_lookup_last() a bit to reduce nesting (Ian Kent) [1069630 999708]
- [fs] vfs: allow umount to handle mountpoints without revalidating them (Ian Kent) [1069630 999708]
- Revert: [fs] vfs: allow umount to handle mountpoints without revalidating them (Ian Kent) [1069630 999708]
- Revert: [fs] vfs: massage umount_lookup_last() a bit to reduce nesting (Ian Kent) [1069630 999708]
- Revert: [fs] vfs: rename user_path_umountat() to user_path_mountpoint_at() (Ian Kent) [1069630 999708]
- Revert: [fs] vfs: introduce kern_path_mountpoint() (Ian Kent) [1069630 999708]
- Revert: [fs] autofs4: fix device ioctl mount lookup (Ian Kent) [1069630 999708]

* Thu May 15 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.20.2.el6]
- [block] floppy: don't write kernel-only members to FDRAWCMD ioctl output (Denys Vlasenko) [1094308 1094310] {CVE-2014-1738 CVE-2014-1737}
- [block] floppy: ignore kernel-only members in FDRAWCMD ioctl input (Denys Vlasenko) [1094308 1094310] {CVE-2014-1738 CVE-2014-1737}
- [fs] vfs: fix autofs/afs/etc magic mountpoint breakage (Frantisek Hrbata) [1094370 1079347] {CVE-2014-0203}
- [char] n_tty: Fix n_tty_write crash when echoing in raw mode (Aristeu Rozanski) [1094236 1094237] {CVE-2014-0196}

* Tue May 13 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.20.1.el6]
- [net] rtnetlink: Only supply IFLA_VF_PORTS information when RTEXT_FILTER_VF is set (Jiri Pirko) [1092870 1081282]
- [net] rtnetlink: Warn when interface's information won't fit in our packet (Jiri Pirko) [1092870 1081282]
- [net] bridge: Correctly receive hw-accelerated vlan traffic (Vlad Yasevich) [1096214 1067722]
- [net] vlan: Allow accelerated packets to flow through the bridge (Vlad Yasevich) [1096214 1067722]
- [infiniband] qib: Add missing serdes init sequence (Doug Ledford) [1080104 1005491]
- [infiniband] qib: Fix txselect regression (Doug Ledford) [1080104 1005491]
- [netdrv] ixgbevf: fix vlan acceleration (Nikolay Aleksandrov) [1094287 1069028]
- [security] selinux: Fix kernel BUG on empty security contexts (Paul Moore) [1062502 1064545] {CVE-2014-1874}
- [netdrv] libertas: potential oops in debugfs (Denys Vlasenko) [1034176 1034177] {CVE-2013-6378}
- [kernel] cgroup: move put_css_set() after setting CGRP_RELEASABLE bit to fix notify_on_release (Naoya Horiguchi) [1081909 1037465]
- [kernel] sched: Use exit hook to avoid use-after-free crash (Naoya Horiguchi) [1081914 1032347]
- [kernel] cgroup: replace list_del() with list_del_init() to avoid panic (Naoya Horiguchi) [1081915 1032343]
- [x86] turbostat: display C8, C9, C10 residency (Neil Horman) [1096711 1080637]
- [scsi] lpfc 8.3.44: Fix kernel panics from corrupted ndlp list (Rob Evers) [1086839 1063699]
- [s390] fix kernel crash due to linkage stack instructions (Hendrik Brueckner) [1067678 1067679] {CVE-2014-2039}
- [x86] kvm: rate-limit global clock updates (Andrew Jones) [1090750 1072373]
- [kernel] hrtimers: Move SMP function call to thread context (Mateusz Guzik) [1079869 1073129]
- [kernel] hrtimers: Support resuming with two or more CPUs online (Mateusz Guzik) [1079869 1073129]
- [fs] autofs4: fix device ioctl mount lookup (Ian Kent) [1069630 999708]
- [fs] vfs: introduce kern_path_mountpoint() (Ian Kent) [1069630 999708]
- [fs] vfs: rename user_path_umountat() to user_path_mountpoint_at() (Ian Kent) [1069630 999708]
- [fs] vfs: massage umount_lookup_last() a bit to reduce nesting (Ian Kent) [1069630 999708]
- [fs] vfs: allow umount to handle mountpoints without revalidating them (Ian Kent) [1069630 999708]
- [fs] ext4: fix WARN_ON from ext4_releasepage() (Carlos Maiolino) [1063508 1036814]
- [fs] vfs: fix getname() && do_getname() interaction (Oleg Nesterov) [1075653 1024689]
- [x86] apic: Make disabled_cpu_apicid static read_mostly, fix typos (Nigel Croxon) [1082622 980621]
- [x86] kexec: Add disable_cpu_apicid kernel parameter (Nigel Croxon) [1082622 980621]
- [kvm] x86: use kvm_read/write_guest_virt_system in task switch (Paolo Bonzini) [1070296 1018581]
- [kvm] x86: small cleanups to kvm_task_switch (Paolo Bonzini) [1070296 1018581]
- [kvm] x86: propagate error from kvm_load_segment_descriptor (Paolo Bonzini) [1070296 1018581]
- [kvm] x86: improve save_guest_segment_descriptor (Paolo Bonzini) [1070296 1018581]
- [kvm] x86: introduce kvm_write_guest_virt_system (Paolo Bonzini) [1070296 1018581]
- [kvm] x86: Fix task switch privilege checks (Paolo Bonzini) [1070296 1018581]
- [powerpc] Make function that parses RTAS error logs global (Steve Best) [1091424 1028682]
- [powerpc] pseries: Add RTAS event log v6 definition (Steve Best) [1091424 1028682]
- [powerpc] pseries: Parse and handle EPOW interrupts (Steve Best) [1091424 1028682]
- [fs] nfsd: don't try to reuse an expired DRC entry off the list (Jeff Layton) [1088779 1036972]
- [fs] nfsd: when reusing an existing repcache entry, unhash it first (Jeff Layton) [1088779 1036972]

* Wed Apr 30 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.19.1.el6]
- [kernel] sched: fix cpu_power initialization (Radim Krcmar) [1091826 1065304]
- [fs] gfs2: Fix uninitialized VFS inode in gfs2_create_inode (Abhijith Das) [1092002 1059808]

* Tue Apr 22 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.18.1.el6]
- [block] fix race between request completion and timeout handling (Jeff Moyer) [1089915 919756]

http://j.mp/SZmb9C

Latest available is [linux-2.6.32-431.20.3](https://github.com/cybernet/rhel6-kernel/releases/tag/2.6.32-431.20.3)
