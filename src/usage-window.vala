/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public enum UIView {
        CPU,
        MEMORY,
        STORAGE,
        NETWORK,
        BATTERY,
        N_VIEWS;

        public static UIView[] all () {
            return { CPU, MEMORY, STORAGE, NETWORK, BATTERY };
        }
    }

    [GtkTemplate (ui = "/org/gnome/usage/ui/usage-window.ui")]
    public class Window : Gtk.ApplicationWindow {

        [GtkChild]
        private Gtk.HeaderBar header_bar;
        [GtkChild]
        private Gtk.Stack stack;
        [GtkChild]
        private Gtk.StackSwitcher stack_switcher;

        public View[] views;

        [GtkCallback]
        private void close_button_clicked (Gtk.Button button)
        {
            Gdk.Event event;

            event = new Gdk.Event (Gdk.EventType.DELETE);

            event.any.window = this.get_window ();
            event.any.send_event = 1;

            Gtk.main_do_event (event);
        }

        public Window (Application app) {
            Object (application: app);

            set_default_size (800, 550);

            views = new View[UIView.N_VIEWS];
            views[UIView.CPU]     = new CPUView ();
            views[UIView.MEMORY]  = new MemoryView ();
            views[UIView.STORAGE] = new StorageView ();
            views[UIView.NETWORK] = new NetworkView ();
            views[UIView.BATTERY] = new BatteryView ();

            stack_switcher.stack = stack;

            foreach (var view in UIView.all ()) {
                stack.add_titled  (views[view], views[view].name, views[view].name);
            };

            show_all ();
        }
    }
}
