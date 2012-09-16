/* -*- indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    [Compact]
    class SizeGroups {
        internal Gtk.SizeGroup name;
        internal Gtk.SizeGroup percent;
    }

    public class ElementWidget : Gtk.Grid {

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

        public ElementWidget (Usage.View view, string text, double load) {
            orientation = Gtk.Orientation.HORIZONTAL;

            unowned SizeGroups size_groups = get_size_groups (view.name);

            var label = new Gtk.Label (text) {
                halign = Gtk.Align.START
            };
            size_groups.name.add_widget (label);
            add (label);

            var progress = new Gtk.LevelBar () {
                value = load,
                margin_top = 5,
                margin_bottom = 5,
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

        public ElementWidget.for_headline (Usage.View view, string text, double load) {
            orientation = Gtk.Orientation.HORIZONTAL;

            unowned SizeGroups size_groups = get_size_groups (view.name);

            var label = new Gtk.Label (text) {
                halign = Gtk.Align.START
            };
            size_groups.name.add_widget (label);
            add (label);

            var progress = new Gtk.LevelBar () {
                value = load,
                margin_top = 20,
                margin_bottom = 20,
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
}
