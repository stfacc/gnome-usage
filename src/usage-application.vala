/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class Application : Gtk.Application {
        Usage.Window? window = null;

        public SystemMonitor monitor;

        protected override void activate () {
            if (window == null) {
                window = new Usage.Window (this);
            }

            window.present ();
        }

        public Application () {
            Object (application_id: "org.gnome.Usage");
            monitor = new SystemMonitor ();
        }
    }
}
