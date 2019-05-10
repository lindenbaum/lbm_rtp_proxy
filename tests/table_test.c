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

#include "tests.h"

#include "../src/table.h"
#include "../src/config.h"

////////////////////////////////////////////////////////////////////////////////
//
// helper functions
//

void dump_config(void) {
  struct config cfg;
  config_get(&cfg);
  uint8_t int_proxy_ip[4] = htoal(ntohl(cfg.int_proxy_addr));
  uint8_t ext_proxy_ip[4] = htoal(ntohl(cfg.ext_proxy_addr));
  uint8_t smoothing = cfg.smoothing;
  uint8_t loopback = cfg.loopback;
  printf("config:" " int_proxy_addr: "IP_FMT " ext_proxy_addr: "IP_FMT" smoothing: "U8_FMT" loopback: "U8_FMT"\n",
         int_proxy_ip[0], int_proxy_ip[1], int_proxy_ip[2], int_proxy_ip[3],
         ext_proxy_ip[0], ext_proxy_ip[1], ext_proxy_ip[2], ext_proxy_ip[3],
         smoothing, loopback);
}

static void set_config(uint8_t int_proxy_ip[4], uint8_t ext_proxy_ip[4]) {
  struct config cfg;
  config_get(&cfg);
  cfg.int_proxy_addr = htonl(atohl(int_proxy_ip));
  cfg.ext_proxy_addr = htonl(atohl(ext_proxy_ip));
  config_set(&cfg);
}

void dump_table(void) {
  struct config cfg;
  config_get(&cfg);
  int index;
  printf("TABLE\n");
  for(index = 0; index < TABLE_SIZE; index++) {
    struct table_entry ent;
    bool contains_entry = table_get(index, &ent);
    if(contains_entry) {
      uint8_t int_proxy_ip[4] = htoal(ntohl(cfg.int_proxy_addr));
      uint8_t ext_proxy_ip[4] = htoal(ntohl(cfg.ext_proxy_addr));
      uint16_t proxy_port = ntohs(index);

      uint8_t sender_ip[4] = htoal(ntohl(ent.sender_addr));
      uint16_t sender_port = ntohs(ent.sender_port);

      uint8_t receiver_ip[4] = htoal(ntohl(ent.receiver_addr));
      uint16_t receiver_port = ntohs(ent.receiver_port);

      uint8_t sbc_ip[4] = htoal(ntohl(ent.sbc_addr));
      uint16_t sbc_port = ntohs(ent.sbc_port);

      printf("=> ("IP_PORT_FMT" -> "IP_PORT_FMT") ~> ("IP_PORT_FMT" -> "IP_PORT_FMT")\n",
             sender_ip[0],    sender_ip[1],   sender_ip[2],    sender_ip[3],     sender_port,
             int_proxy_ip[0], int_proxy_ip[1], int_proxy_ip[2], int_proxy_ip[3], proxy_port,
             ext_proxy_ip[0], ext_proxy_ip[1], ext_proxy_ip[2], ext_proxy_ip[3], proxy_port,
             sbc_ip[0],       sbc_ip[1],       sbc_ip[2],       sbc_ip[3],       sbc_port);
      printf(" < ("IP_PORT_FMT" -> "IP_PORT_FMT") ~> ("IP_PORT_FMT" -> "IP_PORT_FMT")\n",
             sbc_ip[0],       sbc_ip[1],       sbc_ip[2],       sbc_ip[3],       sbc_port,
             ext_proxy_ip[0], ext_proxy_ip[1], ext_proxy_ip[2], ext_proxy_ip[3], proxy_port,
             int_proxy_ip[0], int_proxy_ip[1], int_proxy_ip[2], int_proxy_ip[3], proxy_port,
             receiver_ip[0],  receiver_ip[1],  receiver_ip[2],  receiver_ip[3],  receiver_port);
    }
  }
}

void dump_routing(struct routing *rt) {
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
    printf(BANNER "ROUTING (loopback)\n");
  }
  else {
    printf("ROUTING\n");
  }
  printf("=> ("IP_PORT_FMT" -> "IP_PORT_FMT") ~> ("IP_PORT_FMT" -> "IP_PORT_FMT")\n",
         i_src_addr[0],i_src_addr[1],i_src_addr[2],i_src_addr[3],i_src_port,
         i_prx_addr[0],i_prx_addr[1],i_prx_addr[2],i_prx_addr[3],i_prx_port,
         e_prx_addr[0],e_prx_addr[1],e_prx_addr[2],e_prx_addr[3],e_prx_port,
         e_dst_addr[0],e_dst_addr[1],e_dst_addr[2],e_dst_addr[3],e_dst_port);
  printf(" < ("IP_PORT_FMT" -> "IP_PORT_FMT") ~> ("IP_PORT_FMT" -> "IP_PORT_FMT")\n",
         e_src_addr[0],e_src_addr[1],e_src_addr[2],e_src_addr[3],e_src_port,
         e_prx_addr[0],e_prx_addr[1],e_prx_addr[2],e_prx_addr[3],e_prx_port,
         i_prx_addr[0],i_prx_addr[1],i_prx_addr[2],i_prx_addr[3],i_prx_port,
         i_dst_addr[0],i_dst_addr[1],i_dst_addr[2],i_dst_addr[3],i_dst_port);
}

static void add_route(uint16_t prx_port,
                      uint8_t snd_ip[4], uint16_t snd_port,
                      uint8_t rcv_ip[4], uint16_t rcv_port,
                      uint8_t sbc_ip[4], uint16_t sbc_port) {
  __be16 key = htons(prx_port);

  struct table_entry ent = {
    .sender_addr = htonl(atohl(snd_ip)),
    .sender_port = htons(snd_port),

    .receiver_addr = htonl(atohl(rcv_ip)),
    .receiver_port = htons(rcv_port),

    .sbc_addr = htonl(atohl(sbc_ip)),
    .sbc_port = htons(sbc_port),
  };

  table_put(key, &ent);
}

static void assert_route(uint16_t prx_port,
                         uint8_t snd_ip[4], uint16_t snd_port,
                         uint8_t int_prx_ip[4],
                         uint8_t ext_prx_ip[4],
                         uint8_t sbc_ip[4], uint16_t sbc_port,
                         uint8_t rcv_ip[4], uint16_t rcv_port,
                         char *FILE, int LINE) {
  struct config cfg;
  config_get(&cfg);
  __be16 key = htons(prx_port);
  struct table_entry ent;
  if(table_get(key, &ent)) {

    assert_equals(atohl(snd_ip),     ntohl(ent.sender_addr),    FILE, LINE);
    assert_equals(snd_port,          ntohs(ent.sender_port),    FILE, LINE);

    assert_equals(atohl(int_prx_ip), ntohl(cfg.int_proxy_addr), FILE, LINE);
    assert_equals(atohl(ext_prx_ip), ntohl(cfg.ext_proxy_addr), FILE, LINE);

    assert_equals(atohl(sbc_ip),     ntohl(ent.sbc_addr),       FILE, LINE);
    assert_equals(sbc_port,          ntohs(ent.sbc_port),       FILE, LINE);

    assert_equals(atohl(rcv_ip),     ntohl(ent.receiver_addr),  FILE, LINE);
    assert_equals(rcv_port,          ntohs(ent.receiver_port),  FILE, LINE);

    return;
  }
  exit(-1);
}

static void assert_route_cascading(uint16_t prx_port,
                                   uint8_t src_1_ip[4], uint16_t src_1_port,
                                   uint8_t int_1_ip[4], uint16_t int_1_port,
                                   uint8_t ext_1_ip[4], uint16_t ext_1_port,
                                   uint8_t dst_1_ip[4], uint16_t dst_1_port,
                                   uint8_t src_2_ip[4], uint16_t src_2_port,
                                   uint8_t ext_2_ip[4], uint16_t ext_2_port,
                                   uint8_t int_2_ip[4], uint16_t int_2_port,
                                   uint8_t dst_2_ip[4], uint16_t dst_2_port,
                                   char *FILE, int LINE) {
  struct config cfg;
  config_get(&cfg);
  __be16 key = htons(prx_port);
  struct table_entry ent;
  struct routing rt;
  if(get_routing(key, &cfg, &ent, &rt)) {
    dump_routing(&rt);
    assert_equals(atohl(src_1_ip),  ntohl(rt.i_src_addr), FILE, LINE);
    assert_equals(      src_1_port, ntohs(rt.i_src_port), FILE, LINE);
    assert_equals(atohl(dst_2_ip),  ntohl(rt.i_dst_addr), FILE, LINE);
    assert_equals(      dst_2_port, ntohs(rt.i_dst_port), FILE, LINE);

    assert_equals(atohl(int_1_ip),  ntohl(rt.i_prx_addr), FILE, LINE);
    assert_equals(atohl(int_2_ip),  ntohl(rt.i_prx_addr), FILE, LINE);

    assert_equals(      prx_port,   ntohs(rt.i_prx_port), FILE, LINE);
    assert_equals(      int_2_port, ntohs(rt.i_prx_port), FILE, LINE);

    assert_equals(atohl(ext_1_ip),  ntohl(rt.e_prx_addr), FILE, LINE);
    assert_equals(atohl(ext_2_ip),  ntohl(rt.e_prx_addr), FILE, LINE);

    assert_equals(atohl(src_2_ip),  ntohl(rt.e_src_addr), FILE, LINE);
    assert_equals(      src_2_port, ntohs(rt.e_src_port), FILE, LINE);
    assert_equals(atohl(dst_1_ip),  ntohl(rt.e_dst_addr), FILE, LINE);
    assert_equals(      dst_1_port, ntohs(rt.e_dst_port), FILE, LINE);

    return;
  }
  exit(-1);
}


////////////////////////////////////////////////////////////////////////////////
//
// test functions
//

static void new_table_contains_no_entries_test(void) {
  struct config cfg;
  config_get(&cfg);
  int index;
  for(index = 0; index < TABLE_SIZE; index++) {
    struct table_entry ent;
    bool contains_entry = table_get(index, &ent);
    if(contains_entry) {
      printf(KRED"BUG"KNRM" %d %hu %hhu %hu %hhu %hu %hhu\n", index,
             ent.sender_addr, ent.sender_port,
             ent.receiver_addr, ent.receiver_port,
             ent.sbc_addr, ent.sbc_port);
      exit(-1);
    }
  }
}

#define SBC_IP   {213, 30, 241, 190}
#define PROXY_IP {10, 1, 40, 121}
#define INT_PROXY_IP {1, 1, 1, 1}
#define EXT_PROXY_IP {2, 2, 2, 2}
#define MEDIA_IP {192, 168, 100, 8}

static void cascade_test(void) {
  uint8_t int_ip[4] = PROXY_IP;
  uint8_t ext_ip[4] = PROXY_IP;

  set_config(int_ip, ext_ip);

  uint16_t prx_port_1 = 32768;
  uint16_t prx_port_2 = 32770;

  uint8_t snd_ip_1[4] = MEDIA_IP;
  uint16_t snd_port_1 = 18562;

  uint8_t rcv_ip_1[4] = MEDIA_IP;
  uint16_t rcv_port_1 = 18560;

  uint8_t sbc_ip_1[4] = PROXY_IP;
  uint16_t sbc_port_1 = prx_port_2;

  uint8_t snd_ip_2[4] = MEDIA_IP;
  uint16_t snd_port_2 = 18568;

  uint8_t rcv_ip_2[4] = MEDIA_IP;
  uint16_t rcv_port_2 = 18564;

  uint8_t sbc_ip_2[4] = PROXY_IP;
  uint16_t sbc_port_2 = prx_port_1;

  add_route(prx_port_1,
            snd_ip_1, snd_port_1,
            rcv_ip_1, rcv_port_1,
            sbc_ip_1, sbc_port_1);
  dump_config();
  dump_table();

  assert_route(prx_port_1,
               snd_ip_1, snd_port_1,
               int_ip,
               ext_ip,
               sbc_ip_1, sbc_port_1,
               rcv_ip_1, rcv_port_1,
               __FILE__, __LINE__);

  assert_route_cascading(prx_port_1,
                         // from sender channel
                         snd_ip_1, snd_port_1,
                         int_ip, prx_port_1,
                         ext_ip, prx_port_1,
                         sbc_ip_1, sbc_port_1,
                         // to receiver channel
                         sbc_ip_1, sbc_port_1,
                         ext_ip, prx_port_1,
                         int_ip, prx_port_1,
                         rcv_ip_1, rcv_port_1,
                         //
                         __FILE__, __LINE__);

  add_route(prx_port_2,
            snd_ip_2, snd_port_2,
            rcv_ip_2, rcv_port_2,
            sbc_ip_2, sbc_port_2);
  dump_table();

  assert_route(prx_port_2,
               snd_ip_2, snd_port_2,
               int_ip,
               ext_ip,
               sbc_ip_2, sbc_port_2,
               rcv_ip_2, rcv_port_2,
               __FILE__, __LINE__);

  assert_route_cascading(prx_port_1,
                         // from sender channel
                         snd_ip_1, snd_port_1,
                         int_ip, prx_port_1,
                         int_ip, prx_port_2,
                         rcv_ip_2, rcv_port_2,
                         // to receiver channel
                         snd_ip_2, snd_port_2,
                         int_ip, prx_port_2,
                         int_ip, prx_port_1,
                         rcv_ip_1, rcv_port_1,
                         //
                         __FILE__, __LINE__);

  assert_route_cascading(prx_port_2,
                         // from sender channel
                         snd_ip_2, snd_port_2,
                         int_ip, prx_port_2,
                         int_ip, prx_port_1,
                         rcv_ip_1, rcv_port_1,
                         // to receiver channel
                         snd_ip_1, snd_port_1,
                         int_ip, prx_port_1,
                         int_ip, prx_port_2,
                         rcv_ip_2, rcv_port_2,
                         //
                         __FILE__, __LINE__);

}

static void short_circuiting_test(void) {
  uint8_t int_ip[4] = INT_PROXY_IP;
  uint8_t ext_ip[4] = EXT_PROXY_IP;

  set_config(int_ip, ext_ip);

  uint16_t prx_port  = 32768;

  uint8_t  snd_ip[4] = {0, 0, 0, 0};
  uint16_t snd_port  = 0;

  uint8_t  rcv_ip[4] = INT_PROXY_IP;
  uint16_t rcv_port  = prx_port;

  uint8_t sbc_ip[4]  = SBC_IP;
  uint16_t sbc_port  = 40960;

  add_route(prx_port,
            snd_ip, snd_port,
            rcv_ip, rcv_port,
            sbc_ip, sbc_port);

  dump_config();
  dump_table();

  struct config cfg;
  config_get(&cfg);
  __be16 key = htons(prx_port);
  struct table_entry ent;
  struct routing rt;
  if(get_routing(key, &cfg, &ent, &rt)) {
    dump_routing(&rt);
  }
  else {
    printf(KRED"BUG"KNRM" (no routing)\n");
    exit(-1);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// main function
//

int main(int argc, char **argv) {
  table_init();

  config_init();


  new_table_contains_no_entries_test();

  printf("\n");
  cascade_test();

  printf("\n");
  table_init();
  new_table_contains_no_entries_test();
  short_circuiting_test();

  printf(KGRN"SUCCESS"KNRM"\n");
  exit(0);
}
