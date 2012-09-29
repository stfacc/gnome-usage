/*
 * na-inodeproc.c
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

#include "na-inodeproc.h"

#include <dirent.h>
#include <glib.h>

static void
na_program_free (NAProgram *p)
{
  g_free (p->name);
  g_free (p);
}

GHashTable *
na_inodeproc_table_get ()
{
  static GHashTable *the_table = NULL;

  if (G_UNLIKELY (the_table == NULL))
    the_table = g_hash_table_new_full (g_int64_hash, g_int64_equal, g_free, (GDestroyNotify)na_program_free);

  return the_table;
}

static gboolean
is_number (char *string)
{
  while (*string)
    {
      if (!isdigit (*string))
        return FALSE;
      string++;
    }
  return TRUE;
}

static char *
getprogname (char * pid)
{
  char *filename = g_strdup_printf ("/proc/%s/cmdline", pid);
  char *buffer = NULL;
  g_file_get_contents (filename, &buffer, NULL, NULL);
  g_free (filename);

  return buffer;
}

static void
get_info_by_linkname (GHashTable *table,
                      char       *pid,
                      char       *linkname)
{
  if (strncmp (linkname, "socket:[", 8) == 0)
    {
      char *ptr = linkname + 8;
      gulong *inode = g_new (gulong, 1);
      *inode = strtoul (ptr, NULL, 10);

      NAProgram *newnode = g_new (NAProgram, 1);
      newnode->inode = *inode;
      newnode->pid = strtol (pid, NULL, 10);
      newnode->name = getprogname (pid);

      g_hash_table_insert (table, inode, newnode);

      g_debug ("Found socket with inode %llu pid %s process %s", *inode, pid, newnode->name);
  }
}

/* updates the `inodeproc' inode-to-prg_node 
 * for all inodes belonging to this PID 
 * (/proc/pid/fd/42)
 * */
static void
get_info_for_pid (GHashTable *table,
                  char * pid)
{
  char *dirname = g_strdup_printf ("/proc/%s/fd", pid);

  g_debug ("Getting info for pid %s", pid);

  DIR * dir = opendir(dirname);

  if (!dir)
    {
      g_critical ("Couldn't open dir %s", dirname);
      g_free (dirname);
      return;
    }

  /* walk through /proc/%s/fd/... */
  struct dirent * entry;
  while ((entry = readdir (dir)))
    {
      if (entry->d_type != DT_LNK)
        continue;

      char *filename = g_strdup_printf ("%s/%s", dirname, entry->d_name);
      char *linkname = g_file_read_link (filename, NULL);
      g_free (filename);

      if (linkname == NULL)
        continue;

      get_info_by_linkname (table, pid, linkname);
      g_free (linkname);
    }

  closedir (dir);
  g_free (dirname);
}

void
na_inodeproc_table_refresh (GHashTable *table)
{
  DIR * proc = opendir ("/proc");

  if (proc == NULL)
    g_error ("Error reading /proc, needed to get inode-to-pid-maping");

  struct dirent *entry;
  while ((entry = readdir (proc)))
    {
      if (entry->d_type != DT_DIR)
        continue;
      if (!is_number (entry->d_name))
        continue;
      get_info_for_pid (table, entry->d_name);
    }

  closedir (proc);
}

NAProgram *
na_inodeproc_table_find_pid (GHashTable *table,
                             gulong inode)
{
  NAProgram *node = g_hash_table_lookup (table, &inode);

  if (node != NULL)
    {
      g_debug (":) Found pid in inodeproc table");
      return node;
    }

  na_inodeproc_table_refresh (table);
	
  node = g_hash_table_lookup (table, &inode);
  if (node == NULL)
    g_debug (":( No pid after inodeproc refresh");
  else
    g_debug (":) Found pid after inodeproc refresh");

  return node;
}
