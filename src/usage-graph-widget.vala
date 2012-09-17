/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

namespace Usage {

    public class GraphWidget : Gtk.DrawingArea {
        public int num_points { get; set; default = 40; }

        List<double?> points = null;

        public void push (double p) {
            points.append (p);
            if (points.length () > num_points) {
                points.delete_link (points);
            }
            queue_draw ();
        }

        protected override bool draw (Cairo.Context cr) {
            double width = get_allocated_width ();
            double height = get_allocated_height ();

            cr.set_line_width (1.0);
            cr.set_source_rgb (1, 1, 1);
            cr.rectangle (0, 0, width, height);
            cr.fill_preserve ();
            cr.set_source_rgb (0.5, 0.5, 0.5);
            cr.stroke ();

            if (points == null) {
                return false;
            }

            var delta_x = width / num_points;

            double x_offset = (num_points - points.length ()) * delta_x;

            cr.set_source_rgb (0, 0.3, 1);
            cr.move_to (x_offset, (1 - points.data) * height);
            unowned List<double?> iter = points.next;
            int i = 1;
            while (iter != null) {
                cr.curve_to (x_offset + (i - 0.5) * delta_x, (1 - iter.prev.data) * height,
                             x_offset + (i - 0.5) * delta_x, (1 - iter.data) * height,
                             x_offset + i * delta_x, (1 - iter.data) * height);
                iter = iter.next;
                i++;
            }
            cr.stroke ();

            return false;
        }
    }
}
