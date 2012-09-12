/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class CPUView : View {
        public CPUView () {
            name = "CPU";
            content = new Gtk.Label (name);
        }
    }
}
