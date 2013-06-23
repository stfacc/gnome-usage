/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public enum Columns {
        ICON,
        NAME,
        MAX_SIZE,
        SIZE
    }

    public class StorageModel : Gtk.TreeStore {
        public StorageModel () {
            set_column_types (new Type[] {
                typeof (Icon),    // ICON
                typeof (string),  // NAME
                typeof (int),     // MAX_SIZE
                typeof (int),     // SIZE
            });
        }
    }

    [GtkTemplate (ui = "/org/gnome/usage/ui/usage-storage-row.ui")]
    class StorageRow : Gtk.ListBoxRow {

        [GtkChild]
        Gtk.Image image;
        [GtkChild]
        Gtk.Label name_label;
        [GtkChild]
        Gtk.Label bar_label;
        [GtkChild]
        Gtk.Label right_label;
        [GtkChild]
        Gtk.LevelBar levelbar;

        public Gtk.TreeIter iter;

        public StorageRow (Icon? icon, string name, int max_size, int size, Gtk.TreeIter iter) {
            image.gicon = icon;
            name_label.label = "<b>%s</b>".printf (name);
            levelbar.max_value = max_size;
            levelbar.value = size;
            right_label.label = "<b>%d</b> GB".printf (size);

            this.iter = iter;
        }
    }

    [GtkTemplate (ui = "/org/gnome/usage/ui/usage-storage-list.ui")]
    public class StorageList : Gtk.ListBox {

        public Gtk.TreePath root_path { get; private set; }

        Gtk.TreeStore model_;
        public Gtk.TreeStore model {
            set {
                model_ = value;
                root_path = new Gtk.TreePath.first ();
                update ();
            }
            get {
                return model_;
            }
        }

        construct {
            set_header_func (update_header);
        }

        public void go_back () {
            root_path.up ();
            update ();
        }

        public override void row_activated (Gtk.ListBoxRow row) {
            root_path = model.get_path ((row as StorageRow).iter);
            update ();
        }

        void update_header (Gtk.ListBoxRow row, Gtk.ListBoxRow? before_row) {
            if (before_row != null && row.get_header () == null) {
                row.set_header (new Gtk.Separator (Gtk.Orientation.HORIZONTAL));
            } else {
                row.set_header (null);
            }
        }

        void update () {
            this.foreach ((widget) => { widget.destroy (); });

            string name;
            Icon icon;
            int max_size;
            int size;

            Gtk.TreeIter root_iter, iter;
            if (!model.get_iter (out root_iter, root_path)) {
                return;
            }

            if (!model.iter_children (out iter, root_iter)) {
                return;
            }

            do {
                model.get (iter,
                           Columns.ICON, out icon,
                           Columns.NAME, out name,
                           Columns.MAX_SIZE, out max_size,
                           Columns.SIZE, out size,
                           -1);
                add (new StorageRow (icon, name, max_size, size, iter));
            } while (model.iter_next (ref iter));
        }
    }
}