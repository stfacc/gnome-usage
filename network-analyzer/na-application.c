/*
 * na-application.c
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

#include "na-application.h"

#include "na-pcap.h"
#include "na-process.h"

#include <nm-client.h>


#define NETWORK_ANALYZER_DBUS_NAME        "org.gnome.NetworkAnalyzer"
#define NETWORK_ANALYZER_DBUS_OBJECT_PATH "/org/gnome/NetworkAnalyzer"
#define NETWORK_ANALYZER_DBUS_IFACE       "org.gnome.NetworkAnalyzer"

static GDBusNodeInfo *introspection_data = NULL;
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='org.gnome.NetworkAnalyzer'>"
  "    <signal name='UsageChanged'>"
  "      <arg type='a(sdd)' name='processes_array'/>"
  "    </signal>"
  "  </interface>"
  "</node>";

#define DEFAULT_REFRESH_DELAY 1

enum {
  PROP_ZERO,
  PROP_REFRESH_DELAY,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE (NAApplication, na_application, G_TYPE_APPLICATION)

struct _NAApplicationPrivate
{
  GDBusConnection *system_bus;

  GSList *iface_handles;

  int refresh_delay;
};


static float
tokbps (guint32 bytes)
{
  return (((double)bytes) / PERIOD) / 1024;
}

static void
do_refresh (NAApplication *application)
{
  struct timeval t;
  gettimeofday (&t, NULL);
  GList *proc_info = na_process_info_get_all (t);

  if (proc_info == NULL)
    return;

  GVariantBuilder *builder = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
  while (proc_info != NULL)
    {
      NAProcInfo *info = (NAProcInfo *)proc_info->data;
      g_variant_builder_add (builder, "(sdd)", info->name, tokbps (info->recv), tokbps (info->sent));
      proc_info = proc_info->next;
    }

  g_debug ("DBUS: Emitting signal on system bus");
  g_dbus_connection_emit_signal (na_application_get_system_bus (application),
                                 NULL,
                                 NETWORK_ANALYZER_DBUS_OBJECT_PATH,
                                 NETWORK_ANALYZER_DBUS_IFACE,
                                 "UsageChanged",
                                 g_variant_new ("(a(sdd))", builder),
                                 NULL);

  g_variant_builder_unref (builder);
}

static gboolean
main_loop_cb (gpointer user_data)
{
  NAApplication *application = NA_APPLICATION (user_data);

  GSList *iter = application->priv->iface_handles;
  while (iter != NULL)
    {
      NAPCapHandle *handle = (NAPCapHandle *)iter->data;

      int retval = na_pcap_dispatch (handle);
      if (retval == -1 || retval == -2)
        g_debug ("pcap: Error dispatching for device %s", na_pcap_handle_get_iface (handle));
      else
        g_debug ("pcap: Processed %d packets for device %s", retval, na_pcap_handle_get_iface (handle));

      iter = iter->next;
    }

    do_refresh (application);

    return TRUE;
}

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  NAApplication *application = NA_APPLICATION (user_data);

  application->priv->system_bus = connection;

  guint id = g_dbus_connection_register_object (connection,
                                                NETWORK_ANALYZER_DBUS_OBJECT_PATH,
                                                introspection_data->interfaces[0],
                                                NULL,
                                                NULL,
                                                NULL,
                                                NULL);
  g_assert (id > 0);
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  NAApplication *application = NA_APPLICATION (user_data);

  g_debug ("Acquired the name %s on the bus, starting refresh every %d seconds", name, application->priv->refresh_delay);

  g_timeout_add_seconds (application->priv->refresh_delay, (GSourceFunc)main_loop_cb, application);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  g_debug ("Lost the name %s on the bus", name);
}


static void
get_devices (NAApplication *application)
{
  int i;
  NMClient *nm_client = nm_client_new ();

  const GPtrArray *nm_connections = nm_client_get_active_connections (nm_client);
  for (i = 0; i < nm_connections->len; i++)
    {
      NMActiveConnection *active_conn = NM_ACTIVE_CONNECTION (g_ptr_array_index (nm_connections, i));
      g_debug ("Active connection: %s", nm_active_connection_get_connection (active_conn));
    }

  const GPtrArray *nm_devices = nm_client_get_devices (nm_client);
  for (i = 0; i < nm_devices->len; i++)
    {
      NMDevice *nm_device = NM_DEVICE (g_ptr_array_index (nm_devices, i));
      if (nm_device_get_state (nm_device) == NM_DEVICE_STATE_ACTIVATED)
        {
          const char *iface = nm_device_get_iface (nm_device);
          const char *ip_iface = nm_device_get_ip_iface (nm_device);
          g_debug ("Active device found: %s with ip iface %s", iface, ip_iface);

          NMIP4Config *ip4_config = nm_device_get_ip4_config (nm_device);
          if (ip4_config != NULL)
            {
              const GSList *ip4_addresses = nm_ip4_config_get_addresses (ip4_config);
              const GSList *iter = ip4_addresses;
              while (iter != NULL)
                {
                  guint32 address = nm_ip4_address_get_address ((NMIP4Address *)iter->data);
                  struct in_addr addr = { address, };
                  char *str_addr = g_new (char, INET_ADDRSTRLEN);
                  inet_ntop (AF_INET, &addr, str_addr, INET_ADDRSTRLEN);
                  add_local_address (str_addr);
                  g_debug ("Adding local IP4 address %s", str_addr);

                  iter = iter->next;
                }
            }

          NMIP6Config *ip6_config = nm_device_get_ip6_config (nm_device);
          if (ip6_config != NULL)
            {
              const GSList *ip6_addresses = nm_ip6_config_get_addresses (ip6_config);
              const GSList *iter = ip6_addresses;
              while (iter != NULL)
                {
                  const struct in6_addr *address = nm_ip6_address_get_address ((NMIP6Address *)iter->data);
                  char *str_addr = g_new (char, INET6_ADDRSTRLEN);
                  inet_ntop (AF_INET6, address, str_addr, INET6_ADDRSTRLEN);
                  add_local_address (str_addr);
                  g_debug ("Adding local IP6 address %s", str_addr);

                  iter = iter->next;
                }
            }

          GError *error = NULL;
          NAPCapHandle *handle = na_pcap_open (iface, &error);
          if (handle != NULL)
            {
              application->priv->iface_handles = g_slist_append (application->priv->iface_handles, handle);
            }
          else
            {
              g_debug ("Error opening handler for interface %s: %s\n", iface, error->message);
              g_error_free (error);
            }
        }
    }
}

static void
na_application_activate (GApplication *application)
{
  NAApplication *self = NA_APPLICATION (application);
  //G_APPLICATION_CLASS (na_application_parent_class)->startup (application);

  get_devices (self);

  introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  g_assert (introspection_data != NULL);

  g_application_hold (application);

  guint owner_id = g_bus_own_name (G_BUS_TYPE_SYSTEM,
                                   NETWORK_ANALYZER_DBUS_NAME,
                                   G_BUS_NAME_OWNER_FLAGS_NONE,
                                   on_bus_acquired,
                                   on_name_acquired,
                                   on_name_lost,
                                   self,
                                   NULL);

  g_assert (owner_id > 0);
}

static void
na_application_init (NAApplication *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            NA_TYPE_APPLICATION,
                                            NAApplicationPrivate);

  self->priv->refresh_delay = DEFAULT_REFRESH_DELAY;
}

static void
na_application_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  NAApplication *application = NA_APPLICATION (object);

  switch (prop_id)
    {
    case PROP_REFRESH_DELAY:
      na_application_set_refresh_delay (application, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
na_application_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  NAApplication *application = NA_APPLICATION (object);

  switch (prop_id)
    {
    case PROP_REFRESH_DELAY:
      g_value_set_int (value, application->priv->refresh_delay);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
na_application_class_init (NAApplicationClass *klass)
{
  g_debug ("class init");
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *application_class = G_APPLICATION_CLASS (klass);

  object_class->get_property = na_application_get_property;
  object_class->set_property = na_application_set_property;

  application_class->activate = na_application_activate;

  properties[PROP_REFRESH_DELAY] =
    g_param_spec_int ("refresh-delay", "Refresh delay", "Refresh delay",
                      -1, G_MAXINT,
                      DEFAULT_REFRESH_DELAY,
                      G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);

  g_type_class_add_private (klass, sizeof (NAApplicationPrivate));
}

void
na_application_set_refresh_delay (NAApplication *self,
                                  int refresh_delay)
{
  self->priv->refresh_delay = (refresh_delay >= 0 ? refresh_delay : DEFAULT_REFRESH_DELAY);
}

GDBusConnection *
na_application_get_system_bus (NAApplication *self)
{
  g_return_val_if_fail (NA_IS_APPLICATION (self), NULL);

  return self->priv->system_bus;
}

NAApplication *
na_application_new (int refresh_delay)
{
  g_type_init ();

  return g_object_new (NA_TYPE_APPLICATION,
                       "application-id", "org.gnome.NetworkAnalyzer",
                       "flags", G_APPLICATION_FLAGS_NONE,
                       NULL);
}
