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

#include "command.h"

#include <linux/random.h>

#include "debug.h"

////////////////////////////////////////////////////////////////////////////////
//
// command handling functions
//
// "a <proxy_port> <sender_ip>:<sender_port> <receiver_ip>:<receiver_port> <sbc_ip>:<sbc_port>"
//   add proxy route
//
// "d <proxy_port>"
//   delete proxy route
//
// "c <int_proxy_ip> <ext_proxy_ip>"
//   configure proxy IPs
//
// "s <smoothing (0|1)>
//   configure RTP serial number/ssrc smoothing
//
// "l <loopback (0|1)>
//   configure support for loopback routing
//
// "f"
//   flush configuration and table entries by setting it to all zero
//
////////////////////////////////////////////////////////////////////////////////

static inline void *update_table_function(struct table_entry *entry, void *arg) {
  struct table_entry *ent = arg;

  entry->sender_addr   = ent->sender_addr;
  entry->sender_port   = ent->sender_port;
  entry->receiver_addr = ent->receiver_addr;
  entry->receiver_port = ent->receiver_port;
  entry->sbc_addr      = ent->sbc_addr;
  entry->sbc_port      = ent->sbc_port;

  if(!entry->offset_set) {
    uint16_t random_sn;
    get_random_bytes(&random_sn, sizeof(random_sn));
    entry->offset = random_sn;
    entry->offset_set = 1;
  }

  return arg;
}

static void command_add(const char *parameters) {
  uint16_t proxy_port;

  uint8_t sender_ip[4];
  uint16_t sender_port;

  uint8_t receiver_ip[4];
  uint16_t receiver_port;

  uint8_t sbc_ip[4];
  uint16_t sbc_port;

  if(16 == sscanf(parameters, " "PORT_FMT" "IP_PORT_FMT" "IP_PORT_FMT" "IP_PORT_FMT" ",
                  &proxy_port,
                  &sender_ip[0],   &sender_ip[1],   &sender_ip[2],   &sender_ip[3],   &sender_port,
                  &receiver_ip[0], &receiver_ip[1], &receiver_ip[2], &receiver_ip[3], &receiver_port,
                  &sbc_ip[0],      &sbc_ip[1],      &sbc_ip[2],      &sbc_ip[3],      &sbc_port)) {

    __be16 index = htons(proxy_port);

    struct table_entry ent = {
      .sender_addr = htonl(atohl(sender_ip)),
      .sender_port = htons(sender_port),

      .receiver_addr = htonl(atohl(receiver_ip)),
      .receiver_port = htons(receiver_port),

      .sbc_addr = htonl(atohl(sbc_ip)),
      .sbc_port = htons(sbc_port),
    };

    table_atomically(index, update_table_function, &ent);
  }
  else {
    debug_printk(BANNER "command a failed\n");
  }
}

static void command_delete(const char *parameters) {
  uint16_t proxy_port;

  if(1 == sscanf(parameters, " "PORT_FMT" ",
                 &proxy_port)) {
    __be16 key = htons(proxy_port);

    table_del(key);
  }
  else {
    debug_printk(BANNER "command d failed\n");
  }
}

static void command_configure(const char *parameters) {
  uint8_t int_proxy_ip[4];
  uint8_t ext_proxy_ip[4];

  if(8 == sscanf(parameters, " "IP_FMT" "IP_FMT" ",
                 &int_proxy_ip[0], &int_proxy_ip[1], &int_proxy_ip[2], &int_proxy_ip[3],
                 &ext_proxy_ip[0], &ext_proxy_ip[1], &ext_proxy_ip[2], &ext_proxy_ip[3])) {
    struct config cfg;
    config_get(&cfg);
    cfg.int_proxy_addr = htonl(atohl(int_proxy_ip));
    cfg.ext_proxy_addr = htonl(atohl(ext_proxy_ip));
    config_set(&cfg);
  }
  else {
    debug_printk(BANNER "command c failed\n");
  }
}

static void command_smoothing(const char *parameters) {
  uint8_t smoothing;

  if(1 == sscanf(parameters, " "U8_FMT" ",
                 &smoothing)) {
    struct config cfg;
    config_get(&cfg);
    cfg.smoothing = smoothing;
    config_set(&cfg);
  }
  else {
    debug_printk(BANNER "command s failed\n");
  }
}

static void command_loopback(const char *parameters) {
  uint8_t loopback;

  if(1 == sscanf(parameters, " "U8_FMT" ",
                 &loopback)) {
    struct config cfg;
    config_get(&cfg);
    cfg.loopback = loopback;
    config_set(&cfg);
  }
  else {
    debug_printk(BANNER "command s failed\n");
  }
}

static void command_flush(const char *parameters) {
  if(0 == sscanf(parameters, " ")) {
    config_clr();
    table_clr();
  }
  else {
    debug_printk(BANNER "command f failed\n");
  }
}

// required by procfs.c
bool handle_command(const char *command) {
  debug_printk(BANNER "command: %s\n", command);
  switch(command[0]) {
  case 'a':
    command_add(&command[1]);
    return true;
  case 'd':
    command_delete(&command[1]);
    return true;
  case 'c':
    command_configure(&command[1]);
    return true;
  case 's':
    command_smoothing(&command[1]);
    return true;
  case 'l':
    command_loopback(&command[1]);
    return true;
  case 'f':
    command_flush(&command[1]);
    return true;
  default:
    return false;
  }
}
