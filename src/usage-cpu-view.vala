/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class CPUView : View {

        public CPUView () {
            name = _("CPU");

            var grid = new Gtk.Grid () {
                orientation = Gtk.Orientation.VERTICAL,
                row_spacing = 20,
                column_spacing = 20,
                margin = 40
            };
            content = grid;

            var graph = new GraphWidget () { hexpand = true };
            graph.set_size_request (-1, 150);
            grid.attach (graph, 0, 0, 1, 1);

            var level_bar = new MainLevelBar ();
            grid.attach (level_bar, 1, 0, 1, 1);

            var label = new Gtk.Label (null);
            grid.attach (label, 1, 1, 1, 1);

            var proc_list = new ElementList ();
            grid.attach (proc_list, 0, 2, 2, 1);

            Timeout.add_seconds (1, () => {
                proc_list.foreach ((widget) => { widget.destroy (); });

                level_bar.set_value ((int) Math.ceil (monitor.cpu_load * level_bar.NUM_LEVELS));
                label.set_text ("%d%%".printf ((int) (monitor.cpu_load * 100)));

                graph.push (monitor.cpu_load);

                List<ElementWidget> widget_list = null;
                foreach (unowned Process process in monitor.get_processes ()) {
                    var load = process.cpu_load / monitor.cpu_load;
                    var proc_widget = new ElementWidget (this, process.cmdline, load, false);
                    proc_widget.set_data ("sort_id", (int) (1000 * load));
                    widget_list.insert_sorted (proc_widget, (a, b) => {
                        var aa = a.get_data<int>("sort_id");
                        var bb = b.get_data<int>("sort_id");
                        return bb - aa;
                    });
                }

                int i = 0;
                foreach (var widget in widget_list) {
                    i++;
                    if (i > MAX_NUM_ELEMENTS || widget.get_data<int> ("sort_id") == 0) {
                        break;
                    }
                    proc_list.add (widget);
                }

                return true;
            });
        }
    }
}
