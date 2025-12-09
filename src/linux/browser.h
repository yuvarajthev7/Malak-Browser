#pragma once
#include <gtk/gtk.h>
#include <webkit/webkit.h>

class BrowserApp {
public:
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *url_bar;
    GtkWidget *btn_star;
    GtkWidget *engine_dropdown;

    void create_window(GtkApplication *app);
    void create_tab(const char *url);

    // Static callbacks need access to the instance
    static BrowserApp* instance;
};
