/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class MainLevelBar : Gtk.LevelBar {

        public const int NUM_LEVELS = 20;

        public MainLevelBar () {
            orientation = Gtk.Orientation.VERTICAL;
            mode = Gtk.LevelBarMode.DISCRETE;
            min_value = 0;
            max_value = NUM_LEVELS;

            inverted = true;
            set_size_request (40, -1);

            get_style_context ().add_class ("main-level-bar");
        }
    }
}
