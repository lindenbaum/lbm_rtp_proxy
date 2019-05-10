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

#include "config.h"

static spinlock_t config_lock;
static struct config config;

#ifdef DEBUG
static void config_print(struct config *cfg) {
  uint8_t int_proxy_ip[4] = htoal(ntohl(cfg->int_proxy_addr));
  uint8_t ext_proxy_ip[4] = htoal(ntohl(cfg->ext_proxy_addr));
  uint8_t smoothing = cfg->smoothing;
  uint8_t loopback = cfg->loopback;
  printk(BANNER "config:" " int_proxy_addr: "IP_FMT " ext_proxy_addr: "IP_FMT" smoothing: "U8_FMT" loopback: "U8_FMT"\n",
         int_proxy_ip[0], int_proxy_ip[1], int_proxy_ip[2], int_proxy_ip[3],
         ext_proxy_ip[0], ext_proxy_ip[1], ext_proxy_ip[2], ext_proxy_ip[3],
         smoothing, loopback);
}
#else
#define config_print(x) do {} while(0)
#endif

void config_init(void) {
  spin_lock_init(&config_lock);
  config_clr();
}

void config_set(struct config *cfg) {
  spin_lock_bh(&config_lock);
  config = *cfg;
  spin_unlock_bh(&config_lock);

  config_print(cfg);
}

void config_get(struct config *cfg) {
  spin_lock_bh(&config_lock);
  *cfg = config;
  spin_unlock_bh(&config_lock);
}

void config_clr(void) {
  empty_struct(config, cfg);
  cfg.smoothing = 1;
  cfg.loopback = 1;
  config_set(&cfg);
}
