/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    [Compact]
    class SizeGroups {
        internal Gtk.SizeGroup name;
        internal Gtk.SizeGroup percent;
    }

    public class ElementWidget : Gtk.Grid {

        public bool is_headline { get; private set; }

        static HashTable<string, SizeGroups> size_groups_table;
        
        unowned SizeGroups get_size_groups (string name) {
            if (size_groups_table == null) {
                size_groups_table = new HashTable<string, SizeGroups> (str_hash, str_equal);
            }

            if (!(name in size_groups_table)) {
                var size_groups = new SizeGroups ();
                size_groups.name = new Gtk.SizeGroup (Gtk.SizeGroupMode.HORIZONTAL);
                size_groups.percent = new Gtk.SizeGroup (Gtk.SizeGroupMode.HORIZONTAL);
                size_groups_table[name] = (owned) size_groups;
            }

            return size_groups_table[name];
        }

        public ElementWidget (Usage.View view, string text, double load, bool headline) {
            orientation = Gtk.Orientation.HORIZONTAL;
            is_headline = headline;

            unowned SizeGroups size_groups = get_size_groups (view.name);

            var label = new Gtk.Label (text) {
                halign = Gtk.Align.START
            };

            label.set_size_request (150, -1);
            label.set_ellipsize (Pango.EllipsizeMode.END);
            add (label);

            int margin = headline ? 20 : 5;
            var progress = new Gtk.LevelBar () {
                value = load,
                margin_top = margin,
                margin_bottom = margin,
                hexpand = true
            };
            add (progress);

            label = new Gtk.Label ("%d%%".printf ((int) (100 * load))) {
                margin_right = 20,
                margin_left = 20,
                halign = Gtk.Align.START
            };
            size_groups.percent.add_widget (label);
            add (label);

            show_all ();
        }
    }

    public class ElementList : Egg.ListBox {

        void update_separator (ref Gtk.Widget? separator, Gtk.Widget widget, Gtk.Widget? before) {
            if (before != null && (before as ElementWidget).is_headline && !(widget as ElementWidget).is_headline) {
                separator = new Gtk.EventBox ();
                separator.set_size_request (-1, 40);
                separator.show ();
            } else {
               separator = null;
            }
        }

        public ElementList () {
            set_separator_funcs (update_separator);
        }
    }
}
