/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class StorageLocation {
        public string name;
        public Icon icon;
        public int max_size;
        public int size;

        public bool is_device = false;

        public StorageLocation.computer () {
            name = _("Joel's computer");
            icon = new ThemedIcon ("computer");
            max_size = 10;
            size = 8;
            is_device = true;
        }

        public StorageLocation.photos () {
            name = _("Photos");
            icon = new ThemedIcon ("folder-pictures");
            max_size = 10;
            size = 3;
        }

        public StorageLocation.documents () {
            name = _("Documents");
            icon = new ThemedIcon ("folder-documents");
            max_size = 10;
            size = 1;
        }

        public StorageLocation.trash () {
            name = _("Trash");
            icon = new ThemedIcon ("user-trash");
            max_size = 10;
            size = 2;
        }
    }

    [GtkTemplate (ui = "/org/gnome/usage/ui/usage-storage-view.ui")]
    public class StorageView : View {

        [GtkChild]
        StorageList list_box;

        StorageModel model;

        public override void go_back () {
            list_box.go_back ();
            update_mode ();
        }

        void update_mode () {
            if (list_box.root_path.get_depth () > 1) {
                Gtk.TreeIter iter;
                string name;
                model.get_iter (out iter, list_box.root_path);
                model.get (iter, Columns.NAME, out name, -1);
                mode_changed (name);
            } else {
                mode_changed (null);
            }
        }

        public StorageView () {

            model = new StorageModel ();

            populate_model ();

            list_box.model = model;

            list_box.row_activated.connect_after (() => {
                update_mode ();
            });
        }

        void populate_model () {
            // Add some dummy data for now...

            Gtk.TreeIter parent_iter, iter;
            model.append (out parent_iter, null);

            model.append (out iter, parent_iter);
            model.set (iter,
                       Columns.ICON, new ThemedIcon ("user-trash"),
                       Columns.NAME, _("Trash"),
                       Columns.MAX_SIZE, 10,
                       Columns.SIZE, 1);

            model.append (out iter, parent_iter);
            model.set (iter,
                       Columns.ICON, new ThemedIcon ("folder-music"),
                       Columns.NAME, _("Music"),
                       Columns.MAX_SIZE, 10,
                       Columns.SIZE, 5);

            model.append (out iter, parent_iter);
            model.set (iter,
                       Columns.ICON, new ThemedIcon ("folder-pictures"),
                       Columns.NAME, _("Photos"),
                       Columns.MAX_SIZE, 10,
                       Columns.SIZE, 5);

            model.append (out iter, parent_iter);
            model.set (iter,
                       Columns.ICON, new ThemedIcon ("folder-documents"),
                       Columns.NAME, _("Documents"),
                       Columns.MAX_SIZE, 10,
                       Columns.SIZE, 5);

            model.append (out iter, parent_iter);
            model.set (iter,
                       Columns.ICON, new ThemedIcon ("folder-videos"),
                       Columns.NAME, _("Videos"),
                       Columns.MAX_SIZE, 10,
                       Columns.SIZE, 6);

            parent_iter = iter;

            model.append (out iter, parent_iter);
            model.set (iter,
                       Columns.ICON, new ThemedIcon ("folder"),
                       Columns.NAME, "Movies",
                       Columns.MAX_SIZE, 10,
                       Columns.SIZE, 3);

            model.append (out iter, parent_iter);
            model.set (iter,
                       Columns.ICON, new ThemedIcon ("folder"),
                       Columns.NAME, "Anime",
                       Columns.MAX_SIZE, 10,
                       Columns.SIZE, 2);

            parent_iter = iter;

            model.append (out iter, parent_iter);
            model.set (iter,
                       Columns.ICON, new ThemedIcon ("video"),
                       Columns.NAME, "Howl's moving castle",
                       Columns.MAX_SIZE, 10,
                       Columns.SIZE, 1);
        }
    }
}
