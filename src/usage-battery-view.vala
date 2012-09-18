/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    enum UpDeviceKind {
        UNKNOWN,
        AC_POWER,
        BATTERY,
        UPS,
        MONITOR,
        MOUSE,
        KEYBOARD,
        PDA,
        PHONE,
        MEDIA_PLAYER,
        TABLET,
        COMPUTER
    }

    enum UpDeviceState {
        UNKNOWN,
        CHARGING,
        DISCHARGING,
        EMPTY,
        FULLY_CHARGED,
        PENDING_CHARGE,
        PENDING_DISCHARGE
    }

    struct UpDevice {
        string id;
        uint kind;
        string icon;
        double percentage;
        uint state;
        uint64 seconds;
    }

    [DBus (name = "org.gnome.SettingsDaemon.Power")]
    interface PowerManager : Object {
        public abstract UpDevice get_primary_device () throws IOError;
        public abstract UpDevice[] get_devices () throws IOError;
    }

    public class BatteryView : View {
        public BatteryView () {
            name = _("Battery");

            PowerManager power_manager = null;
            try {
                power_manager = Bus.get_proxy_sync (BusType.SESSION,
                                                    "org.gnome.SettingsDaemon",
                                                    "/org/gnome/SettingsDaemon/Power");
            } catch (IOError e) {
                stderr.printf ("%s\n", e.message);
            }

            var grid = new Gtk.Grid () {
                orientation = Gtk.Orientation.VERTICAL,
                row_spacing = 20,
                column_spacing = 20,
                margin = 40
            };
            content = grid;

            var graph = new GraphWidget () { hexpand = true, num_points = 400 };
            graph.set_size_request (-1, 150);
            grid.attach (graph, 0, 0, 1, 1);

            var time_label = new Gtk.Label (null) { halign = Gtk.Align.END };
            grid.attach (time_label, 0, 1, 1, 1);

            var level_bar = new MainLevelBar ();
            grid.attach (level_bar, 1, 0, 1, 1);

            var percent_label = new Gtk.Label (null);
            grid.attach (percent_label, 1, 1, 1, 1);

            Timeout.add_seconds (1, () => {
                if (power_manager != null) {
                    var devices = power_manager.get_devices ();
                    foreach (var device in devices) {
                        if (device.kind == UpDeviceKind.BATTERY) {
                            var time = (int) (device.seconds / 60);
                            var hours = (int) (time / 60);
                            var minutes = time % 60;

                            level_bar.set_value (device.percentage * level_bar.NUM_LEVELS / 100);
                            percent_label.set_text ("%d%%".printf ((int) device.percentage));
                            string timestring;
                            switch (device.state) {
                                case UpDeviceState.CHARGING:
                                    timestring = "%d:%d hours remaining to complete charge".printf (hours, minutes);
                                    break;
                                case UpDeviceState.FULLY_CHARGED:
                                    timestring = "Fully charged";
                                    break;
                                default:
                                    timestring = "%d:%d hours remaining".printf (hours, minutes);
                                    break;
                            }
                            time_label.set_text (timestring);
                            graph.push ((double) device.percentage / 100);

                            break;
                        }
                    }
                }

                return true;
            });
        }
    }
}
