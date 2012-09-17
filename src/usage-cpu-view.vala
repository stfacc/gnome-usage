/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class CPUView : View {

        public CPUView () {
            name = _("CPU");

            var proc_list = new ElementList ();
            content = proc_list;

            Timeout.add_seconds (1, () => {
                proc_list.foreach ((widget) => { widget.destroy (); });

                proc_list.add (new ElementWidget (this, "", monitor.cpu_load, true));

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
