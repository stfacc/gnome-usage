/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class MemoryView : View {
        public MemoryView () {
            name = "Memory";
            content = new Gtk.Label (name);
        }
    }
}
