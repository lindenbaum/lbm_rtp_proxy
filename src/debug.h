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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUG

#ifdef __KERNEL__
#include <linux/ip.h>
#include <linux/udp.h>
#endif

#include "module.h"

const char *ip_summed_toString(__u8 ip_summed);

void debug_print_tuple(const char *msg,
                 __be32 src_addr, __be16 src_port,
                 __be32 dst_addr, __be16 dst_port);

void debug_print_skb(const char *msg, struct sk_buff *skb, struct iphdr *ip_header, struct udphdr *udp_header);

#define debug_printk(fmt, ...) printk(fmt, ##__VA_ARGS__)

#else

#define ip_summed_toString(ip_summed) (void *)0
#define debug_print_tuple(msg, src_addr, src_port, dst_addr, dst_port) do {} while(0)
#define debug_print_skb(msg, skb, ip_header, udp_header) do {} while(0)
#define debug_printk(fmt, ...) do {} while(0)

#endif

#endif // _DEBUG_H_
