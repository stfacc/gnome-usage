/*
 * na-process.h
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

#ifndef _NA_PROCESS_H_
#define _NA_PROCESS_H_

#include "na-application.h"
#include "na-connection.h"

#include <glib.h>

G_BEGIN_DECLS

/* the amount of time after the last packet was recieved
 * after which a connection is removed */
#define CONNTIMEOUT 25

/* the amount of time after the last packet was recieved
 * after which a process is removed */
#define PROCESSTIMEOUT 50

typedef struct _NAProcess NAProcess;

typedef struct
{
  char *name;
  pid_t pid;
  guint32 sent;
  guint32 recv;
} NAProcInfo;

NAProcess *na_process_new                  (const char *name);

void       na_process_free                 (NAProcess *process);

int        na_process_get_last_packet_time (NAProcess *process);

uid_t      na_process_get_uid              (NAProcess *process);

pid_t      na_process_get_pid              (NAProcess *process);

char      *na_process_get_name             (NAProcess *process);

void       na_process_sum_connections      (NAProcess *process,
                                            struct timeval t,
                                            guint32 *recv,
                                            guint32 *sent);

NAProcess *na_process_find_from_connection (NAConnection *connection);

GList     *na_process_info_get_all         (struct timeval t);

G_END_DECLS

#endif
