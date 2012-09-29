/*
 * na-application.h
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

#ifndef _NA_APPLICATION_H_
#define _NA_APPLICATION_H_

#include <gio/gio.h>

G_BEGIN_DECLS

#define NA_TYPE_APPLICATION            (na_application_get_type ())
#define NA_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NA_TYPE_APPLICATION, NAApplication))
#define NA_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((obj), NA_TYPE_APPLICATION, NAApplicationClass))
#define NA_IS_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NA_TYPE_APPLICATION))
#define NA_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), NA_TYPE_APPLICATION))
#define NA_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NA_TYPE_APPLICATION, NAApplicationClass))

typedef struct _NAApplication        NAApplication;
typedef struct _NAApplicationClass   NAApplicationClass;
typedef struct _NAApplicationPrivate NAApplicationPrivate;

struct _NAApplication
{
  GApplication parent;

  NAApplicationPrivate *priv;
};

struct _NAApplicationClass
{
  GApplicationClass parent_class;
};

GType            na_application_get_type () G_GNUC_CONST;
NAApplication   *na_application_new ();

void             na_application_set_refresh_delay (NAApplication *self, int refresh_delay);
GDBusConnection *na_application_get_system_bus (NAApplication *self);

G_END_DECLS

#endif
