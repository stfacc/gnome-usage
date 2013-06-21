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

    class NetworkListRow : Gtk.ListBoxRow {
        static Gtk.SizeGroup size_group = null;

        public int sort_id;

        public NetworkListRow (string name, string received, string sent, int _sort_id) {
            if (size_group == null) {
                size_group = new Gtk.SizeGroup (Gtk.SizeGroupMode.HORIZONTAL);
            }

            var grid = new Gtk.Grid () {
                orientation = Gtk.Orientation.HORIZONTAL,
                column_spacing = 10
            };

            var label = new Gtk.Label (null) {
                hexpand = true,
                halign = Gtk.Align.START
            };
            label.set_markup (name);
            grid.add (label);

            label = new Gtk.Label (null);
            label.set_markup (received);
            label.xalign = 1.0f;
            size_group.add_widget (label);
            grid.add (label);
            label = new Gtk.Label (null);
            label.set_markup (sent);
            label.xalign = 1.0f;
            size_group.add_widget (label);
            grid.add (label);

            add (grid);

            sort_id = _sort_id;
        }

    }

    public class NetworkView : View {

        const int NETWORK_ANALYZER_TIMEOUT = 2;

        NetworkAnalyzer network_analyzer = null;
        GraphWidget graph;
        Gtk.ListBox list_box;


        public NetworkView () {
            name = _("Network");

            var grid = new Gtk.Grid () {
                orientation = Gtk.Orientation.VERTICAL,
                row_spacing = 20,
                column_spacing = 20,
                margin = 40
            };
            add (grid);

            graph = new GraphWidget () { hexpand = true };
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

            list_box = new Gtk.ListBox ();
            grid.attach (list_box, 0, 1, 1, 1);

            list_box.set_sort_func ((a, b) => {
                return (b as NetworkListRow).sort_id - (a as NetworkListRow).sort_id;
            });

            start_network_analyzer ();
        }

        void start_network_analyzer () {
            Bus.get_proxy.begin<NetworkAnalyzer> (BusType.SYSTEM,
                                                  "org.gnome.NetworkAnalyzer",
                                                  "/org/gnome/NetworkAnalyzer",
                                                  0,
                                                  null,
                                                  (obj, res) => {
                                                      try {
                                                          network_analyzer = Bus.get_proxy.end (res);
                                                          connect_network_analyzer_signal ();
                                                      } catch (Error e) {
                                                          print ("Error connecting to NetworkAnalyzer\n");
                                                      }
                                                  });
        }

        void connect_network_analyzer_signal () {
            uint network_analyzer_timeout_id = 0;
            network_analyzer.usage_changed.connect ((proc_info) => {
                if (network_analyzer_timeout_id > 0) {
                    Source.remove (network_analyzer_timeout_id);
                }
                network_analyzer_timeout_id = Timeout.add_seconds (NETWORK_ANALYZER_TIMEOUT, () => {
                        start_network_analyzer ();
                        return false;
                });

                try {
                    network_analyzer.acknowledge ();
                } catch (Error e) {
                    print ("Error calling dbus method org.gnome.NetworkAnalyzer.Acknowledge\n");
                }

                list_box.foreach ((widget) => { widget.destroy (); });

                list_box.add (new NetworkListRow ("", _("<b>Received (kb/s)</b>"), _("<b>Sent (kb/s)</b>"), int.MAX - 2));

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

                    list_box.add (new NetworkListRow (info.name, "%.2f".printf (info.received), "%.2f".printf (info.sent), (int)(info.received * 100)));
                }

                if (unknown_received > 0 || unknown_sent > 0) {
                    list_box.add (new NetworkListRow (_("Unknown traffic"), "%.2f".printf (unknown_received), "%.2f".printf (unknown_sent), -1));
                }

                list_box.add (new NetworkListRow (_("<b>Total</b>"), "<b>%.2f</b>".printf (total_received), "<b>%.2f</b>".printf (total_sent), -2));

                list_box.show_all ();

                // Set an arbitrary maximum of 500kb/s
                graph.push (total_received / 500.0);
            });
        }
    }
}
