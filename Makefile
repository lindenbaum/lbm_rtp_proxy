#
# Copyright (C) 2015  Lindenbaum GmbH
# Copyright (C) 2021  vier GmbH
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

KSRC ?= /lib/modules/$(shell uname -r)/build
KBUILD := $(KSRC)

ifeq ($(LBM_RTP_PROXY_VERSION),)
  LBM_RTP_PROXY_VERSION := 1.0.3-1
endif

ifdef DEBUG
  EXTRA_CFLAGS += -DDEBUG
  LBM_RTP_PROXY_VERSION += $(LBM_RTP_PROXY_VERSION)-debug
endif
EXTRA_CFLAGS += -DMODULE_NAME="\"lbm_rtp_proxy\""
EXTRA_CFLAGS += -DVERSION="\"$(LBM_RTP_PROXY_VERSION)\""
EXTRA_CFLAGS += -Wall -Wno-date-time

obj-m += lbm_rtp_proxy.o
lbm_rtp_proxy-objs := src/module.o \
                      src/config.o \
                      src/table.o \
                      src/procfs.o \
                      src/mangle.o \
                      src/checksum.o \
                      src/debug.o \
                      src/command.o \
                      src/rewrite.o

.PHONY: all
all:
	$(MAKE) -C $(KSRC) M=$(shell pwd) O=$(KSRC) modules

.PHONY: test
test:
	$(MAKE) -C tests all

.PHONY: clean
clean:
	$(MAKE) -C tests clean || true
	$(MAKE) -C $(KSRC) M=$(shell pwd) clean || true
