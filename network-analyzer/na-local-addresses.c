/*
 * na-local-addreses.c
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

#include "na-local-addresses.h"

#include <glib.h>

static GSList *local_addresses = NULL;

void
add_local_address (const char *address)
{
  local_addresses = g_slist_prepend (local_addresses, (gpointer *)address);
}

gboolean
is_local_address (const char *address)
{
  GSList *iter = local_addresses;
  while (iter != NULL)
    {
      if (g_strcmp0 ((char *)iter->data, address) == 0)
        return TRUE;

      iter = iter->next;
    }

  return FALSE;
}

const GSList *
get_local_addresses ()
{
  return local_addresses;
}
