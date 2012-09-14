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
}
