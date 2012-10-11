/*
 * na-conninode.c
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

#include "na-conninode.h"

#include "na-local-addresses.h"

#include <arpa/inet.h>
#include <glib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h> /* for strlen */


/* 
 * connection-inode table. takes information from /proc/net/tcp.
 * key contains source ip, source port, destination ip, destination 
 * port in format: '1.2.3.4:5-1.2.3.4:5'
 */


/*
 * parses a /proc/net/tcp-line of the form:
 *     sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
 *     10: 020310AC:1770 9DD8A9C3:A525 01 00000000:00000000 00:00000000 00000000     0        0 2119 1 c0f4f0c0 206 40 10 3 -1                            
 *     11: 020310AC:0404 936B2ECF:0747 01 00000000:00000000 00:00000000 00000000  1000        0 2109 1 c0f4fc00 368 40 20 2 -1                            
 *
 * and of the form:
 *      2: 0000000000000000FFFF0000020310AC:0016 0000000000000000FFFF00009DD8A9C3:A526 01 00000000:00000000 02:000A7214 00000000     0        0 2525 2 c732eca0 201 40 1 2 -1
 *
 */
static void
addtoconninode (GHashTable *table,
                char *buffer)
{
  sa_family_t sa_family;
  struct in6_addr result_addr_local;
  struct in6_addr result_addr_remote;

  char rem_addr[128], local_addr[128];
  int local_port, rem_port;
  struct in6_addr in6_local;
  struct in6_addr in6_remote;

  gulong *inode = g_new (gulong, 1);

  int matches = sscanf (buffer, "%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %*X %*X:%*X %*X:%*X %*X %*d %*d %ld %*512s\n",
                        local_addr, &local_port, rem_addr, &rem_port, inode);

  if (matches != 5)
    {
      g_critical ("Unexpected buffer: '%s'",buffer);
      return;
    }
	
  if (*inode == 0)
    {
      /* connection is in TIME_WAIT state. We rely on 
       * the old data still in the table. */
      return;
    }

  if (strlen (local_addr) > 8)
    {
      /* this is an IPv6-style row */

      /* Demangle what the kernel gives us */
      sscanf (local_addr, "%08X%08X%08X%08X", 
              &in6_local.s6_addr32[0], &in6_local.s6_addr32[1],
              &in6_local.s6_addr32[2], &in6_local.s6_addr32[3]);
      sscanf (rem_addr, "%08X%08X%08X%08X",
              &in6_remote.s6_addr32[0], &in6_remote.s6_addr32[1],
              &in6_remote.s6_addr32[2], &in6_remote.s6_addr32[3]);

      if ((in6_local.s6_addr32[0] == 0x0) && (in6_local.s6_addr32[1] == 0x0)
           && (in6_local.s6_addr32[2] == 0xFFFF0000))
        {
          /* IPv4-compatible address */
          result_addr_local  = *((struct in6_addr*) &(in6_local.s6_addr32[3]));
          result_addr_remote = *((struct in6_addr*) &(in6_remote.s6_addr32[3]));
          sa_family = AF_INET;
        }
      else
        {
          /* real IPv6 address */

          //inet_ntop(AF_INET6, &in6_local, addr6, sizeof(addr6));
          //INET6_getsock(addr6, (struct sockaddr *) &localaddr);
          //inet_ntop(AF_INET6, &in6_remote, addr6, sizeof(addr6));
          //INET6_getsock(addr6, (struct sockaddr *) &remaddr);
          //localaddr.sin6_family = AF_INET6;
          //remaddr.sin6_family = AF_INET6;
          result_addr_local  = in6_local;
          result_addr_remote = in6_remote;
          sa_family = AF_INET6;
        }
    }
  else
    {
      /* this is an IPv4-style row */
      sscanf (local_addr, "%X", (guint *) &result_addr_local);
      sscanf (rem_addr, "%X",   (guint *) &result_addr_remote);
      sa_family = AF_INET;
    }

  char *local_string = g_new (char, INET6_ADDRSTRLEN);
  char *remote_string = g_new (char, INET6_ADDRSTRLEN);
  inet_ntop (sa_family, &result_addr_local,  local_string,  INET6_ADDRSTRLEN);
  inet_ntop (sa_family, &result_addr_remote, remote_string, INET6_ADDRSTRLEN);

  char *hashkey = g_strdup_printf ("%s:%d-%s:%d", local_string, local_port, remote_string, rem_port);

  g_hash_table_insert (table, hashkey, inode);

  /* workaround: sometimes, when a connection is actually from 172.16.3.1 to
   * 172.16.3.3, packages arrive from 195.169.216.157 to 172.16.3.3, where
   * 172.16.3.1 and 195.169.216.157 are the local addresses of different 
   * interfaces */
  const GSList *iter = get_local_addresses ();
  while (iter != NULL)
    {
      /* TODO maybe only add the ones with the same sa_family */
      hashkey = g_strdup_printf ("%s:%d-%s:%d", (char *)iter->data, local_port, remote_string, rem_port);
      gulong *inode_dup = g_new (gulong, 1);
      *inode_dup = *inode;
      g_hash_table_insert (table, hashkey, inode_dup);
      iter = iter->next;
    }

  g_free (local_string);
  g_free (remote_string);
}

static gboolean
addprocinfo (GHashTable *table,
             const char *filename)
{
  FILE * procinfo = fopen (filename, "r");

  char buffer[8192];

  if (procinfo == NULL)
    return FALSE;

  /* Discard header line */
  fgets (buffer, sizeof (buffer), procinfo);
  do
    {
      if (fgets (buffer, sizeof (buffer), procinfo))
        addtoconninode (table, buffer);
    }
  while (!feof (procinfo));

  fclose (procinfo);

  return TRUE;
}

void
na_conninode_table_refresh (GHashTable *table)
{
  /* we don't forget old mappings, just overwrite */

  addprocinfo (table, "/proc/net/tcp");
  addprocinfo (table, "/proc/net/tcp6");
  addprocinfo (table, "/proc/net/udp");
  addprocinfo (table, "/proc/net/udp6");
}

GHashTable *
na_conninode_table_get ()
{
  static GHashTable *the_table = NULL;

  if (G_UNLIKELY (the_table == NULL))
    the_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  return the_table;
}
