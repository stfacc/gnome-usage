/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    [GtkTemplate (ui = "/org/gnome/usage/ui/usage-element-row.ui")]
    public class ElementRow : Gtk.ListBoxRow {

        [GtkChild]
        private Gtk.Label start_label;
        [GtkChild]
        private Gtk.LevelBar level_bar;
        [GtkChild]
        private Gtk.Label end_label;

        public int sort_id;

        public bool is_headline { get; private set; }

        public ElementRow (string text, double load, bool headline) {
            is_headline = headline;

            start_label.set_markup (text);
            start_label.set_size_request (150, -1);

            int margin = headline ? 20 : 5;
            level_bar.value = load;
            level_bar.margin_top = margin;
            level_bar.margin_bottom = margin;

            end_label.label = "%d%%".printf ((int) (100 * load));
            end_label.set_size_request (50, -1);
        }
    }

    public class ElementList : Gtk.ListBox {

        void update_header (Gtk.ListBoxRow row, Gtk.ListBoxRow? before_row) {
            if (before_row != null && (before_row as ElementRow).is_headline && !(row as ElementRow).is_headline) {
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
