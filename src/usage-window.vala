/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class Window : Gtk.ApplicationWindow {
        public Window (Application app) {
            Object (application: app);

            title = _("Usage");
            set_default_size (800, 500);
            hide_titlebar_when_maximized = true;
        }
    }
}
