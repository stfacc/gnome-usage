/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class CPUView : View {

        public CPUView () {
            name = "CPU";

            var grid = new Gtk.Grid () {
                orientation = Gtk.Orientation.VERTICAL,
                margin_left = 20,
                margin_right = 20
            };
            content = grid;

            var level_bar = new Gtk.LevelBar () {
                hexpand = true,
                margin_top = 20,
                margin_bottom = 20
            };
            grid.attach (level_bar, 1, 0, 1, 1);

            var label = new Gtk.Label ("") {
                margin_left = 20,
                margin_right = 20
            };
            grid.attach (label, 2, 0, 1, 1);

            var proc_list = new Egg.ListBox ();
            grid.attach (proc_list, 1, 1, 2, 1);
            proc_list.set_sort_func ((a, b) => {
                var aa = a.get_data<int>("sort_id");
                var bb = b.get_data<int>("sort_id");
                return bb - aa;
            });
            proc_list.set_filter_func ((w) => {
                return w.get_data<int>("sort_id") > 0;
            });

            var monitor = ((Application) GLib.Application.get_default ()).monitor;

            Timeout.add (200, () => {
                level_bar.set_value (monitor.cpu_load);
                return true;
            });

            Timeout.add_seconds (1, () => {
                label.set_text ("%d%%".printf ((int) (monitor.cpu_load * 100)));

                proc_list.foreach ((widget) => { widget.destroy (); });
                foreach (unowned Process process in monitor.process_table.get_values ()) {
                    var load = process.cpu_load / monitor.cpu_load;
                    var proc_widget = new ElementWidget (this, process.cmdline, load);
                    proc_widget.set_data ("sort_id", (int) (100 * load));
                    proc_list.add (proc_widget);
                }

                return true;
            });
        }
    }
}
