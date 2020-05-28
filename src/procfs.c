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

#include "procfs.h"

#include "config.h"
#include "table.h"

#include "debug.h"

#include <linux/version.h>

////////////////////////////////////////////////////////////////////////////////
//
// proc file read handling: output non-empty table entries
//
////////////////////////////////////////////////////////////////////////////////

static int next_table_index(int index) {
  struct table_entry entry;
  while(1) {
    ++index;
    if(index < TABLE_SIZE) {
      if(table_get(index, &entry)) {
        break;
      }
      else {
        continue;
      }
    }
    else {
      return 0;
    }
  }
  return index;
}

static void seq_show_table_entry(struct seq_file *seq, uint16_t index, struct table_entry *ent, struct config *cfg)  {
  if(!index) {
    uint8_t int_proxy_ip[4] = htoal(ntohl(cfg->int_proxy_addr));
    uint8_t ext_proxy_ip[4] = htoal(ntohl(cfg->ext_proxy_addr));
    uint8_t smoothing = cfg->smoothing;
    uint8_t loopback = cfg->loopback;
    seq_printf(seq, "config:" " int_proxy_addr: "IP_FMT " ext_proxy_addr: "IP_FMT" smoothing: "U8_FMT" loopback: "U8_FMT"\n",
               int_proxy_ip[0], int_proxy_ip[1], int_proxy_ip[2], int_proxy_ip[3],
               ext_proxy_ip[0], ext_proxy_ip[1], ext_proxy_ip[2], ext_proxy_ip[3],
               smoothing, loopback);
  }
  else {
    uint8_t int_proxy_ip[4] = htoal(ntohl(cfg->int_proxy_addr));
    uint8_t ext_proxy_ip[4] = htoal(ntohl(cfg->ext_proxy_addr));
    uint16_t proxy_port = ntohs(index);

    uint8_t sender_ip[4] = htoal(ntohl(ent->sender_addr));
    uint16_t sender_port = ntohs(ent->sender_port);

    uint8_t receiver_ip[4] = htoal(ntohl(ent->receiver_addr));
    uint16_t receiver_port = ntohs(ent->receiver_port);

    uint8_t sbc_ip[4] = htoal(ntohl(ent->sbc_addr));
    uint16_t sbc_port = ntohs(ent->sbc_port);

    seq_printf(seq,
               "=> ("IP_PORT_FMT" -> "IP_PORT_FMT") ~> ("IP_PORT_FMT" -> "IP_PORT_FMT")\n",
               sender_ip[0],    sender_ip[1],   sender_ip[2],    sender_ip[3],     sender_port,
               int_proxy_ip[0], int_proxy_ip[1], int_proxy_ip[2], int_proxy_ip[3], proxy_port,
               ext_proxy_ip[0], ext_proxy_ip[1], ext_proxy_ip[2], ext_proxy_ip[3], proxy_port,
               sbc_ip[0],       sbc_ip[1],       sbc_ip[2],       sbc_ip[3],       sbc_port);
    seq_printf(seq,
               " < ("IP_PORT_FMT" -> "IP_PORT_FMT") ~> ("IP_PORT_FMT" -> "IP_PORT_FMT")\n",
               sbc_ip[0],       sbc_ip[1],       sbc_ip[2],       sbc_ip[3],       sbc_port,
               ext_proxy_ip[0], ext_proxy_ip[1], ext_proxy_ip[2], ext_proxy_ip[3], proxy_port,
               int_proxy_ip[0], int_proxy_ip[1], int_proxy_ip[2], int_proxy_ip[3], proxy_port,
               receiver_ip[0],  receiver_ip[1],  receiver_ip[2],  receiver_ip[3],  receiver_port);
  }
}

struct iter {
  int index;
};

static void *rtp_proxy_seq_start(struct seq_file *seq, loff_t *pos) {
  int position = (int)*pos;
  struct iter *iter = seq->private;
  if(!position) {
    iter->index = 0;
    return iter;
  }
  else {
    int index = 0;
    while(position >= 0) {
      --position;
      index = next_table_index(index);
      if(!index) {
        break;
      }
    }
    if(index) {
      iter->index = index;
      return iter;
    }
    return NULL;
  }
}

static void *rtp_proxy_seq_next(struct seq_file *seq, void *v, loff_t *pos) {
  struct iter *iter = v;
  int index = iter->index;
  ++*pos;
  index = next_table_index(index);
  if(index) {
    iter->index = index;
    return iter;
  }
  return NULL;
}

static void rtp_proxy_seq_stop(struct seq_file *seq, void *v) {
  // do nothing
}

static int rtp_proxy_seq_show(struct seq_file *seq, void *v) {
  struct iter *iter = v;
  int index = iter->index;
  struct table_entry ent;
  struct config cfg;
  table_get(index, &ent);
  config_get(&cfg);
  seq_show_table_entry(seq, index, &ent, &cfg);
  return 0;
}

static struct seq_operations rtp_proxy_seq_ops = {
  .start = rtp_proxy_seq_start,
  .next  = rtp_proxy_seq_next,
  .stop  = rtp_proxy_seq_stop,
  .show  = rtp_proxy_seq_show,
};

static int rtp_proxy_open(struct inode *inode, struct file *file) {
  return seq_open_private(file, &rtp_proxy_seq_ops, sizeof(struct iter));
}

////////////////////////////////////////////////////////////////////////////////
//
// proc file write handling: read and dispatch commands
//
////////////////////////////////////////////////////////////////////////////////

#define WRITE_BUF_SIZE 256
static ssize_t rtp_proxy_write(struct file *file, const char *user_buffer, size_t len, loff_t *off) {
  size_t size = WRITE_BUF_SIZE;
  char *kernel_buffer = kmalloc(size, GFP_KERNEL);
  if(!kernel_buffer) {
    return -ENOMEM;
  }

  if(len < size) {
    size = len;
  }
  if(copy_from_user(kernel_buffer, user_buffer, size)) {
    kfree(kernel_buffer);
    return -EFAULT;
  }
  if(size >= WRITE_BUF_SIZE) {
    size = WRITE_BUF_SIZE - 1;
  }
  kernel_buffer[size] = '\0';

  if(size > 0) {
    if(!handle_command(kernel_buffer)) {
      debug_printk(BANNER "command %s failed\n", kernel_buffer);
    }
  }

  kfree(kernel_buffer);
  return size;
}

////////////////////////////////////////////////////////////////////////////////
//
// proc file creation and removal
//
////////////////////////////////////////////////////////////////////////////////

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations rtp_proxy_file_ops = {
  .owner = THIS_MODULE,
  .open = rtp_proxy_open,
  .write = rtp_proxy_write,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = seq_release_private,
};
#else
static const struct proc_ops rtp_proxy_file_ops = {
  .proc_open = rtp_proxy_open,
  .proc_write = rtp_proxy_write,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = seq_release_private
};
#endif


void proc_file_create(void) {
  proc_create(MODULE_NAME, S_IRUGO | S_IWUGO, NULL, &rtp_proxy_file_ops);
}

void proc_file_remove(void) {
  remove_proc_entry(MODULE_NAME, NULL);
}
