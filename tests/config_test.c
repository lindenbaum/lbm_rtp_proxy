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

#include "../src/config.h"

static void new_config_is_all_zero_test(void) {
  struct config cfg;
  config_get(&cfg);
  bool is_non_zero = cfg.int_proxy_addr || cfg.ext_proxy_addr;
  if(is_non_zero) {
    printf("BUG %hu %hu\n",
           cfg.int_proxy_addr, cfg.ext_proxy_addr);
    exit(-1);
  }
}

int main(int argc, char **argv) {
  config_init();

  new_config_is_all_zero_test();

  printf(KGRN"SUCCESS"KNRM"\n");
  exit(0);
}
