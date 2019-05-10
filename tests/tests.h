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

#ifndef _TESTS_H_
#define _TESTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <asm/types.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <errno.h>
#include <endian.h>

#if __BYTE_ORDER == __BIG_ENDIAN
# define __BIG_ENDIAN_BITFIELD
#elif __BYTE_ORDER == __LITTLE_ENDIAN
# define __LITTLE_ENDIAN_BITFIELD
#endif

// "mock out" definitions

#define KERN_INFO ""
#define printk printf

#define __be16 __u16
#define __be32 __u32

#define USHRT_MAX  ((u16)(~0U))
#define SHRT_MAX   ((s16)(USHRT_MAX>>1))
#define SHRT_MIN   ((s16)(-SHRT_MAX - 1))
#define INT_MAX    ((int)(~0U>>1))
#define INT_MIN    (-INT_MAX - 1)
#define UINT_MAX   (~0U)
#define LONG_MAX   ((long)(~0UL>>1))
#define LONG_MIN   (-LONG_MAX - 1)
#define ULONG_MAX  (~0UL)
#define LLONG_MAX  ((long long)(~0ULL>>1))
#define LLONG_MIN  (-LLONG_MAX - 1)
#define ULLONG_MAX (~0ULL)

#define THIS_MODULE 0

// provide spinlock mock definitions

typedef int spinlock_t;
#define spin_lock_init(_) do{}while(0)
#define spin_lock_bh(_)   do{}while(0)
#define spin_unlock_bh(_) do{}while(0)

// provide sk_buff mock definitions

struct net_device {
  char *name;
  int features;
};

#define NETIF_F_V4_CSUM 1

#define CHECKSUM_NONE        0
#define CHECKSUM_UNNECESSARY 1
#define CHECKSUM_COMPLETE    2
#define CHECKSUM_PARTIAL     3

#define __wsum  __u32
#define __sum16 __u16

#define CSUM_MANGLED_0 -1

struct sk_buff {
  struct net_device *dev;
  __u8               ip_summed:2;
  union {
    __wsum           csum;
    struct {
      __u16          csum_start;
      __u16          csum_offset;
    };
  };
  unsigned char     *head,
                    *data;
};

struct rtable {
  int rt_flags;
};

#define RTCF_LOCAL 1

#define skb_dst(a) 0
#define skb_linearize(a) 0
#define skb_network_header(a) (unsigned char *)0
#define skb_transport_header(a) (unsigned char *)0
#define skb_rtable(a) (struct rtable *)0
#define ip_send_check(a) 0
#define ip_fast_csum(a, b) 0
#define csum_tcpudp_magic(a, b, c, d, e) 0
#define csum_partial(a, b, c) 0

// provide netfilter mock definitions

struct nf_hook_ops;
typedef unsigned int nf_hookfn(const struct nf_hook_ops *ops,
                               struct sk_buff *skb,
                               const struct net_device *in,
                               const struct net_device *out,
                               int (*okfn)(struct sk_buff *));
struct nf_hook_ops {
  nf_hookfn     *hook;
  struct module *owner;
  void          *priv;
  u_int8_t       pf;
  unsigned int   hooknum;
  int            priority;
};

#define NF_IP_PRI_FIRST  INT_MIN
#define NF_IP_PRI_FILTER 0
#define NF_IP_PRI_LAST   INT_MAX

#define NF_MAX_HOOKS 8

#define NF_ACCEPT 1
#define NF_DROP   0

// provide proc fs mock definitions

#define gfp_t unsigned
#define GFP_KERNEL 0

void *kmalloc(size_t size, gfp_t flags);
void kfree(const void *objp);
long copy_from_user(void *to, const void *from, unsigned long n);

struct inode {
};

struct file {
};

struct file_operations {
  void *owner;
  void *open;
  void *read;
  void *write;
  void *llseek;
  void *release;
};

struct seq_file {
  void *private;
};

struct seq_operations {
  void *start;
  void *next;
  void *stop;
  void *show;
};

#define seq_read (void *)0
#define seq_lseek (void *)0
#define seq_release_private (void *)0

#define umode_t unsigned int
#define S_IRUGO 0
#define S_IWUGO 0

struct proc_dir_entry {
};

// assertion macros

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define assert_equals(expected, actual, file, line) if((expected) != (actual)) { fprintf(stderr, KRED"FAILURE"KNRM" %s:%d: expected: %lld, actual: %lld\n", file, line, (long long)(expected), (long long)(actual)); exit(-1); }

#endif // _TESTS_H_
