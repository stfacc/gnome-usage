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
        private Gtk.Button back_button;
        [GtkChild]
        private Gtk.Stack stack;
        [GtkChild]
        private Gtk.StackSwitcher stack_switcher;

        public View[] views;

        public Window (Application app) {
            Object (application: app);

            set_default_size (800, 550);

            var back_button_image = back_button.get_child () as Gtk.Image;
            if (get_direction () == Gtk.TextDirection.LTR) {
                back_button_image.icon_name = "go-previous-symbolic";
            } else {
                back_button_image.icon_name = "go-previous-rtl-symbolic";
            }

            back_button.clicked.connect (() => {
                (stack.visible_child as View).go_back ();
            });

            views = new View[UIView.N_VIEWS];
            views[UIView.CPU]     = new CPUView ();
            views[UIView.MEMORY]  = new MemoryView ();
            views[UIView.STORAGE] = new StorageView ();
            views[UIView.NETWORK] = new NetworkView ();
            views[UIView.BATTERY] = new BatteryView ();

            stack_switcher.stack = stack;

            foreach (var view in UIView.all ()) {
                stack.add_titled  (views[view], views[view].name, views[view].name);
                views[view].mode_changed.connect ((title) => {
                    if (title == null) {
                        header_bar.custom_title = stack_switcher;
                        back_button.hide ();
                    } else {
                        header_bar.custom_title = null;
                        header_bar.title = title;
                        back_button.show ();
                    }
                });
            };

            show ();
        }
    }
}
