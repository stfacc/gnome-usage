/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public enum TopbarMode {
        OVERVIEW,
        DETAIL
    }

    class TopbarButton : Gtk.RadioButton {
        static Gtk.RadioButton? group_ = null;

        string text;
        new Gtk.Label label;

        public TopbarButton (string text_) {
            Object (group: group_);
            set_mode (false);
            text = text_;
            label = new Gtk.Label (text);
            add (label);
            if (group_ == null) {
                group_ = this;
            }

            toggled.connect (() => {
                if (get_active ()) {
                    label.set_markup ("<b>%s</b>".printf (text));
                } else {
                    label.set_markup ("%s".printf (text));
                }
            });
        }
    }

    public class Topbar : Gtk.Box {

        public UIView current_view { get; set; }

        public Topbar (Usage.Window window) {
            var notebook = new Gtk.Notebook () { show_tabs = false };
            add (notebook);

            var tb = new Gtk.Toolbar () { hexpand = true };
            tb.get_style_context ().add_class (Gtk.STYLE_CLASS_MENUBAR);
            notebook.append_page (tb);

            var left_item = new Gtk.ToolItem ();
            tb.insert (left_item, -1);

            var center_item = new Gtk.ToolItem ();
            center_item.set_expand (true);
            tb.insert (center_item, -1);

            var right_item = new Gtk.ToolItem ();
            tb.insert (right_item, -1);

            var size_group = new Gtk.SizeGroup (Gtk.SizeGroupMode.HORIZONTAL);
            size_group.add_widget (left_item);
            size_group.add_widget (right_item);

            var bbox = new Gtk.ButtonBox (Gtk.Orientation.HORIZONTAL);
            bbox.layout_style = Gtk.ButtonBoxStyle.CENTER;
            bbox.get_style_context ().add_class (Gtk.STYLE_CLASS_LINKED);
            center_item.add (bbox);

            Gtk.RadioButton[] buttons = {};
            foreach (var view_id in UIView.all ()) {
                var view = window.views[view_id];

                var button = new TopbarButton (view.name);
                button.toggled.connect (() => {
                    if (button.get_active ()) {
                        current_view = view_id;
                    }
                });
                bbox.add (button);
                buttons += button;

                view.toggled_mode.connect ((mode) => {
                    if (mode == TopbarMode.DETAIL) {
                        notebook.remove_page (TopbarMode.DETAIL);
                        notebook.append_page (view.topbar_detail_content);
                    }
                    notebook.page = mode;
                });
            }

            notify["current-view"].connect (() => {
                buttons[current_view].set_active (true);
            });

            bind_property ("current-view", window, "current-view",
                           BindingFlags.BIDIRECTIONAL | BindingFlags.SYNC_CREATE);
        }
    }
}
