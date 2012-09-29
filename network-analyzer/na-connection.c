/*
 * na-connection.c
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

#include "na-connection.h"

#include <glib.h>

static GList *connections = NULL;

typedef struct
{
  GList *list;
} NAPacketList;

static NAPacketList *
na_packet_list_new ()
{
  NAPacketList *packet_list = g_new0 (NAPacketList, 1);
  return packet_list;
}

static void
na_packet_list_free (NAPacketList *packet_list)
{
  g_list_free_full (packet_list->list, (GDestroyNotify)na_packet_free);
  g_free (packet_list);
}

static void
na_packet_list_add_packet (NAPacketList *packet_list,
                           NAPacket     *packet)
{
  if (packet_list->list == NULL)
    {
      packet_list->list = g_list_append (packet_list->list, na_packet_copy (packet));
    }
  else
    {
      NAPacket *first_packet = (NAPacket *)packet_list->list->data;
      if (packet->time.tv_sec == first_packet->time.tv_sec)
        first_packet->len += packet->len;
      else
        packet_list->list = g_list_append (packet_list->list, na_packet_copy (packet));
    }
}

static guint32
na_packet_list_sum_packets (NAPacketList   *packet_list,
                            struct timeval t)
{
  guint32 total = 0;

  GList *iter = packet_list->list;
  while (iter != NULL)
    {
      NAPacket *packet = (NAPacket *)iter->data;
      if (packet->time.tv_sec <= t.tv_sec - PERIOD)
        {
          g_list_free_full (iter, (GDestroyNotify)na_packet_free);
          if (iter->prev != NULL)
            iter->prev->next = NULL;
          else
            packet_list->list = NULL;
          return total;
        }
      total += packet->len;
      iter = iter->next;
    }

  return total;
}

struct _NAConnection
{
  NAPacket *refpacket;

  guint32 total_sent;     /* bytes */
  guint32 total_received; /* bytes */

  NAPacketList *sent_packets;
  NAPacketList *recv_packets;

  int last_packet_time; /* seconds */
};

NAConnection *
na_connection_new (NAPacket *packet)
{
  g_return_val_if_fail (packet != NULL, NULL);

  NAConnection *connection = g_new (NAConnection, 1);

  connection->sent_packets = na_packet_list_new ();
  connection->recv_packets = na_packet_list_new ();

  if (na_packet_is_outgoing (packet))
    {
      connection->total_sent = packet->len;
      connection->total_received = 0;
      connection->refpacket = na_packet_copy (packet);
      na_packet_list_add_packet (connection->sent_packets, packet);
    }
  else
    {
      connection->total_received = packet->len;
      connection->total_sent = 0;
      connection->refpacket = na_packet_new_inverted (packet);
      na_packet_list_add_packet (connection->recv_packets, packet);
    }

  connection->last_packet_time = packet->time.tv_sec;

  connections = g_list_append (connections, connection);

  return connection;
}

void
na_connection_free (NAConnection *connection)
{
  na_packet_free (connection->refpacket);
  na_packet_list_free (connection->sent_packets);
  na_packet_list_free (connection->recv_packets);

  connections = g_list_remove (connections, connection);

  g_free (connection);
}

void
na_connection_add_packet (NAConnection *connection,
                          NAPacket     *packet)
{
  connection->last_packet_time = packet->time.tv_sec;

  if (na_packet_is_outgoing (packet))
    {
      connection->total_sent += packet->len;
      na_packet_list_add_packet (connection->sent_packets, packet);
    }
  else
    {
      connection->total_received += packet->len;
      na_packet_list_add_packet (connection->recv_packets, packet);
    }
}

int
na_connection_get_last_packet_time (NAConnection *connection)
{
  return connection->last_packet_time;
}

void
na_connection_sum_packets (NAConnection  *connection,
                           struct timeval t,
                           guint32       *recv,
                           guint32       *sent)
{
  *sent = na_packet_list_sum_packets (connection->sent_packets, t);
  *recv = na_packet_list_sum_packets (connection->recv_packets, t);
}

guint32
na_connection_get_total_sent (NAConnection *connection)
{
  return connection->total_sent;
}

guint32
na_connection_get_total_received (NAConnection *connection)
{
  return connection->total_received;
}

NAPacket *
na_connection_get_refpacket (NAConnection *connection)
{
  return connection->refpacket;
}

NAConnection *
na_connection_find_from_packet (NAPacket *packet)
{
  GList *iter = connections;
  while (iter != NULL)
    {
      /* the reference packet is always *outgoing* */
      NAConnection *connection = (NAConnection *)iter->data;
      if (na_packet_match (packet, connection->refpacket))
        return connection;

      iter = iter->next;
    }

  /* Try again, now with the packet inverted */
  iter = connections;
  NAPacket *inverted_packet = na_packet_new_inverted (packet);
  while (iter != NULL)
    {
      /* the reference packet is always *outgoing* */
      NAConnection *connection = (NAConnection *)iter->data;
      if (na_packet_match (inverted_packet, connection->refpacket))
        {
          na_packet_free (inverted_packet);
          return connection;
        }

      iter = iter->next;
    }
  na_packet_free (inverted_packet);

  return NULL;
}
