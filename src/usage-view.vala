/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public abstract class View : Gtk.Bin {

        protected const int MAX_NUM_ELEMENTS = 6;

        protected SystemMonitor monitor;

        public signal void mode_changed (string? title);

        public virtual void go_back () {
        }

        public View () {
            monitor = (GLib.Application.get_default () as Application).monitor;
            visible = true;
        }
    }
}

