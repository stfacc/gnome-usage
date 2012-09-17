/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class NetworkView : View {
        public NetworkView () {
            name = _("Network");
            content = new Gtk.Label (name);
        }
    }
}
