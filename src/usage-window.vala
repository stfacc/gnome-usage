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

    public class Window : Gtk.ApplicationWindow {

        public UIView current_view { get; set; }

        public View[] views;

        public Window (Application app) {
            Object (application: app);

            title = _("Usage");
            set_default_size (800, 550);
            hide_titlebar_when_maximized = true;

            views = new View[UIView.N_VIEWS];
            views[UIView.CPU]     = new CPUView ();
            views[UIView.MEMORY]  = new MemoryView ();
            views[UIView.STORAGE] = new StorageView ();
            views[UIView.NETWORK] = new NetworkView ();
            views[UIView.BATTERY] = new BatteryView ();

            var content = new Gtk.Grid () { orientation = Gtk.Orientation.VERTICAL };
            var header_bar = new Gtk.HeaderBar ();
            var stack_switcher = new Gtk.StackSwitcher ();
            header_bar.custom_title = stack_switcher;

            content.add (header_bar);

            var stack = new Gtk.Stack ();

            foreach (var view in UIView.all ()) {
                stack.add_titled  (views[view].content, views[view].name, views[view].name);
            };
            content.add (stack);

            stack_switcher.stack = stack;

            add (content);
            show_all ();
        }
    }
}
