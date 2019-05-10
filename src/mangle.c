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

#include "mangle.h"

#include <linux/version.h>

static inline bool get_ip_and_udp_headers(struct sk_buff *skb, struct iphdr **ip_header_out, struct udphdr **udp_header_out) {
  if(skb) {
    if(!skb_linearize(skb)) {
      struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);
      if(ip_header) {
        if (ip_header->protocol == IPPROTO_UDP) {
          struct udphdr *udp_header = (struct udphdr *)skb_transport_header(skb);
          if(udp_header) {
            if(ip_header_out) {
              *ip_header_out = ip_header;
            }
            if(udp_header_out) {
              *udp_header_out = udp_header;
            }
            return true;
          }
        }
      }
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// GENERIC HOOK FUNCTION
////////////////////////////////////////////////////////////////////////////////

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
static unsigned int generic_hook_func(const struct nf_hook_ops *ops,
                                      struct sk_buff *skb,
                                      const struct net_device *_in,
                                      const struct net_device *_out,
                                      const struct nf_hook_state *state) {
  struct mangle_hook *mangle_hook = ops->priv;
#else
static unsigned int generic_hook_func(void *priv,
                                      struct sk_buff *skb,
                                      const struct nf_hook_state *state) {
  struct mangle_hook *mangle_hook = priv;
#endif
  if(mangle_hook) {
    if(mangle_hook->name) {
      struct iphdr *ip_header;
      struct udphdr *udp_header;
      if(get_ip_and_udp_headers(skb, &ip_header, &udp_header)) {
        if(mangle_hook->fn) {
          // get internal/external proxy IPs from config
          struct config cfg;
          config_get(&cfg);
          {
            // lookup entry for destination port of incoming UDP packet
            __be16 index = udp_header->dest;
            struct table_entry ent;
            struct routing rt;
            // if entry is found
            if(get_routing(index, &cfg, &ent, &rt)) {
              debug_print_skb(mangle_hook->name, skb, ip_header, udp_header);
              if(state->hook == NF_IP_PRE_ROUTING || state->hook == NF_IP_LOCAL_OUT) {
                handle_incoming_checksums(skb, ip_header, udp_header);
              }
              switch(mangle_hook->fn(ip_header, udp_header, &ent, &rt)) {
              case NF_ACCEPT:
                if(state->hook == NF_IP_LOCAL_OUT) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
                  int err = ip_route_me_harder(skb, RTN_UNSPEC);
#else
                  int err = ip_route_me_harder(state->net, skb, RTN_UNSPEC);
#endif
                  if (err < 0) {
                    debug_printk(BANNER " ip_route_me_harder FAILED -> DROP\n");
                    return NF_DROP_ERR(err);
                  }
                }
                else if(state->hook == NF_IP_POST_ROUTING) {
                  handle_outgoing_checksums(skb, ip_header, udp_header);
                }
                return NF_ACCEPT;
              case NF_DROP:
                debug_printk(BANNER " packet could not be routed -> DROP\n");
                return NF_DROP;
              }
            }
          }
        }
      }
    }
  }
  return NF_ACCEPT;
}

////////////////////////////////////////////////////////////////////////////////
// registration and unregistration of netfilter hooks
////////////////////////////////////////////////////////////////////////////////

static int hook_count = 0;
static struct nf_hook_ops hook_ops[NF_MAX_HOOKS];

void register_nf_hooks(void) {
  struct mangle_hook *mangle_hooks;
  hook_count = get_mangle_hooks(&mangle_hooks);
  if(hook_count > NF_MAX_HOOKS) {
    debug_printk(BANNER "too many hooks (%d), setting to 0", hook_count);
    hook_count = 0;
  }
  if(hook_count > 0) {
    int i;
    for(i = 0; i < hook_count; i++) {
      hook_ops[i].hook     = generic_hook_func;
      hook_ops[i].hooknum  = mangle_hooks[i].hooknum;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
      hook_ops[i].owner    = THIS_MODULE;
#endif
      hook_ops[i].priv     = &mangle_hooks[i];
      hook_ops[i].pf       = PF_INET;
      hook_ops[i].priority = mangle_hooks[i].priority;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
    nf_register_hooks(hook_ops, hook_count);
#else
    // TODO init_net might not always be appropriate
    // https://stackoverflow.com/a/13355999
    nf_register_net_hooks(&init_net, hook_ops, hook_count);
#endif
  }
}

void unregister_nf_hooks(void) {
  if(hook_count > 0) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
    nf_unregister_hooks(hook_ops, hook_count);
#else
    // TODO init_net might not always be appropriate
    // https://stackoverflow.com/a/13355999
    nf_unregister_net_hooks(&init_net, hook_ops, hook_count);
#endif
  }
}
