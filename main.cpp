#include <iostream>
#include <vector>

#include <gtk/gtk.h>
#include <gio/gunixoutputstream.h>

typedef struct mouse_info {
	double x;
	double y;
	bool reset;
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
	cairo_line_to(cr, x, y);
	cairo_stroke(cr);
	cairo_destroy(cr);

	gdouble tlx = mh.x > x ? x : mh.x;
	gdouble tly = mh.y > y ? y : mh.y;
	gint w = abs(mh.x - x);
	gint h = abs(mh.y - y);
	gtk_widget_queue_draw_area(widget, tlx, tly, w, h);
	gtk_widget_queue_draw(widget);
	mh.x = x;
	mh.y = y;
}

static gboolean button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->button == GDK_BUTTON_PRIMARY)
	{
		draw_brush(widget, event->x, event->y);
	}
	else if (event->button == GDK_BUTTON_SECONDARY)
	{
		GdkPixbuf* buf = gdk_pixbuf_get_from_surface(gtk.surface, 0, 0, ss.width, ss.height);
		GOutputStream *ostream = g_unix_output_stream_new(1, true);
		gdk_pixbuf_save_to_stream(buf, ostream, "png", NULL, NULL);
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

static gboolean configure_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GdkPixbuf *buf = (GdkPixbuf*) data;
	gtk.surface = gdk_cairo_surface_create_from_pixbuf(buf, 1, NULL);
	return false;
}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	cairo_set_source_surface(cr, gtk.surface, 0, 0);
	cairo_paint(cr);
	return false;
}

static void set_sizes(GdkPixbufLoader *loader, gint width, gint height, gpointer data)
{
	gtk_widget_set_size_request(gtk.drawing_area, 1, 1);
	ss.width = width;
	ss.height = height;
}

static void activate(GtkApplication* app, gpointer data)
{
	gtk.window = gtk_application_window_new(app);
	gtk_window_set_default_size(GTK_WINDOW(gtk.window), 1, 1);

	GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
	g_signal_connect(G_OBJECT(loader), "size_prepared", G_CALLBACK(set_sizes), NULL);
	gdk_pixbuf_loader_write(loader, (const guchar*) data, ss.bytes, NULL);
	gdk_pixbuf_loader_close(loader, NULL);
	GdkPixbuf* buf = gdk_pixbuf_loader_get_pixbuf(loader);
	
	gtk.drawing_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(gtk.window), gtk.drawing_area);

	g_signal_connect(G_OBJECT(gtk.drawing_area), "draw", G_CALLBACK(draw_cb), NULL);
	g_signal_connect(G_OBJECT(gtk.drawing_area), "configure-event", G_CALLBACK(configure_cb), buf);
	g_signal_connect(G_OBJECT(gtk.drawing_area), "motion-notify-event", G_CALLBACK(motion_notify_cb), NULL);
	g_signal_connect(G_OBJECT(gtk.drawing_area), "button-press-event", G_CALLBACK(button_press_cb), NULL);
	gtk_widget_set_events(gtk.drawing_area, GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK);

	gtk_widget_show_all(gtk.window);
}

int main (int argc, char **argv)
{
	char c;
	std::vector<char> v;
	while (std::cin.get(c))
	{
		v.push_back(c);
		ss.bytes++;
	}
	char* data = &v[0];

	GtkApplication *app = gtk_application_new("ml.rhkr.ieditor", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), data);

	return g_application_run(G_APPLICATION(app), argc, argv);
}
