/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class BatteryView : View {
        public BatteryView () {
            name = "Battery";
            content = new Gtk.Label (name);
        }
    }
}
