/**
 * Copyright (C) 2015  Lindenbaum GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "checksum.h"

static inline bool can_hw_csum(struct sk_buff *skb) {
  return !skb->dev || skb->dev->features & (NETIF_F_IP_CSUM | NETIF_F_HW_CSUM);
}

static inline __wsum udp_csum_partial(struct udphdr *udp_header, __wsum csum) {
  return csum_partial(udp_header, ntohs(udp_header->len), csum);
}

static inline __sum16 udp_csum_pseudo_header_partial(struct iphdr *ip_header, struct udphdr *udp_header) {
  return ~csum_tcpudp_magic(S_ADDR, D_ADDR,
                            ntohs(udp_header->len), IPPROTO_UDP,
                            0);
}

static inline __sum16 udp_csum_pseudo_header(struct iphdr *ip_header, struct udphdr *udp_header, __wsum csum) {
  __sum16 sum = csum_tcpudp_magic(S_ADDR, D_ADDR,
                                  ntohs(udp_header->len), IPPROTO_UDP,
                                  csum);
  if(!sum) {
    return CSUM_MANGLED_0;
  }
  else {
    return sum;
  }
}

static inline void verify_checksums(struct iphdr *ip_header, struct udphdr *udp_header) {
  {
    __sum16 ip_check = ip_fast_csum((unsigned char *)ip_header, ip_header->ihl);
    if(ip_check != 0) {
      printk(BANNER " !!! verify_checksums: ip_fast_csum check failed! expected:"CSUM_FMT" actual:"CSUM_FMT"\n", ntohs(0), ntohs(ip_check));
    }
  }
  {
    if(udp_header->check) {
      __wsum csum = udp_csum_partial(udp_header, 0);
      __sum16 udp_check = ~udp_csum_pseudo_header(ip_header, udp_header, csum);
      if(udp_check != 0) {
        printk(BANNER " !!! verify_checksums: udp_csum_partial/udp_csum_pseudo_header check failed! expected:"CSUM_FMT" actual:"CSUM_FMT"\n", ntohs(0), ntohs(udp_check));
      }
    }
  }
}

static inline void verify_checksums_partial(struct iphdr *ip_header, struct udphdr *udp_header) {
  {
    __sum16 ip_check = ip_fast_csum((unsigned char *)ip_header, ip_header->ihl);
    if(ip_check != 0) {
      printk(BANNER " !!! verify_checksums_partial: ip_fast_csum check failed! expected:"CSUM_FMT" actual:"CSUM_FMT"\n", ntohs(0), ntohs(ip_check));
    }
  }
  {
    __sum16 udp_check = udp_csum_pseudo_header_partial(ip_header, udp_header);
    if(udp_check != udp_header->check) {
      printk(BANNER " !!! verify_checksums_partial: udp_csum_pseudo_header_partial check failed! expected:"CSUM_FMT" actual:"CSUM_FMT"\n", ntohs(udp_header->check), ntohs(udp_check));
    }
  }
}

void handle_incoming_checksums(struct sk_buff *skb, struct iphdr *ip_header, struct udphdr *udp_header) {
  //  A. Checksumming of received packets by device.
  switch(skb->ip_summed) {
  case CHECKSUM_NONE:
    //    Device failed to checksum this packet e.g. due to lack of capabilities.
    //    The packet contains full (though not verified) checksum in packet but
    //    not in skb->csum. Thus, skb->csum is undefined in this case.
    verify_checksums(ip_header, udp_header); // FIXME actuallay check if checksum is okay
    //skb->ip_summed = CHECKSUM_UNNECESSARY;
    break;
  case CHECKSUM_UNNECESSARY:
    //    The hardware you're dealing with doesn't calculate the full checksum
    //    (as in CHECKSUM_COMPLETE), but it does parse headers and verify checksums
    //    for specific protocols. For such packets it will set CHECKSUM_UNNECESSARY
    //    if their checksums are okay. skb->csum is still undefined in this case
    //    though. It is a bad option, but, unfortunately, nowadays most vendors do
    //    this. Apparently with the secret goal to sell you new devices, when you
    //    will add new protocol to your host, f.e. IPv6 8)
    //
    //    CHECKSUM_UNNECESSARY is applicable to following protocols:
    //      TCP: IPv6 and IPv4.
    //      UDP: IPv4 and IPv6. A device may apply CHECKSUM_UNNECESSARY to a
    //        zero UDP checksum for either IPv4 or IPv6, the networking stack
    //        may perform further validation in this case.
    //      GRE: only if the checksum is present in the header.
    //      SCTP: indicates the CRC in SCTP header has been validated.
    //
    //    skb->csum_level indicates the number of consecutive checksums found in
    //    the packet minus one that have been verified as CHECKSUM_UNNECESSARY.
    //    For instance if a device receives an IPv6->UDP->GRE->IPv4->TCP packet
    //    and a device is able to verify the checksums for UDP (possibly zero),
    //    GRE (checksum flag is set), and TCP-- skb->csum_level would be set to
    //    two. If the device were only able to verify the UDP checksum and not
    //    GRE, either because it doesn't support GRE checksum of because GRE
    //    checksum is bad, skb->csum_level would be set to zero (TCP checksum is
    //    not considered in this case).
#ifdef DEBUG
    verify_checksums(ip_header, udp_header);
#endif
    break;
  case CHECKSUM_COMPLETE:
    //    This is the most generic way. The device supplied checksum of the _whole_
    //    packet as seen by netif_rx() and fills out in skb->csum. Meaning, the
    //    hardware doesn't need to parse L3/L4 headers to implement this.
    //
    //    Note: Even if device supports only some protocols, but is able to produce
    //    skb->csum, it MUST use CHECKSUM_COMPLETE, not CHECKSUM_UNNECESSARY.
    debug_printk(BANNER " !!! WARNING(incoming): unsupported checksum type: %s\n", ip_summed_toString(skb->ip_summed));
#ifdef DEBUG
    verify_checksums(ip_header, udp_header);
#endif
    skb->ip_summed = CHECKSUM_UNNECESSARY;
    break;
  case CHECKSUM_PARTIAL:
    //    This is identical to the case for output below. This may occur on a packet
    //    received directly from another Linux OS, e.g., a virtualized Linux kernel
    //    on the same host. The packet can be treated in the same way as
    //    CHECKSUM_UNNECESSARY, except that on output (i.e., forwarding) the
    //    checksum must be filled in by the OS or the hardware.
#ifdef DEBUG
    verify_checksums_partial(ip_header, udp_header);
#endif
    break;
  default:
    break;
  }
}

static inline void calculate_ip_checksum(struct iphdr *ip_header) {
#ifdef DEBUG
  __sum16 ip_check  = ip_header->check;
#endif

  ip_header->check = 0;
  ip_header->check = ip_fast_csum((unsigned char *)ip_header, ip_header->ihl);
  debug_printk(BANNER " ip  csum: "CSUM_FMT" -> "CSUM_FMT"\n", ntohs(ip_check),  ntohs(ip_header->check));
}

static inline void calculate_pseudo_header_udp_checksum(struct iphdr *ip_header, struct udphdr *udp_header) {
#ifdef DEBUG
  __sum16 udp_check = udp_header->check;
#endif

  udp_header->check = udp_csum_pseudo_header_partial(ip_header, udp_header);
  debug_printk(BANNER " udp pseudo header csum: "CSUM_FMT" -> "CSUM_FMT"\n", ntohs(udp_check), ntohs(udp_header->check));
}

static inline void calculate_udp_checksum_full(struct iphdr *ip_header, struct udphdr *udp_header) {
#ifdef DEBUG
  __sum16 udp_check = udp_header->check;
#endif

  __wsum csum       = 0;
  udp_header->check = 0;
  csum              = udp_csum_partial(udp_header, csum);
  udp_header->check = udp_csum_pseudo_header(ip_header, udp_header, csum);
  debug_printk(BANNER " udp csum: "CSUM_FMT" -> "CSUM_FMT"\n", ntohs(udp_check), ntohs(udp_header->check));
}

static inline bool is_local_out(struct sk_buff *skb) {
  struct rtable *rt = skb_rtable(skb);
  return rt && rt->rt_flags & RTCF_LOCAL;
}

void handle_outgoing_checksums(struct sk_buff *skb, struct iphdr *ip_header, struct udphdr *udp_header) {
  //  B. Checksumming on output.
  switch(skb->ip_summed) {
  case CHECKSUM_PARTIAL:
    //    The device is required to checksum the packet as seen by hard_start_xmit()
    //    from skb->csum_start up to the end, and to record/write the checksum at
    //    offset skb->csum_start + skb->csum_offset.
    //
    //    The device must show its capabilities in dev->features, set up at device
    //    setup time, e.g. netdev_features.h:
    //
    //       NETIF_F_HW_CSUM - It's a clever device, it's able to checksum everything.
    //       NETIF_F_IP_CSUM - Device is dumb, it's able to checksum only TCP/UDP over
    //                         IPv4. Sigh. Vendors like this way for an unknown reason.
    //                         Though, see comment above about CHECKSUM_UNNECESSARY. 8)
    //       NETIF_F_IPV6_CSUM - About as dumb as the last one but does IPv6 instead.
    //       NETIF_F_...     - Well, you get the picture.
    calculate_ip_checksum(ip_header);
    calculate_pseudo_header_udp_checksum(ip_header, udp_header);

    debug_print_skb(" keeping CHECKSUM_PARTIAL", skb, ip_header, udp_header);
    break;
  case CHECKSUM_NONE:
    //    The skb was already checksummed by the protocol, or a checksum is not
    //    required.
  case CHECKSUM_UNNECESSARY:
    //    Normally, the device will do per protocol specific checksumming. Protocol
    //    implementations that do not want the NIC to perform the checksum
    //    calculation should use this flag in their outgoing skbs.
    //
    //       NETIF_F_FCOE_CRC - This indicates that the device can do FCoE FC CRC
    //                          offload. Correspondingly, the FCoE protocol driver
    //                          stack should use CHECKSUM_UNNECESSARY.
  default:
    //debug_printk(BANNER " !!! WARNING(outgoing): unsupported checksum type: %s\n", ip_summed_toString(skb->ip_summed));
    calculate_ip_checksum(ip_header);
    if(!is_local_out(skb) && can_hw_csum(skb)) {
      skb->ip_summed   = CHECKSUM_PARTIAL;
      skb->csum_start  = skb_transport_header(skb) - skb->head;
      skb->csum_offset = offsetof(struct udphdr, check);

      calculate_pseudo_header_udp_checksum(ip_header, udp_header);

      debug_print_skb(" going from CHECKSUM_UNNECESSARY to CHECKSUM_PARTIAL", skb, ip_header, udp_header);
    }
    else {
      calculate_udp_checksum_full(ip_header, udp_header);

      debug_print_skb(" keeping CHECKSUM_UNNECESSARY", skb, ip_header, udp_header);
    }
    break;
  }
  //  Any questions? No questions, good.           --ANK
}
