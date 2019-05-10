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

#ifndef _PROCFS_H_
#define _PROCFS_H_

#ifdef __KERNEL__
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#endif

#include "module.h"

//API

// create proc file
void proc_file_create(void);

// remove proc file
void proc_file_remove(void);

// must be defined elsewhere
extern bool handle_command(const char *command);

#endif // _PROCFS_H_
