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
            content.add (new Usage.Topbar (this));

            var notebook = new Gtk.Notebook () { show_tabs = false, vexpand = true };
            var style_context = notebook.get_style_context ();
            style_context.add_class (Gtk.STYLE_CLASS_VIEW);
            style_context.add_class ("content-view");

            foreach (var view in UIView.all ()) {
                notebook.append_page (views[view].content);
            };
            content.add (notebook);

            notify["current-view"].connect (() => {
                notebook.page = current_view;
            });

            current_view = UIView.CPU;

            add (content);
            show_all ();
        }
    }
}
