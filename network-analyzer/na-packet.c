/*
 * na-packet.c
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

#include "na-packet.h"

#include "na-local-addresses.h"

#include <glib.h>
#include <netinet/in.h>

void
na_packet_free (NAPacket *packet)
{
  g_free (packet->hashstring);
  g_free (packet);
}

NAPacket *
na_packet_copy (NAPacket *packet)
{
  NAPacket *new_packet = g_new (NAPacket, 1);
  *new_packet = *packet;
  new_packet->hashstring = g_strdup (packet->hashstring);
  return new_packet;
}

gboolean
na_packet_match (NAPacket *packet, NAPacket *other)
{
  return (packet->s_port == other->s_port) && (packet->d_port == other->d_port)
    && (packet->s_ip.s_addr == other->s_ip.s_addr)
    && (packet->d_ip.s_addr == other->d_ip.s_addr);
}

gboolean
na_packet_is_outgoing (NAPacket *packet)
{
  if (packet->dir == NA_PACKET_DIRECTION_UNKNOWN)
    {
      char str_addr[INET6_ADDRSTRLEN];
      if (packet->sa_family == AF_INET)
        inet_ntop (AF_INET, &packet->s_ip, str_addr, INET6_ADDRSTRLEN);
      else
        inet_ntop (AF_INET6, &packet->s_ip6, str_addr, INET6_ADDRSTRLEN);
      if (is_local_address (str_addr))
        packet->dir = NA_PACKET_DIRECTION_OUTGOING;
      else
        packet->dir = NA_PACKET_DIRECTION_INCOMING;
    }

  return (packet->dir == NA_PACKET_DIRECTION_OUTGOING);
}

NAPacket *
na_packet_new_inverted (NAPacket *packet)
{
  if (packet->sa_family == AF_INET)
    {
      return na_packet_new_from_ip4 (packet->d_ip, packet->d_port,
                                     packet->s_ip, packet->s_port,
                                     packet->len, packet->time, NA_PACKET_DIRECTION_UNKNOWN);
    }
  else
    {
      return na_packet_new_from_ip6 (packet->d_ip6, packet->d_port,
                                     packet->s_ip6, packet->s_port,
                                     packet->len, packet->time, NA_PACKET_DIRECTION_UNKNOWN);
    }
}

NAPacket *
na_packet_new_from_ip4 (struct in_addr s_ip, gushort s_port,
                        struct in_addr d_ip, gushort d_port,
                        u_int32_t len, struct timeval time, NAPacketDirection dir)
{
  NAPacket *packet = g_new0 (NAPacket, 1);

  packet->s_ip = s_ip;
  packet->s_port = s_port;
  packet->d_ip = d_ip;
  packet->d_port = d_port;
  packet->len = len;
  packet->time = time;
  packet->dir = dir;
  packet->sa_family = AF_INET;
  packet->hashstring = NULL;

  return packet;
}

NAPacket *na_packet_new_from_ip6 (struct in6_addr s_ip, gushort s_port,
                                  struct in6_addr d_ip, gushort d_port,
                                  u_int32_t len, struct timeval time, NAPacketDirection dir)
{
  NAPacket *packet = g_new0 (NAPacket, 1);

  packet->s_ip6 = s_ip;
  packet->s_port = s_port;
  packet->d_ip6 = d_ip;
  packet->d_port = d_port;
  packet->len = len;
  packet->time = time;
  packet->dir = dir;
  packet->sa_family = AF_INET6;
  packet->hashstring = NULL;

  return packet;
}

char *
na_packet_get_hashstring (NAPacket *packet)
{
  if (packet->hashstring == NULL)
    {
      char *local_string  = g_new (char, INET6_ADDRSTRLEN);
      char *remote_string = g_new (char, INET6_ADDRSTRLEN);

      if (packet->sa_family == AF_INET)
        {
          inet_ntop (AF_INET, &packet->s_ip, local_string, INET6_ADDRSTRLEN);
          inet_ntop (AF_INET, &packet->d_ip, remote_string, INET6_ADDRSTRLEN);
	}
      else
        {
          inet_ntop (AF_INET6, &packet->s_ip6, local_string, INET6_ADDRSTRLEN);
          inet_ntop (AF_INET6, &packet->d_ip6, remote_string, INET6_ADDRSTRLEN);
	}

      if (na_packet_is_outgoing (packet))
        packet->hashstring = g_strdup_printf ("%s:%d-%s:%d", local_string, packet->s_port, remote_string, packet->d_port);
      else
        packet->hashstring = g_strdup_printf ("%s:%d-%s:%d", remote_string, packet->d_port, local_string, packet->s_port);

      g_free (local_string);
      g_free (remote_string);
    }

  return packet->hashstring;
}
