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

#include "table.h"

struct table_row {
  spinlock_t lock;
  struct table_entry entry;
};

static struct table_row table[TABLE_SIZE];

static inline bool is_entry_valid( struct table_entry *entry) {
  return
    entry->receiver_addr && entry->receiver_port &&
    entry->sbc_addr && entry->sbc_port;
}

void table_init(void) {
  int index;
  for(index = 0; index < TABLE_SIZE; index++) {
    spin_lock_init(&table[index].lock);
  }
  table_clr();
}

bool table_get(__be16 index, struct table_entry *entry) {
  spin_lock_bh(&table[index].lock);
  *entry = table[index].entry;
  spin_unlock_bh(&table[index].lock);
  return is_entry_valid(entry);
}

void table_put(__be16 index, struct table_entry *entry) {
  spin_lock_bh(&table[index].lock);
  table[index].entry = *entry;
  spin_unlock_bh(&table[index].lock);
}

void table_del(__be16 index) {
  empty_struct(table_entry, empty);
  table_put(index, &empty);
}

void table_clr(void) {
  int index;
  for(index = 0; index < TABLE_SIZE; index++) {
    table_del(index);
  }
}

void *table_atomically(__be16 index, table_function *fn, void *arg) {
  if(fn) {
    void *result = NULL;
    spin_lock_bh(&table[index].lock);
    result = fn(&table[index].entry, arg);
    spin_unlock_bh(&table[index].lock);
    return result;
  }
  else {
    return NULL;
  }
}

static inline void init_routing(__be16 index,
                                struct config *cfg,
                                struct table_entry *entry,
                                struct routing *routing) {
  __be32 i_src_addr = entry->sender_addr;
  __be16 i_src_port = entry->sender_port;
  __be32 i_dst_addr = entry->receiver_addr;
  __be16 i_dst_port = entry->receiver_port;

  __be32 i_prx_addr = cfg->int_proxy_addr;
  __be16 i_prx_port = index;
  __be32 e_prx_addr = cfg->ext_proxy_addr;
  __be16 e_prx_port = index;

  __be32 e_src_addr = entry->sbc_addr;
  __be16 e_src_port = entry->sbc_port;
  __be32 e_dst_addr = entry->sbc_addr;
  __be16 e_dst_port = entry->sbc_port;

  uint8_t smoothing = cfg->smoothing;

  if(cfg->loopback && i_dst_addr == i_prx_addr && i_dst_port == i_prx_port) {
    routing->i_src_addr = e_src_addr;
    routing->i_src_port = e_src_port;
    routing->i_dst_addr = e_dst_addr;
    routing->i_dst_port = e_dst_port;

    routing->i_prx_addr = e_prx_addr;
    routing->i_prx_port = e_prx_port;
    routing->e_prx_addr = e_prx_addr;
    routing->e_prx_port = e_prx_port;

    routing->e_src_addr = e_src_addr;
    routing->e_src_port = e_src_port;
    routing->e_dst_addr = e_dst_addr;
    routing->e_dst_port = e_dst_port;

    routing->loopback = 1;
  }
  else {
    routing->i_src_addr = i_src_addr;
    routing->i_src_port = i_src_port;
    routing->i_dst_addr = i_dst_addr;
    routing->i_dst_port = i_dst_port;

    routing->i_prx_addr = i_prx_addr;
    routing->i_prx_port = i_prx_port;
    routing->e_prx_addr = e_prx_addr;
    routing->e_prx_port = e_prx_port;

    routing->e_src_addr = e_src_addr;
    routing->e_src_port = e_src_port;
    routing->e_dst_addr = e_dst_addr;
    routing->e_dst_port = e_dst_port;

    routing->loopback = 0;
  }

  routing->smoothing = smoothing;
}

bool get_routing(__be16 index,
                 struct config *cfg,
                 struct table_entry *entry,
                 struct routing *routing) {
  if(table_get(index, entry)) {
    init_routing(index, cfg, entry, routing);
    if(cfg->loopback && entry->sbc_addr == cfg->ext_proxy_addr) {
      __be16 ind = entry->sbc_port;
      struct table_entry ent;
      if(table_get(ind, &ent)) {
        struct routing tmp;
        init_routing(ind, cfg, &ent, &tmp);

        routing->e_prx_addr = tmp.i_prx_addr;
        routing->e_prx_port = tmp.i_prx_port;
        routing->e_src_addr = tmp.i_src_addr;
        routing->e_src_port = tmp.i_src_port;
        routing->e_dst_addr = tmp.i_dst_addr;
        routing->e_dst_port = tmp.i_dst_port;
      }
    }
    return true;
  }
  return false;
}

#ifdef DEBUG
void debug_print_routing(struct routing *rt) {
  uint8_t  i_src_addr[4] = htoal(ntohl(rt->i_src_addr));
  uint16_t i_src_port    = ntohs(rt->i_src_port);
  uint8_t  i_dst_addr[4] = htoal(ntohl(rt->i_dst_addr));
  uint16_t i_dst_port    = ntohs(rt->i_dst_port);
  uint8_t  i_prx_addr[4] = htoal(ntohl(rt->i_prx_addr));
  uint16_t i_prx_port    = ntohs(rt->i_prx_port);
  uint8_t  e_prx_addr[4] = htoal(ntohl(rt->e_prx_addr));
  uint16_t e_prx_port    = ntohs(rt->e_prx_port);
  uint8_t  e_src_addr[4] = htoal(ntohl(rt->e_src_addr));
  uint16_t e_src_port    = ntohs(rt->e_src_port);
  uint8_t  e_dst_addr[4] = htoal(ntohl(rt->e_dst_addr));
  uint16_t e_dst_port    = ntohs(rt->e_dst_port);

  if(rt->loopback) {
    printk(BANNER "ROUTING (loopback)\n");
  }
  else {
    printk(BANNER "ROUTING\n");
  }

  printk(BANNER "=> ("IP_PORT_FMT" -> "IP_PORT_FMT") ~> ("IP_PORT_FMT" -> "IP_PORT_FMT")\n",
         i_src_addr[0],i_src_addr[1],i_src_addr[2],i_src_addr[3],i_src_port,
         i_prx_addr[0],i_prx_addr[1],i_prx_addr[2],i_prx_addr[3],i_prx_port,
         e_prx_addr[0],e_prx_addr[1],e_prx_addr[2],e_prx_addr[3],e_prx_port,
         e_dst_addr[0],e_dst_addr[1],e_dst_addr[2],e_dst_addr[3],e_dst_port);
  printk(BANNER " < ("IP_PORT_FMT" -> "IP_PORT_FMT") ~> ("IP_PORT_FMT" -> "IP_PORT_FMT")\n",
         e_src_addr[0],e_src_addr[1],e_src_addr[2],e_src_addr[3],e_src_port,
         e_prx_addr[0],e_prx_addr[1],e_prx_addr[2],e_prx_addr[3],e_prx_port,
         i_prx_addr[0],i_prx_addr[1],i_prx_addr[2],i_prx_addr[3],i_prx_port,
         i_dst_addr[0],i_dst_addr[1],i_dst_addr[2],i_dst_addr[3],i_dst_port);
}
#endif
