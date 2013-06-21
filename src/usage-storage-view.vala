/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class StorageView : View {
        public StorageView () {
            name = _("Storage");
            add (new Gtk.Label (name));
        }
    }
}
