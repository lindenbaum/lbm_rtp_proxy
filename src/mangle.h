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

#ifndef _MANGLE_H_
#define _MANGLE_H_

#ifdef __KERNEL__
#include <linux/netfilter_ipv4.h>
#endif

#include "module.h"
#include "debug.h"

#include "config.h"
#include "table.h"
#include "checksum.h"

// simplified nf_hookfn
typedef unsigned int mangle_hook_fn(struct iphdr *ip_header,
                                    struct udphdr *udp_header,
                                    struct table_entry *ent,
                                    struct routing *rt);

// simplified nf_hook_ops
struct mangle_hook {
  int                hooknum;  // same as nf_hook_ops hooknum
  const char        *name;     // if non-NULL, interpret as UDP packet (and log hook name in DEBUG mode)
  mangle_hook_fn    *fn;       // if also non-NULL possibly mangle UDP packet
  int                priority; // same as nf_hook_ops priority
};

// API
void register_nf_hooks(void);

void unregister_nf_hooks(void);

// must be defined elsewhere
extern int get_mangle_hooks(struct mangle_hook **mangle_hooks);

#endif // _MANGLE_H_
