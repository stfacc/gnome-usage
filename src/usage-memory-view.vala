/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class MemoryView : View {

        const int MAX_NUM_ELEMENTS = 6;

        public MemoryView () {
            name = "Memory";

            var grid = new Gtk.Grid () {
                orientation = Gtk.Orientation.VERTICAL,
                margin_left = 20,
                margin_right = 20
            };
            grid.set_column_spacing (20);
            content = grid;

            var proc_list = new ElementList ();
            grid.attach (proc_list, 0, 0, 1, 1);

            var monitor = ((Application) GLib.Application.get_default ()).monitor;

            Timeout.add_seconds (1, () => {
                proc_list.foreach ((widget) => { widget.destroy (); });

                proc_list.add (new ElementWidget (this, _("Memory"), monitor.mem_usage, true));
                proc_list.add (new ElementWidget (this, _("Swap"), monitor.swap_usage, true));

                List<ElementWidget> widget_list = null;
                foreach (unowned Process process in monitor.process_table.get_values ()) {
                    var mem = (double) process.mem_usage;
                    var proc_widget = new ElementWidget (this, process.cmdline, mem, false);
                    proc_widget.set_data ("sort_id", (int) (1000 * mem));
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
