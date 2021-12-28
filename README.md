lbm_rtp_proxy
=============

A kernel module proxying/relaying RTP/RTCP traffic.

Attention Developers
--------------------

Always **adapt the version** after hacking on this project.

* Update the `LBM_RTP_PROXY_VERSION` in the [Makefile](./Makefile)
* Update the version in the [RPM build .spec file](./dist/lbm_rtp_proxy.spec)
* Create a git tag for the version, this is important because the source
  archive is defined in terms of the version, i.e.
  `https://github.com/lindenbaum/lbm_rtp_proxy/archive/%{version}.tar.gz`


Netfilter IP Hooks
------------------

The following is a little drawing, that helps understanding the kernels packet
filter/routing chains.

```
    +---------------------------------------------------------------------------------------------+
    |                                                                                             |
    |                             N E T F I L T E R   I P   H O O K S                             |
    |                             ===================================                             |
    |                                                                                             |
    |     -------                               -------                               -------     |
    |   /        (0)                          /        (2)                          /        (4)  |
       |    PRE    |      +-----------+      |           |                         |   POST    |
 ----> |           | ---> |  Routing  | ---> |  FORWARD  | -----------+----------> |           | ---->
 from  |  ROUTING  |      +-----------+      |           |            |            |  ROUTING  |   to
 wire   \         /             |             \         /             |             \         /   wire
          -------               |               -------               |               -------
    |                           |                               +-----------+                     |
    |                           |                               |  Routing  |                     |
    |                           |                               +-----------+                     |
    |                           |                                     ^                           |
    |                           |                                     |                           |
    |                           v                                     |                           |
    |                        -------                               -------                        |
    |                      /        (1)        ---------         /        (3)                     |
    |                     |   LOCAL   |      |   Local   |      |   LOCAL   |                     |
    |                     |           | ---> |           | ---> |           |                     |
    |                     |    IN     |      | Processes |      |    OUT    |                     |
    |                      \         /         ---------         \         /                      |
    |                        -------                               -------                        |
    |                                                                                             |
    +---------------------------------------------------------------------------------------------+
    |                                                                                             |
    |  (0) After promisc drops, checksum checks                                                   |
    |  (1) If the packet is destined for this box                                                 |
    |  (2) If the packet is destined for another interface                                        |
    |  (3) Packets coming from a local process                                                    |
    |  (4) Packets about to hit the wire                                                          |
    |                                                                                             |
    +---------------------------------------------------------------------------------------------+
```
