/*
 * na-packet.h
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

#ifndef _NA_PACKET_H
#define _NA_PACKET_H

//#define _BSD_SOURCE 1
#include <glib.h>
#include <netinet/in.h>

G_BEGIN_DECLS

typedef enum {
  NA_PACKET_DIRECTION_UNKNOWN,
  NA_PACKET_DIRECTION_INCOMING,
  NA_PACKET_DIRECTION_OUTGOING
} NAPacketDirection;

typedef struct {
  struct in6_addr s_ip6;
  struct in6_addr d_ip6;
  struct in_addr s_ip;
  struct in_addr d_ip;
  unsigned short s_port;
  unsigned short d_port;
  guint32 len;
  struct timeval time;

  /* private */
  NAPacketDirection dir;
  sa_family_t sa_family;
  char *hashstring;
} NAPacket;

void      na_packet_free (NAPacket *packet);

NAPacket *na_packet_copy (NAPacket *packet);

gboolean  na_packet_match (NAPacket *packet, NAPacket *other);

gboolean  na_packet_is_outgoing (NAPacket *packet);

NAPacket *na_packet_new_inverted (NAPacket *packet);

NAPacket *na_packet_new_from_ip4 (struct in_addr s_ip, gushort s_port,
                                  struct in_addr d_ip, gushort d_port,
                                  u_int32_t len, struct timeval time, NAPacketDirection dir);

NAPacket *na_packet_new_from_ip6 (struct in6_addr s_ip, gushort s_port,
                                  struct in6_addr d_ip, gushort d_port,
                                  u_int32_t len, struct timeval time, NAPacketDirection dir);

char     *na_packet_get_hashstring (NAPacket *packet);

G_END_DECLS

#endif
