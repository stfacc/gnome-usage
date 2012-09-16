/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public abstract class View {
        public string name { get; protected set; }
        public Gtk.Widget content { get; protected set; }
        public signal void toggled_mode (Usage.TopbarMode mode);
        public Gtk.Widget topbar_detail_content { get; protected set; }
    }
}

