#include <iostream>
#include <vector>

#include <gtk/gtk.h>

const char* cstr;
int size;

static void activate(GtkApplication* app, gpointer user_data)
{
	GtkWidget *window;
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "win");
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

	// load image
	GdkPixbufLoader* loader;
	loader = gdk_pixbuf_loader_new();
	gdk_pixbuf_loader_write(loader, (const guchar*) cstr, size, NULL);
	gdk_pixbuf_loader_close(loader, NULL);

	GdkPixbuf* pixbuf;
	pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

	GtkWidget *picture;
	picture = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add(GTK_CONTAINER(window), picture);
	
	gtk_widget_show_all(window);
}


int main (int argc, char **argv)
{
	// read data into char vector
	std::vector <char> v;
	
	char c;
	while (std::cin.get(c))
	{
		v.push_back(c);
		size++;
	}
	
	// dont use cstr because it might contain null bytes
	// which chops the string early

	cstr = &v[0];
	std::cout << "Size: " << size << "\n";
	
	GtkApplication *app;
	int status;

	app = gtk_application_new("ml.rhkr.ieditor", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}


