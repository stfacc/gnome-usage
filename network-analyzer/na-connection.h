/*
 * na-connection.h
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

#ifndef _NA_CONNECTION_H_
#define _NA_CONNECTION_H_

#include "na-packet.h"

#include <glib.h>

G_BEGIN_DECLS

/* take the average speed over the last 5 seconds */
#define PERIOD 5

typedef struct _NAConnection NAConnection;

NAConnection *na_connection_new                  (NAPacket *packet);
void          na_connection_free                 (NAConnection *connection);
void          na_connection_add_packet           (NAConnection *connection,
                                                  NAPacket *packet);
int           na_connection_get_last_packet_time (NAConnection *connection);
void          na_connection_sum_packets          (NAConnection *connection,
                                                  struct timeval t,
                                                  guint32 *recv,
                                                  guint32 *sent);

guint32       na_connection_get_total_sent        (NAConnection *connection);
guint32       na_connection_get_total_received    (NAConnection *connection);

NAPacket     *na_connection_get_refpacket         (NAConnection *connection);

NAConnection *na_connection_find_from_packet      (NAPacket *packet);

G_END_DECLS

#endif
