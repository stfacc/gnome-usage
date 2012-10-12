/*
 * na-process.c
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

#include "na-process.h"

#include "na-application.h"
#include "na-conninode.h"
#include "na-inodeproc.h"

#include <strings.h>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pwd.h>
#include <glib.h>


static GList *processes = NULL;

struct _NAProcess
{
  char *name;
  GList *connections;
  int pid;
  gulong inode;
  uid_t uid;
};


NAProcess *
na_process_new (gulong      inode,
                const char *name)
{
  NAProcess *process = g_new (NAProcess, 1);

  process->name = g_strdup (name);
  process->inode = inode;
  process->connections = NULL;
  process->pid = 0;
  process->uid = 0;

  return process;
}

void
na_process_free (NAProcess *process)
{
  g_free (process->name);
  g_free (process);
}

int
na_process_get_last_packet_time (NAProcess *process)
{
  int last_packet_time = 0;
  GList *iter = process->connections;
  while (iter != NULL)
    {
      NAConnection *connection = (NAConnection *)iter->data;
      int conn_last_packet_time = na_connection_get_last_packet_time (connection);
      if (conn_last_packet_time > last_packet_time)
      last_packet_time = conn_last_packet_time;
      iter = iter->next;
    }
  return last_packet_time;
}

uid_t
na_process_get_uid (NAProcess *process)
{
  return process->uid;
}

void
na_process_set_uid (NAProcess *process,
                    uid_t uid)
{
  process->uid = uid;
}

gulong
na_process_get_inode (NAProcess *process)
{
  return process->inode;
}

char *
na_process_get_name (NAProcess *process)
{
  return process->name;
}

void
na_process_sum_connections (NAProcess     *process,
                            struct timeval t,
                            guint32       *recv,
                            guint32       *sent)
{
  *sent = *recv = 0;

  GList *iter = process->connections;
  while (iter != NULL)
    {
      g_debug ("Summing connection for %s", process->name);

      NAConnection *connection = (NAConnection *)iter->data;
      if (na_connection_get_last_packet_time (connection) <= t.tv_sec - CONNTIMEOUT)
        {
          g_debug ("Removing stalled connection");
          GList *next = iter->next;
          na_connection_free (connection);
          process->connections = g_list_delete_link (process->connections, iter);
          iter = next;
        }
      else
        {
          guint32 conn_sent = 0;
          guint32 conn_recv = 0;
          na_connection_sum_packets (connection, t, &conn_recv, &conn_sent);
          g_debug ("Connection: sent %d, recv %d", conn_sent, conn_recv);
          *sent += conn_sent;
          *recv += conn_recv;
          iter = iter->next;
        }
    }
}


GList *
na_process_info_get_all (struct timeval t)
{
  GList *proc_info = NULL;

  na_conninode_table_refresh (na_conninode_table_get ());

  GList *iter = processes;
  while (iter != NULL)
    {
      NAProcess *process = (NAProcess *)iter->data;

      /* remove timed-out processes */
      if ((na_process_get_last_packet_time (process) + PROCESSTIMEOUT <= t.tv_sec))
        {
          g_debug ("Deleting timed-out process");

          GList *next = iter->next;
          na_process_free (process);
          processes = g_list_delete_link (processes, iter);
          iter = next;
        }
      else
        {
          NAProcInfo *info = g_new (NAProcInfo, 1);
          info->name = process->name;
          na_process_sum_connections (process, t, &info->recv, &info->sent);
          proc_info = g_list_append (proc_info, info);

          iter = iter->next;
        }
    }

  return proc_info;
}


static NAProcess *
find_process_from_program (NAProgram *node)
{
  GList *iter = processes;
  while (iter != NULL)
    {
      NAProcess *process = (NAProcess *)iter->data;
      if (node->pid == process->pid)
        return process;
      iter = iter->next;
    }

  return NULL;
}

/* finds process based on inode, if any */
/* should be done quickly after arrival of the packet, 
 * otherwise findPID will be outdated */
static NAProcess *
find_process (GHashTable *table,
              gulong inode)
{
  NAProgram *node = na_inodeproc_table_find_pid (table, inode);

  if (node == NULL)
    return NULL;

  return find_process_from_program (node);
}

/* 
 * returns the process from proclist with matching pid
 * if the inode is not associated with any PID, return NULL
 * if the process is not yet in the proclist, add it
 */
static NAProcess *
get_process_from_inode (gulong inode)
{
  GHashTable *table = na_inodeproc_table_get ();

  NAProgram *node = na_inodeproc_table_find_pid (table, inode);
	
  if (node == NULL)
    {
      g_debug ("No PID information for inode %lu", inode);
      return NULL;
    }

  NAProcess *proc = find_process_from_program (node);

  if (proc != NULL)
    return proc;

  NAProcess *newproc = na_process_new (inode, node->name);
  newproc->pid = node->pid;

  char *procdir = g_strdup_printf ("/proc/%d", node->pid);
  struct stat stats;
  int retval = stat (procdir, &stats);
  g_free (procdir);

/* 0 seems a proper default. 
 * used in case the PID disappeared while nethogs was running
 * TODO we can store node->uid this while info on the inodes,
 * right? */
/*
if (!ROBUST && (retval != 0))
{
	std::cerr << "Couldn't stat " << procdir << std::endl;
	assert (false);
}
*/

  if (retval != 0)
    na_process_set_uid (newproc, 0);
  else
    na_process_set_uid (newproc, stats.st_uid);

/*if (getpwuid(stats.st_uid) == NULL) {
	std::stderr << "uid for inode 
	if (!ROBUST)
		assert(false);
}*/
  processes = g_list_append (processes, newproc);

  return newproc;
}

/* 
 * Used when a new connection is encountered. Finds corresponding
 * process and adds the connection. If the connection  doesn't belong
 * to any known process, the process list is updated and a new process
 * is made. If no process can be found even then, it's added to the 
 * 'unknown' process.
 */
NAProcess *
na_process_find_from_connection (NAConnection *connection)
{
  GHashTable *conninode_table = na_conninode_table_get ();
  GHashTable *inodeproc_table = na_inodeproc_table_get ();

  NAPacket *refpacket = na_connection_get_refpacket (connection);
  char *hashstring = na_packet_get_hashstring (refpacket);
  gulong *inode = (gulong *) g_hash_table_lookup (conninode_table, hashstring);

  if (inode == NULL)
    {
      // no? refresh and check conn/inode table
      g_debug ("?  new connection not in connection-to-inode table before refresh."); 
// refresh the inode->pid table first. Presumably processing the renewed connection->inode table 
// is slow, making this worthwhile.
// We take the fact for granted that we might already know the inode->pid (unlikely anyway if we 
// haven't seen the connection->inode yet though).
      na_inodeproc_table_refresh (inodeproc_table);
      na_conninode_table_refresh (conninode_table);
      inode = (gulong *) g_hash_table_lookup (conninode_table, hashstring);

      if (inode == NULL)
        g_debug (":( inode for connection not found after refresh."); 
      else
        g_debug (":) inode for connection found after refresh.");
#if REVERSEHACK
		if (inode == 0)
		{
			/* HACK: the following is a hack for cases where the 
			 * 'local' addresses aren't properly recognised, as is 
			 * currently the case for IPv6 */

		 	/* we reverse the direction of the stream if 
			 * successful. */
			Packet * reversepacket = connection->refpacket->newInverted();
			inode = *((gulong *) g_hash_table_lookup (conninode_table, na_packet_get_hashstring (reversepacket)));

			if (inode == 0)
			{
				delete reversepacket;
                                g_debug ("LOC: %s STILL not in connection-to-inode table - adding to the unknown process",na_packet_get_hashstring (connection->refpacket));
				unknowntcp->connections = new ConnList (connection, unknowntcp->connections);
				return unknowntcp;
			}

			delete connection->refpacket;
			connection->refpacket = reversepacket;
		}
#endif
    }
  else
    {
      g_debug (";) new connection in connection-to-inode table before refresh.");
    }

  g_debug ("   inode # %llu", inode != NULL ? *inode : 0);

  NAProcess *proc = NULL;
  if (inode != NULL)
    proc = get_process_from_inode (*inode);

  if (proc == NULL)
    {
      proc = na_process_new (inode != NULL ? *inode : 0, hashstring);
      processes = g_list_append (processes, proc);
    }

  proc->connections = g_list_append (proc->connections, connection);

  return proc;
}
