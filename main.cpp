#include <iostream>
#include <vector>

#include <gtk/gtk.h>
#include <gio/gunixoutputstream.h>

typedef struct mouse_info {
	double x;
	double y;
	bool reset;
	int color_index = 0;
} mouse_info;

typedef struct ss_info {
	int width;
	int height;
	int bytes;
} ss_info;

typedef struct gtk_info {
	cairo_surface_t *surface;
	GtkWidget *window;
	GtkWidget *drawing_area;
} gtk_info;

static mouse_info mh;
static ss_info ss;
static gtk_info gtk;
static std::vector<char> v;
static double colors[][3] = {
	{1, 1, 1},
	{0, 0, 1},
	{1, 0, 0},
	{0, 1, 0},
	{0, 0, 0}
};
static int num_colors = 5;

static void clean_and_close()
{
	gtk_window_close(GTK_WINDOW(gtk.window));
}

static void draw_brush(GtkWidget *widget, gdouble x, gdouble y)
{
	cairo_t *cr = cairo_create(gtk.surface);
	if (mh.reset)
	{
		cairo_move_to(cr, x, y);
		mh.reset = false;
	}
	else
	{
		cairo_move_to(cr, mh.x, mh.y);
	}
	double *c = colors[mh.color_index];
	cairo_set_source_rgb(cr, c[0], c[1], c[2]);
	cairo_line_to(cr, x, y);
	cairo_stroke(cr);
	cairo_destroy(cr);

	gtk_widget_queue_draw(widget);

	mh.x = x;
	mh.y = y;
}

static gboolean button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// draw button (lmb)
	if (event->button == GDK_BUTTON_PRIMARY)
	{
		draw_brush(widget, event->x, event->y);
	}
	// output and quit button (rmb)
	else if (event->button == GDK_BUTTON_SECONDARY)
	{
		GdkPixbuf* buf = gdk_pixbuf_get_from_surface(gtk.surface, 0, 0, ss.width, ss.height);
		GOutputStream *ostream = g_unix_output_stream_new(1, true);
		gdk_pixbuf_save_to_stream(buf, ostream, "png", NULL, NULL);

		clean_and_close();
	}
	return true;
}

static gboolean motion_notify_cb(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	if (event->state & GDK_BUTTON1_MASK)
	{
		draw_brush(widget, event->x, event->y);
	}
	else
	{
		mh.reset = true;
	}
	return true;
}

static gboolean pick_color(GtkWidget  *widget, GdkEvent *event, gpointer data)
{
	if (event->scroll.direction == GDK_SCROLL_UP)
	{
		mh.color_index = (mh.color_index + 1) % num_colors;
	}
	else if (event->scroll.direction == GDK_SCROLL_DOWN)
	{
		mh.color_index = (mh.color_index + num_colors + 2) % num_colors;
	}
	double *c = colors[mh.color_index];
	GdkRGBA col;
	col.red = c[0];
	col.green = c[1];
	col.blue = c[2];
	col.alpha = 1;
	gtk_widget_override_background_color(GTK_WIDGET(gtk.window), GTK_STATE_FLAG_NORMAL, &col);
	return true;
}

static gboolean configure_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GdkPixbuf *buf = (GdkPixbuf*) data;
	gtk.surface = gdk_cairo_surface_create_from_pixbuf(buf, 1, NULL);
	return true;
}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	cairo_set_source_surface(cr, gtk.surface, 0, 0);
	cairo_paint(cr);
	return true;
}

static void set_sizes(GdkPixbufLoader *loader, gint width, gint height, gpointer data)
{
	gtk_window_set_default_size(GTK_WINDOW(gtk.window), width, height);
	ss.width = width;
	ss.height = height;
}

static void activate(GtkApplication *app, gpointer data)
{
	gtk.window = gtk_application_window_new(app);
	gtk_window_set_resizable(GTK_WINDOW(gtk.window), false);
	gtk_container_set_border_width(GTK_CONTAINER(gtk.window), 5);

	GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
	g_signal_connect(G_OBJECT(loader), "size_prepared", G_CALLBACK(set_sizes), NULL);
	gdk_pixbuf_loader_write(loader, (const guchar*) data, ss.bytes, NULL);
	gdk_pixbuf_loader_close(loader, NULL);
	v.clear();
	v.shrink_to_fit();
	GdkPixbuf* buf = gdk_pixbuf_loader_get_pixbuf(loader);
	
	gtk.drawing_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(gtk.window), gtk.drawing_area);

	g_signal_connect(G_OBJECT(gtk.drawing_area), "draw", G_CALLBACK(draw_cb), NULL);
	g_signal_connect(G_OBJECT(gtk.drawing_area), "configure-event", G_CALLBACK(configure_cb), buf);
	g_signal_connect(G_OBJECT(gtk.drawing_area), "motion-notify-event", G_CALLBACK(motion_notify_cb), NULL);
	g_signal_connect(G_OBJECT(gtk.drawing_area), "button-press-event", G_CALLBACK(button_press_cb), NULL);
	g_signal_connect(G_OBJECT(gtk.drawing_area), "scroll-event", G_CALLBACK(pick_color), NULL);
	gtk_widget_set_events(gtk.drawing_area, GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);
	gtk_widget_show_all(gtk.window);
}

int main (int argc, char **argv)
{
	char c;
	while (std::cin.get(c))
	{
		v.push_back(c);
		ss.bytes++;
	}
	char* data = &v[0];

	GtkApplication *app = gtk_application_new("ml.rhkr.ieditor", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), data);

	int status = g_application_run(G_APPLICATION(app), argc, argv);

	return status;
}
