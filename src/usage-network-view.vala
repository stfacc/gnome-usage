/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    struct ProcessInfo {
        string name;
        int pid;
        double received;
        double sent;
    }

    [DBus (name = "org.gnome.NetworkAnalyzer")]
    interface NetworkAnalyzer : Object {
        public abstract void acknowledge () throws IOError;
        public signal void usage_changed (ProcessInfo[] proc_info);
    }

    public class NetworkView : View {

        NetworkAnalyzer network_analyzer = null;
        Gtk.SizeGroup size_group;

        Gtk.Grid make_element (string name, string received, string sent, int sort_id) {
            var element = new Gtk.Grid () {
                orientation = Gtk.Orientation.HORIZONTAL,
                column_spacing = 10
            };

            var label = new Gtk.Label (null) {
                hexpand = true,
                halign = Gtk.Align.START
            };
            label.set_markup (name);
            element.add (label);

            label = new Gtk.Label (null);
            label.set_markup (received);
            label.xalign = 1.0f;
            size_group.add_widget (label);
            element.add (label);
            label = new Gtk.Label (null);
            label.set_markup (sent);
            label.xalign = 1.0f;
            size_group.add_widget (label);
            element.add (label);

            element.set_data ("sort_id", sort_id);

            return element;
        }

        public NetworkView () {
            name = _("Network");

            var grid = new Gtk.Grid () {
                orientation = Gtk.Orientation.VERTICAL,
                row_spacing = 20,
                column_spacing = 20,
                margin = 40
            };
            content = grid;

            var graph = new GraphWidget () { hexpand = true };
            graph.set_size_request (-1, 150);
            var graph_overlay = new Gtk.Overlay ();
            graph_overlay.add (graph);
            var graph_label = new Gtk.Label (_("Received network traffic")) {
                margin = 5,
                halign = Gtk.Align.START,
                valign = Gtk.Align.START
            };
            graph_label.get_style_context ().add_class ("dim-label");
            graph_overlay.add_overlay (graph_label);
            grid.attach (graph_overlay, 0, 0, 1, 1);

            var list_box = new Egg.ListBox ();
            grid.attach (list_box, 0, 1, 1, 1);

            list_box.set_sort_func ((a, b) => {
                var aa = a.get_data<int>("sort_id");
                var bb = b.get_data<int>("sort_id");
                return bb - aa;
            });

            size_group = new Gtk.SizeGroup (Gtk.SizeGroupMode.HORIZONTAL);

            try {
                network_analyzer = Bus.get_proxy_sync (BusType.SYSTEM,
                                                       "org.gnome.NetworkAnalyzer",
                                                       "/org/gnome/NetworkAnalyzer");
                network_analyzer.usage_changed.connect ((proc_info) => {
                    network_analyzer.acknowledge ();

                    list_box.foreach ((widget) => { widget.destroy (); });

                    list_box.add (make_element ("", _("<b>Received (kb/s)</b>"), _("<b>Sent (kb/s)</b>"), int.MAX - 2));

                    double total_received = 0;
                    double total_sent = 0;
                    double unknown_received = 0;
                    double unknown_sent = 0;

                    foreach (var info in proc_info) {
                        total_received += info.received;
                        total_sent += info.sent;

                        if (info.pid == 0) {
                            unknown_received += info.received;
                            unknown_sent += info.sent;
                            continue;
                        }

                        list_box.add (make_element (info.name, "%.2f".printf (info.received), "%.2f".printf (info.sent), (int)(info.received * 100)));
                    }

                    if (unknown_received > 0 || unknown_sent > 0) {
                        list_box.add (make_element (_("Unknown traffic"), "%.2f".printf (unknown_received), "%.2f".printf (unknown_sent), -1));
                    }

                    list_box.add (make_element (_("<b>Total</b>"), "<b>%.2f</b>".printf (total_received), "<b>%.2f</b>".printf (total_sent), -2));

                    list_box.show_all ();

                    // Set an arbitrary maximum of 500kb/s
                    graph.push (total_received / 500.0);
                });
            } catch (Error e) {
                print ("DBus error: %s\n", e.message);
            }
        }
    }
}
