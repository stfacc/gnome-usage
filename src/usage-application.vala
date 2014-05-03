/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class Application : Gtk.Application {
        Usage.Window? window = null;

        const GLib.ActionEntry[] action_entries = {
            { "quit", on_quit_activate }
        };

        public SystemMonitor monitor;

        protected override void activate () {
            if (window == null) {
                window = new Usage.Window (this);
            }

            window.present ();
        }

        protected override void startup () {
            base.startup ();

            var builder = new Gtk.Builder ();
            try {
                builder.add_from_resource ("/org/gnome/usage/ui/usage-app-menu.ui");
            } catch (Error e) {
                error ("loading menu builder file: %s", e.message);
            }
            var app_menu = builder.get_object ("appmenu") as MenuModel;
            set_app_menu (app_menu);

            var provider = new Gtk.CssProvider ();
            provider.load_from_data (
"""
.main-level-bar {
    -GtkLevelBar-min-block-height: 5;
}
""", -1);
            Gtk.StyleContext.add_provider_for_screen (Gdk.Screen.get_default (), provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
        }

        public Application () {
            Object (application_id: "org.gnome.Usage");
            monitor = new SystemMonitor ();

            add_action_entries (action_entries, this);
        }

        void on_quit_activate () {
            quit ();
        }
    }
}
