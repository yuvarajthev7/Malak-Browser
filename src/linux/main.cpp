#include <gtk/gtk.h>
#include "browser.h"
#include "adblock.h"
#include "storage.h"

static void on_activate(GtkApplication *app, gpointer) {
    setup_adblock();
    load_bookmarks();
    BrowserApp *browser = new BrowserApp();
    browser->create_window(app);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.malak.browser", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Perform cleanup after the window closes
    perform_exit_cleanup();

    return status;
}
