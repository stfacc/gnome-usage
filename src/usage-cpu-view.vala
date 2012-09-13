/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    [Compact]
    public class Process {
        internal int pid;
        internal string cmdline;
        internal double cpu_load;
        internal uint64 cpu_last_used;

        internal bool alive;
    }

    public class SystemMonitor {
        public double cpu_load { get; private set; }

        uint64 cpu_last_used = 0;
        uint64 cpu_last_total = 0;

        const int UPDATE_INTERVAL = 1000;
        public HashTable<uint, Process> process_table;

        public SystemMonitor () {
            GTop.init ();

            process_table = new HashTable<uint, Process> (direct_hash, direct_equal);

            Timeout.add (UPDATE_INTERVAL, () => {
                GTop.Cpu cpu_data;
                GTop.get_cpu (out cpu_data);
                var used = cpu_data.user + cpu_data.nice + cpu_data.sys;

                cpu_load = ((double) (used - cpu_last_used)) / (cpu_data.total - cpu_last_total);

                foreach (unowned Process process in process_table.get_values ()) {
                    process.alive = false;
                }

                var uid = Posix.getuid ();
                GTop.Proclist proclist;
                var pids = GTop.get_proclist (out proclist, GTop.KERN_PROC_UID, uid);
                for (int i = 0; i < proclist.number; i++) {
                    GTop.ProcState proc_state;
                    GTop.ProcTime proc_time;
                    GTop.get_proc_state (out proc_state, pids[i]);
                    GTop.get_proc_time (out proc_time, pids[i]);

                    if (!(pids[i] in process_table)) {
                        var process = new Process ();
                        process.alive = true;
                        process.cmdline = (string) proc_state.cmd;
                        process.cpu_load = 0;
                        process.cpu_last_used = proc_time.rtime;
                        process_table.insert (pids[i], (owned) process);
                    } else {
                        unowned Process process = process_table[pids[i]];
                        process.cpu_load = ((double) (proc_time.rtime - process.cpu_last_used)) / (cpu_data.total - cpu_last_total);
                        process.alive = true;
                        process.cpu_last_used = proc_time.rtime;
                    }
                }

                foreach (unowned Process process in process_table.get_values ()) {
                    if (process.alive == false) {
                        process_table.remove (process.pid);
                    }
                }

                cpu_last_used = used;
                cpu_last_total = cpu_data.total;

                return true;
            });
        }
    }

    public class ProcessWidget : Gtk.Grid {
        static Gtk.SizeGroup name_size_group = null;
        static Gtk.SizeGroup percent_size_group = null;

        void ensure_size_groups () {
            if (name_size_group == null) {
                name_size_group = new Gtk.SizeGroup (Gtk.SizeGroupMode.HORIZONTAL);
                percent_size_group = new Gtk.SizeGroup (Gtk.SizeGroupMode.HORIZONTAL);
            }
        }

        public ProcessWidget (string cmdline, double load) {
            orientation = Gtk.Orientation.HORIZONTAL;

            ensure_size_groups ();

            var label = new Gtk.Label (cmdline) {
                halign = Gtk.Align.START
            };
            name_size_group.add_widget (label);
            add (label);

            var progress = new Gtk.ProgressBar () {
                fraction = load,
                hexpand = true
            };
            add (progress);

            label = new Gtk.Label ("%d%%".printf ((int) (100 * load))) {
                margin_right = 20,
                margin_left = 20,
                halign = Gtk.Align.START
            };
            percent_size_group.add_widget (label);
            add (label);

            show_all ();
        }
    }

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

            var monitor = new SystemMonitor ();

            Timeout.add (200, () => {
                level_bar.set_value (monitor.cpu_load);
                return true;
            });

            Timeout.add_seconds (1, () => {
                label.set_text ("%d%%".printf ((int) (monitor.cpu_load * 100)));

                proc_list.foreach ((widget) => { widget.destroy (); });
                foreach (unowned Process process in monitor.process_table.get_values ()) {
                    var load = process.cpu_load / monitor.cpu_load;
                    var proc_widget = new ProcessWidget (process.cmdline, load);
                    proc_widget.set_data ("sort_id", (int) (100 * load));
                    proc_list.add (proc_widget);
                }

                return true;
            });
        }
    }
}
