/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class MemoryView : View {

        public MemoryView () {
            name = _("Memory");

            var grid = new Gtk.Grid () {
                orientation = Gtk.Orientation.VERTICAL,
                margin = 40
            };
            grid.set_column_spacing (20);
            add (grid);

            var proc_list = new ElementList ();
            grid.attach (proc_list, 0, 0, 1, 1);

            Timeout.add_seconds (1, () => {
                proc_list.foreach ((widget) => { widget.destroy (); });

                proc_list.add (new ElementRow (_("<b>Memory</b>"), monitor.mem_usage, true));
                proc_list.add (new ElementRow (_("<b>Swap</b>"), monitor.swap_usage, true));

                List<ElementRow> widget_list = null;
                foreach (unowned Process process in monitor.get_processes ()) {
                    var mem = (double) process.mem_usage;
                    var proc_widget = new ElementRow (process.cmdline, mem, false);
                    proc_widget.sort_id = (int) (1000 * mem);
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

            grid.show_all ();
        }
    }
}
