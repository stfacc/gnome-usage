/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class MemoryView : View {
        public MemoryView () {
            name = "Memory";

            var grid = new Gtk.Grid () {
                orientation = Gtk.Orientation.VERTICAL,
                margin_left = 20,
                margin_right = 20
            };
            grid.set_column_spacing (20);
            content = grid;

            var label = new Gtk.Label (_("Memory"));
            grid.attach (label, 0, 0, 1, 1);
            var mem_level_bar = new Gtk.LevelBar () {
                hexpand = true,
                margin_top = 20,
                margin_bottom = 20
            };
            grid.attach (mem_level_bar, 1, 0, 1, 1);

            var mem_label = new Gtk.Label ("") {
                margin_left = 20,
                margin_right = 20
            };
            grid.attach (mem_label, 2, 0, 1, 1);

            label = new Gtk.Label (_("Swap"));
            grid.attach (label, 0, 1, 1, 1);

            var swap_level_bar = new Gtk.LevelBar () {
                hexpand = true,
                margin_top = 20,
                margin_bottom = 20
            };
            grid.attach (swap_level_bar, 1, 1, 1, 1);

            var swap_label = new Gtk.Label ("") {
                margin_left = 20,
                margin_right = 20
            };
            grid.attach (swap_label, 2, 1, 1, 1);

            var monitor = ((Application) GLib.Application.get_default ()).monitor;

            Timeout.add_seconds (1, () => {
                mem_level_bar.set_value (monitor.mem_usage);
                mem_label.set_text ("%d%%".printf ((int) (monitor.mem_usage * 100)));

                swap_level_bar.set_value (monitor.swap_usage);
                swap_label.set_text ("%d%%".printf ((int) (monitor.swap_usage * 100)));
                return true;
            });
        }
    }
}
