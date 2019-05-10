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
#include "table.h"
#include "procfs.h"
#include "mangle.h"

#ifndef VERSION
#define VERSION "V1.0"
#endif

#ifdef DEBUG
#  ifdef TESTS
#    define MODE "DEBUG TESTS"
#  else
#    define MODE "DEBUG"
#  endif
#else
#  ifdef TESTS
#    define MODE "PRODUCTION TESTS"
#  else
#    define MODE "PRODUCTION"
#  endif
#endif

#ifndef TESTS
static
#endif
int __init rtp_proxy_init(void) {
  printk(BANNER "init "VERSION" "MODE" [build date "__DATE__" "__TIME__"]\n");
  config_init();
  table_init();
  proc_file_create();
  register_nf_hooks();
  return 0;
}

#ifndef TESTS
static
#endif
void __exit rtp_proxy_exit(void) {
  unregister_nf_hooks();
  proc_file_remove();
  printk(BANNER "exit\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lindenbaum GmbH");
MODULE_DESCRIPTION("Lindenbaum RTP proxy/relay kernel module");
module_init(rtp_proxy_init);
module_exit(rtp_proxy_exit);
