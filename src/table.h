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

#ifndef _TABLE_H_
#define _TABLE_H_

#include "module.h"

#include "config.h"

#include "debug.h"

struct table_entry {
  __be32 sender_addr;
  __be16 sender_port;

  __be32 receiver_addr;
  __be16 receiver_port;

  __be32 sbc_addr;
  __be16 sbc_port;

  uint16_t last_sn;
  uint16_t offset;
  uint8_t offset_set;
  uint8_t entry_used;
};

struct routing {
  __be32 i_src_addr;
  __be16 i_src_port;
  __be32 i_dst_addr;
  __be16 i_dst_port;

  __be32 i_prx_addr;
  __be16 i_prx_port;
  __be32 e_prx_addr;
  __be16 e_prx_port;

  __be32 e_src_addr;
  __be16 e_src_port;
  __be32 e_dst_addr;
  __be16 e_dst_port;

  uint8_t loopback;

  uint8_t smoothing;
};

#define TABLE_SIZE 65536

void table_init(void);

bool table_get(__be16 index, struct table_entry *entry);

void table_put(__be16 index, struct table_entry *entry);

void table_del(__be16 index);

void table_clr(void);

typedef void *table_function(struct table_entry *entry, void *arg);

void *table_atomically(__be16 index, table_function *fn, void *arg);

bool get_routing(__be16 index,
                 struct config *cfg,
                 struct table_entry *entry,
                 struct routing *routing);

#ifdef DEBUG
void debug_print_routing(struct routing *rt);
#endif

#endif // _TABLE_H_
