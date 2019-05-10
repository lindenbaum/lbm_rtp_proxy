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

#include "rewrite.h"

#include "rtp_packet.h"
#include "rtcp_packet.h"

////////////////////////////////////////////////////////////////////////////////
//
// RTP REWRITE
//
// try to maintain smooth RTP sequence numbers by adding an offsets when a
// sequence number drop of at least MIN_DELTA is detected. Replace SSRC
// by the last two numbers of the IP address, followed by the port number
// of the external UDP destination (a.k.a. SBC).
//
////////////////////////////////////////////////////////////////////////////////

#define MIN_DELTA 4

struct set_SN_arg {
  uint16_t sn;
};

static inline void *set_SN_function(struct table_entry *entry, void *arg) {
  struct set_SN_arg *a = arg;

  const uint16_t sn = a->sn;

  if(!entry->entry_used) {
    entry->entry_used = 1;
  }
  else {
    uint16_t last_sn = entry->last_sn;
    uint16_t expected = last_sn + 1;
    if(sn < expected) {
      uint16_t delta = expected - sn;
      if(delta >= MIN_DELTA) {
        entry->offset += delta;
      }
    }
  }

  entry->last_sn = sn;
  a->sn += entry->offset;

  return arg;
}

static inline void rewrite_rtp(struct udphdr *udp_header, __be16 index, struct table_entry *ent) {
  int32_t remaining = ntohs(udp_header->len);
  uint8_t *data = (uint8_t *)udp_header;
  if(remaining >= sizeof(struct udphdr)) {
    data += sizeof(struct udphdr);
    remaining -= sizeof(struct udphdr);
    if(ntohs(index) & 1) { // odd port -> RTCP packet
      return;
    }
    else { // even port -> RTP PACKET
      if(remaining >= sizeof(struct rtp_packet)) {
        struct rtp_packet *packet = (struct rtp_packet *)data;
        if(packet->V == 2) {
          struct set_SN_arg a = { .sn = ntohs(packet->SN), };
          table_atomically(index, set_SN_function, &a);
          packet->SN = htons(a.sn);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// MATCHING LOGIC
//
// if a packet comes from src_addr:src_port to dst_addr:dst_port,
// then rewrite the packet so that it appears to come from src1_addr:src1_port
// and goes to dst1_addr:dst1_port. If an address or port is ______________ (0),
// then it always matches.
//
////////////////////////////////////////////////////////////////////////////////

static inline int match_udp_packet(struct iphdr *ip_header, struct udphdr *udp_header,
                                   __be32 src_addr, __be16 src_port, __be32 dst_addr, __be16 dst_port) {
  if((!src_addr || S_ADDR == src_addr) &&
     (!src_port || S_PORT == src_port) &&
     (!dst_addr || D_ADDR == dst_addr) &&
     (!dst_port || D_PORT == dst_port)) {
    int match = 5;
    if(!src_addr) match--;
    if(!src_port) match--;
    if(!dst_addr) match--;
    if(!dst_port) match--;
    return match;
  }
  else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// REWRITE LOGIC
//
// Addresses do not get rewritten if the target address is ______________ (0),
// likewise for ports.
//
////////////////////////////////////////////////////////////////////////////////

static inline void rewrite_udp_packet(struct iphdr *ip_header, struct udphdr *udp_header,
                                      __be32 src_addr, __be16 src_port, __be32 dst_addr, __be16 dst_port) {
  debug_print_tuple(" BEFORE REWRITE :", S_ADDR, S_PORT, D_ADDR, D_PORT);
  if(src_addr) S_ADDR = src_addr;
  if(src_port) S_PORT = src_port;
  if(dst_addr) D_ADDR = dst_addr;
  if(dst_port) D_PORT = dst_port;
  debug_print_tuple(" AFTER REWRITE  :", S_ADDR, S_PORT, D_ADDR, D_PORT);
}

////////////////////////////////////////////////////////////////////////////////
//
// ROUTING LOGIC
//
// The general idea is as follows: If a packet hits the proxy port, then the
// direction the packet comes from is analyzed. Packets from the sender get
// redirected to the sbc, and packets from the sbc get redirected to the
// receiver.
// In the pre routing step, only the destination address of the packet is
// altered, so a routing decision can be made. The reason for leaving the
// destination port unaltered in this step is that we still need the original
// port to determine the complete route in the post routing step.
// In the post routing step, the source address and the source and destination
// ports are altered to complete the routing.
//
////////////////////////////////////////////////////////////////////////////////

//  shorter names for routing field selection
#define I_SRC_ADDR rt->i_src_addr
#define I_SRC_PORT rt->i_src_port
#define I_DST_ADDR rt->i_dst_addr
#define I_DST_PORT rt->i_dst_port

#define I_PRX_ADDR rt->i_prx_addr
#define I_PRX_PORT rt->i_prx_port
#define E_PRX_ADDR rt->e_prx_addr
#define E_PRX_PORT rt->e_prx_port

#define E_SRC_ADDR rt->e_src_addr
#define E_SRC_PORT rt->e_src_port
#define E_DST_ADDR rt->e_dst_addr
#define E_DST_PORT rt->e_dst_port

#define __________ 0

enum { LOOPBACK_ROUTE, OUTGOING_ROUTE, INCOMING_ROUTE, AMBIGIUOS_ROUTE, NO_ROUTE, };

static inline int match_routes(struct iphdr *ip_header, struct udphdr *udp_header,
                               struct routing *rt) {
  if(rt->loopback) {
#ifdef DEBUG
    printk(BANNER "LOOPBACK_ROUTE\n");
#endif
    return LOOPBACK_ROUTE;
  }
  else {
    int outgoing_route_match = match_udp_packet(ip_header, udp_header, I_SRC_ADDR, I_SRC_PORT, __________, I_PRX_PORT);
    int incoming_route_match = match_udp_packet(ip_header, udp_header, E_SRC_ADDR, E_SRC_PORT, __________, E_PRX_PORT);
    if(outgoing_route_match > 0 || incoming_route_match > 0) {
      if(outgoing_route_match > incoming_route_match) {
#ifdef DEBUG
        printk(BANNER "OUTGOING_ROUTE\n");
#endif
        return OUTGOING_ROUTE;
      }
      else if(incoming_route_match > outgoing_route_match) {
#ifdef DEBUG
#endif
        printk(BANNER "INCOMING_ROUTE\n");
        return INCOMING_ROUTE;
      }
      else {
#ifdef DEBUG
#endif
        printk(BANNER "AMBIGIUOS_ROUTE\n");
        return AMBIGIUOS_ROUTE;
      }
    }
    else {
#ifdef DEBUG
#endif
      printk(BANNER "NO_ROUTE\n");
      return NO_ROUTE;
    }
  }
}

static inline unsigned int handle_incoming_udp_packet(struct iphdr *ip_header, struct udphdr *udp_header,
                                                      struct routing *rt) {
  switch(match_routes(ip_header, udp_header, rt)) {
  case LOOPBACK_ROUTE:
  case OUTGOING_ROUTE:
    rewrite_udp_packet(ip_header, udp_header, __________, __________, E_DST_ADDR, __________);
    return NF_ACCEPT;
  case INCOMING_ROUTE:
    rewrite_udp_packet(ip_header, udp_header, __________, __________, I_DST_ADDR, __________);
    return NF_ACCEPT;
  case AMBIGIUOS_ROUTE:
  default:
    return NF_DROP;
  }
}

static unsigned int handle_outgoing_udp_packet(struct iphdr *ip_header, struct udphdr *udp_header,
                                               struct table_entry *ent, struct routing *rt) {
  switch(match_routes(ip_header, udp_header, rt)) {
  case LOOPBACK_ROUTE:
  case OUTGOING_ROUTE:
    rewrite_udp_packet(ip_header, udp_header, E_PRX_ADDR, E_PRX_PORT, __________, E_DST_PORT);
    if(rt->smoothing){
      rewrite_rtp(udp_header, E_PRX_PORT, ent);
    }
    return NF_ACCEPT;
  case INCOMING_ROUTE:
    rewrite_udp_packet(ip_header, udp_header, I_PRX_ADDR, I_PRX_PORT, __________, I_DST_PORT);
      return NF_ACCEPT;
  default:
    return NF_DROP;
  }
}

static unsigned int pre_route_udp_packet(struct iphdr *ip_header, struct udphdr *udp_header,
                                         struct table_entry *_ent, struct routing *rt) {
  return handle_incoming_udp_packet(ip_header, udp_header, rt);
}

static unsigned int local_in_udp_packet(struct iphdr *ip_header, struct udphdr *udp_header,
                                        struct table_entry *ent, struct routing *rt) {
  return handle_outgoing_udp_packet(ip_header, udp_header, ent, rt);
}

//static unsigned int forward_udp_packet(struct iphdr *ip_header, struct udphdr *udp_header,
//                                       struct table_entry *ent, struct routing *rt) {
//  return NF_ACCEPT;
//}

static unsigned int local_out_udp_packet(struct iphdr *ip_header, struct udphdr *udp_header,
                                         struct table_entry *_ent, struct routing *rt) {
  return handle_incoming_udp_packet(ip_header, udp_header, rt);
}

static unsigned int post_route_udp_packet(struct iphdr *ip_header, struct udphdr *udp_header,
                                          struct table_entry *ent, struct routing *rt) {
  return handle_outgoing_udp_packet(ip_header, udp_header, ent, rt);
}

static struct mangle_hook mangle_hook[] =
  {
   { .hooknum = NF_IP_PRE_ROUTING,  .name = "PRE_ROUTING ", .fn = pre_route_udp_packet,  .priority = NF_IP_PRI_FIRST, },
   { .hooknum = NF_IP_LOCAL_IN,     .name = "LOCAL_IN    ", .fn = local_in_udp_packet,   .priority = NF_IP_PRI_LAST,  },
   //{ .hooknum = NF_IP_FORWARD,      .name = "FORWARD     ", .fn = forward_udp_packet,    .priority = NF_IP_PRI_FILTER, },
   { .hooknum = NF_IP_LOCAL_OUT,    .name = "LOCAL_OUT   ", .fn = local_out_udp_packet,  .priority = NF_IP_PRI_FIRST, },
   { .hooknum = NF_IP_POST_ROUTING, .name = "POST_ROUTING", .fn = post_route_udp_packet, .priority = NF_IP_PRI_LAST,  },
  };

#define HOOK_COUNT (sizeof(mangle_hook) / sizeof(mangle_hook[0]))

// required by mangle.c
int get_mangle_hooks(struct mangle_hook **mangle_hooks) {
  if(mangle_hooks) {
    *mangle_hooks = mangle_hook;
  }
  return HOOK_COUNT;
}
