/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    [Compact]
    class SizeGroups {
        internal Gtk.SizeGroup name;
        internal Gtk.SizeGroup percent;
    }

    public class ElementWidget : Gtk.ListBoxRow {

        public int sort_id;

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
            var grid = new Gtk.Grid () { orientation = Gtk.Orientation.HORIZONTAL };
            is_headline = headline;

            var label = new Gtk.Label (null) {
                halign = Gtk.Align.START
            };
            label.set_markup (text);
            label.set_ellipsize (Pango.EllipsizeMode.END);
            var box = new Gtk.EventBox ();
            box.set_size_request (150, -1);
            box.add (label);
            grid.add (box);

            int margin = headline ? 20 : 5;
            var progress = new Gtk.LevelBar () {
                value = load,
                margin_top = margin,
                margin_bottom = margin,
                hexpand = true
            };
            grid.add (progress);

            label = new Gtk.Label ("%d%%".printf ((int) (100 * load))) {
                halign = Gtk.Align.END
            };
            box = new Gtk.EventBox ();
            box.set_size_request (50, -1);
            box.add (label);
            grid.add (box);

            add (grid);

            show_all ();
        }
    }

    public class ElementList : Gtk.ListBox {

        void update_header (Gtk.ListBoxRow row, Gtk.ListBoxRow? before_row) {
            if (before_row != null && (before_row as ElementWidget).is_headline && !(row as ElementWidget).is_headline) {
                var header = new Gtk.EventBox ();
                header.set_size_request (-1, 40);
                row.set_header (header);
            } else {
                row.set_header (null);
            }
        }

        public ElementList () {
            set_selection_mode (Gtk.SelectionMode.NONE);
            set_header_func (update_header);
        }
    }
}
