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

#include "debug.h"

#ifdef DEBUG
const char *ip_summed_toString(__u8 ip_summed) {
  switch(ip_summed) {
  case CHECKSUM_NONE:
    return "CHECKSUM_NONE";
  case CHECKSUM_UNNECESSARY:
    return "CHECKSUM_UNNECESSARY";
  case CHECKSUM_COMPLETE:
    return "CHECKSUM_COMPLETE";
  case CHECKSUM_PARTIAL:
    return "CHECKSUM_PARTIAL";
  default:
    return "CHECKSUM_UNKNOWN";
  }
}

void debug_print_tuple(const char *msg,
                       __be32 src_addr, __be16 src_port,
                       __be32 dst_addr, __be16 dst_port) {
  __u8 src_addr_ip[4] = htoal(ntohl(src_addr));
  __u8 dst_addr_ip[4] = htoal(ntohl(dst_addr));
  printk(BANNER "%s "IP_PORT_FMT" -> " IP_PORT_FMT"\n",
         msg,
         src_addr_ip[0], src_addr_ip[1], src_addr_ip[2], src_addr_ip[3], ntohs(src_port),
         dst_addr_ip[0], dst_addr_ip[1], dst_addr_ip[2], dst_addr_ip[3], ntohs(dst_port));
}

void debug_print_skb(const char *msg, struct sk_buff *skb, struct iphdr *ip_header, struct udphdr *udp_header) {
  debug_print_tuple(msg, S_ADDR, S_PORT, D_ADDR, D_PORT);

  printk(BANNER " id:%hu ttl:%hhu ip_csum:"CSUM_FMT" udp_csum:"CSUM_FMT" dev:%s dst:%p\n",
         ntohs(ip_header->id),
         ip_header->ttl,
         ntohs(ip_header->check),
         ntohs(udp_header->check),
         skb->dev->name,
         skb_dst(skb));

  printk(BANNER " data:%lu tail:%lu end:%lu nh:%lu th:%lu ip_summed:%s csum_start:"U16_FMT" csum_offset:"U16_FMT"\n",
         skb->data - skb->head,
         skb_tail_pointer(skb) - skb->head,
         skb_end_pointer(skb) - skb->head,
         skb_network_header(skb) - skb->head,
         skb_transport_header(skb) - skb->head,
         ip_summed_toString(skb->ip_summed),
         skb->csum_start,
         skb->csum_offset);
}
#endif
