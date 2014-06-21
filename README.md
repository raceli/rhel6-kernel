RHEL6 Kernel - v2.6.32.x
============

Detailed changes between RHEL kernel releases

* Mon Mar 03 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.11.2.el6]
- [net] sctp: fix sctp_sf_do_5_1D_ce to verify if peer is AUTH capable (Daniel Borkmann) [1070715 1067451] {CVE-2014-0101}
- [vhost] validate vhost_get_vq_desc return value (Michael S. Tsirkin) [1062579 1058677] {CVE-2014-0055}

* Fri Feb 28 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.11.1.el6]
- [net] netpoll: take rcu_read_lock_bh() in netpoll_send_skb_on_dev() (Florian Westphal) [1063271 1049052]
- [fs] cifs: sanity check length of data to send before sending (Sachin Prabhu) [1065668 1062590] {CVE-2014-0069}
- [fs] cifs: ensure that uncached writes handle unmapped areas correctly (Sachin Prabhu) [1065668 1062590] {CVE-2014-0069}
- [infiniband] ipoib: Report operstate consistently when brought up without a link (Michal Schmidt) [1064464 995300]
- [security] selinux: fix broken peer recv check (Paul Moore) [1059991 1043051]
- [fs] GFS2: Fix slab memory leak in gfs2_bufdata (Robert S Peterson) [1064913 1024024]
- [fs] GFS2: Fix use-after-free race when calling gfs2_remove_from_ail (Robert S Peterson) [1064913 1024024]
- [fs] nfs: always make sure page is up-to-date before extending a write to cover the entire page (Scott Mayhew) [1066942 1054493]
- [fs] xfs: ensure we capture IO errors correctly (Lachlan McIlroy) [1058418 1021325]
- [mm] get rid of unnecessary pageblock scanning in setup_zone_migrate_reserve (Motohiro Kosaki) [1062113 1043353]
- [security] selinux: process labeled IPsec TCP SYN-ACK packets properly in selinux_ip_postroute() (Paul Moore) [1055364 1024631]
- [security] selinux: look for IPsec labels on both inbound and outbound packets (Paul Moore) [1055364 1024631]
- [security] selinux: handle TCP SYN-ACK packets correctly in selinux_ip_postroute() (Paul Moore) [1055364 1024631]
- [security] selinux: handle TCP SYN-ACK packets correctly in selinux_ip_output() (Paul Moore) [1055364 1024631]
- [edac] e752x_edac: Fix pci_dev usage count (Aristeu Rozanski) [1058420 1029530]
- [s390] mm: handle asce-type exceptions as normal page fault (Hendrik Brueckner) [1057164 1034268]
- [s390] mm: correct tlb flush on page table upgrade (Hendrik Brueckner) [1057165 1034269]
- [net] fix memory information leaks in recv protocol handlers (Florian Westphal) [1039868 1039869]
- [usb] cdc-wdm: fix buffer overflow (Alexander Gordeev) [922000 922001] {CVE-2013-1860}
- [usb] cdc-wdm: Fix race between autosuspend and reading from the device (Alexander Gordeev) [922000 922001] {CVE-2013-1860}

* Tue Feb 25 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.10.1.el6]
- [fs] xfs: xfs_remove deadlocks due to inverted AGF vs AGI lock ordering (Brian Foster) [1067775 1059334]
- [x86] apic: Map the local apic when parsing the MP table (Prarit Bhargava) [1063507 1061873]

* Wed Feb 19 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.9.1.el6]
- [netdrv] bonding: add NETIF_F_NO_CSUM vlan_features (Ivan Vecera) [1063199 1059777]

* Sat Feb 15 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.8.1.el6]
- [netdrv] enic: remove enic->vlan_group check (Stefan Assmann) [1064115 1057704]

* Fri Feb 07 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.7.1.el6]
- [char] n_tty: Fix unsafe update of available buffer space (Jiri Benc) [1060491 980188]
- [char] n_tty: Fix stuck throttled driver (Jiri Benc) [1060491 980188]
- [char] tty: Add safe tty throttle/unthrottle functions (Jiri Benc) [1060491 980188]
- [char] tty: note race we need to fix (Jiri Benc) [1060491 980188]

* Sat Jan 18 2014 Petr Holasek <pholasek@redhat.com> [2.6.32-431.6.1.el6]
- [mm] memcg: fix oom schedule_timeout() (Ulrich Obergfell) [1054072 1034237]
- [mm] memcg: change memcg_oom_mutex to spinlock (Ulrich Obergfell) [1054072 1034237]
- [mm] memcg: fix hierarchical oom locking (Ulrich Obergfell) [1054072 1034237]
- [mm] memcg: make oom_lock 0 and 1 based rather than counter (Ulrich Obergfell) [1054072 1034237]

http://j.mp/1iCxJbP

Latest available is [linux-2.6.32-431.17.1](https://github.com/cybernet/rhel6-kernel/releases/tag/2.6.32-431.17.1)
