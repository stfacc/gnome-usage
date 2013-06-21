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
            add (grid);

            var graph = new GraphWidget () { hexpand = true };
            graph.set_size_request (-1, 150);
            var graph_overlay = new Gtk.Overlay ();
            graph_overlay.add (graph);
            var graph_label = new Gtk.Label (_("Total CPU load")) {
                margin = 5,
                halign = Gtk.Align.START,
                valign = Gtk.Align.START
            };
            graph_label.get_style_context ().add_class ("dim-label");
            graph_overlay.add_overlay (graph_label);
            grid.attach (graph_overlay, 0, 0, 1, 1);

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

                List<ElementRow> widget_list = null;
                foreach (unowned Process process in monitor.get_processes ()) {
                    var load = process.cpu_load / monitor.cpu_load;
                    var proc_widget = new ElementRow (process.cmdline, load, false);
                    proc_widget.sort_id = (int) (1000 * load);
                    widget_list.insert_sorted (proc_widget, (a, b) => {
                        return (b as ElementRow).sort_id - (a as ElementRow).sort_id;
                    });
                }

                for (int i = 0; i < MAX_NUM_ELEMENTS; i ++) {
                    proc_list.add (widget_list.data);
                    widget_list = (owned) widget_list.next;
                }

                return true;
            });
        }
    }
}
