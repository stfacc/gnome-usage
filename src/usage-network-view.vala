/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    struct ProcessInfo {
        string name;
        double received;
        double sent;
    }

    [DBus (name = "org.gnome.NetworkAnalyzer")]
    interface NetworkAnalyzer : Object {
        public signal void usage_changed (ProcessInfo[] proc_info);
    }

    public class NetworkView : View {

        NetworkAnalyzer network_analyzer = null;

        public NetworkView () {
            name = _("Network");

            var list_box = new Egg.ListBox () {
                margin = 40
            };
            content = list_box;

            var size_group = new Gtk.SizeGroup (Gtk.SizeGroupMode.HORIZONTAL);

            try {
                network_analyzer = Bus.get_proxy_sync (BusType.SYSTEM,
                                                       "org.gnome.NetworkAnalyzer",
                                                       "/org/gnome/NetworkAnalyzer");
                network_analyzer.usage_changed.connect ((proc_info) => {
                    list_box.foreach ((widget) => { widget.destroy (); });

                    var title = new Gtk.Grid () {
                        orientation = Gtk.Orientation.HORIZONTAL,
                        column_spacing = 10,
                        halign = Gtk.Align.END
                    };
                    var label = new Gtk.Label (null);
                    label.set_markup ("<b>Received (kb/s)</b>");
                    size_group.add_widget (label);
                    label.xalign = 1.0f;
                    title.add (label);
                    label = new Gtk.Label (null);
                    label.set_markup ("<b>Sent (kb/s)</b>");
                    size_group.add_widget (label);
                    label.xalign = 1.0f;
                    title.add (label);

                    list_box.add (title);

                    foreach (var info in proc_info) {
                        var element = new Gtk.Grid () {
                            orientation = Gtk.Orientation.HORIZONTAL,
                            column_spacing = 10
                        };

                        label = new Gtk.Label (info.name) {
                            hexpand = true,
                            halign = Gtk.Align.START
                        };
                        element.add (label);

                        label = new Gtk.Label ("%.2f".printf (info.received));
                        label.xalign = 1.0f;
                        size_group.add_widget (label);
                        element.add (label);
                        label = new Gtk.Label ("%.2f".printf (info.sent));
                        label.xalign = 1.0f;
                        size_group.add_widget (label);
                        element.add (label);

                        list_box.add (element);
                    }

                    list_box.show_all ();
                });
            } catch (Error e) {
                print ("DBus error: %s\n", e.message);
            }
        }
    }
}
