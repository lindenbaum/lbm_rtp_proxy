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

#ifndef _MODULE_H_
#define _MODULE_H_

#ifdef __KERNEL__
#  include <linux/module.h>
#else
#  ifdef TESTS
#    include "../tests/tests.h"
#  endif
#endif

// check for name of the module
#ifndef MODULE_NAME
#error "MODULE_NAME not set"
#endif

// banner to be used for printk output
#define BANNER KERN_INFO MODULE_NAME " -- "

// format string constants
#define U8_FMT "%hhu"
#define U16_FMT "%hu"
#define IP_FMT U8_FMT"."U8_FMT"."U8_FMT"."U8_FMT
#define PORT_FMT U16_FMT
#define IP_PORT_FMT IP_FMT":"PORT_FMT
#define CSUM_FMT "%04hx"

// byte array <-> host byte order integer conversion of IP4 addresses
#define atohl(ip) ((ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | (ip[3] << 0))
#define htoal(ip) { (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip >> 0) & 0xFF, }

// create local variable "name" of type struct "type", and fill with zeros
#define empty_struct(type, name) struct type name; memset(&name, 0, sizeof(name))

/* IP Hooks */
/* After promisc drops, checksum checks. */
#define NF_IP_PRE_ROUTING       0
/* If the packet is destined for this box. */
#define NF_IP_LOCAL_IN          1
/* If the packet is destined for another interface. */
#define NF_IP_FORWARD           2
/* Packets coming from a local process. */
#define NF_IP_LOCAL_OUT         3
/* Packets about to hit the wire. */
#define NF_IP_POST_ROUTING      4
#define NF_IP_NUMHOOKS          5

// shorter names for ip/udp header field selection
#define S_ADDR ip_header->saddr
#define S_PORT udp_header->source
#define D_ADDR ip_header->daddr
#define D_PORT udp_header->dest

#endif // _MODULE_H_
