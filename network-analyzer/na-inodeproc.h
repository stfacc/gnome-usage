/*
 * na-inodeproc.h
 *
 * Copyright (c) 2012 Stefano Facchini
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
/* Partly derived from NetHogs, Copyright (c) 2004-2011 Arnout Engelen */

#ifndef _NA_INODEPROC_H_
#define _NA_INODEPROC_H_

#include <glib.h>

G_BEGIN_DECLS

typedef struct {
  gulong inode;
  int pid;
  char *name;
} NAProgram;

GHashTable *na_inodeproc_table_get ();

void        na_inodeproc_table_refresh (GHashTable *table);

NAProgram  *na_inodeproc_table_find_pid (GHashTable *table, gulong inode);

G_END_DECLS

#endif
