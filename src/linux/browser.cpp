#include "browser.h"
#include "adblock.h"
#include "storage.h"
#include <string>
#include <iostream>

BrowserApp* BrowserApp::instance = nullptr;

static WebKitWebView* get_current_view() {
    if (!BrowserApp::instance || !BrowserApp::instance->notebook) return nullptr;
    int page = gtk_notebook_get_current_page(GTK_NOTEBOOK(BrowserApp::instance->notebook));
    if (page == -1) return nullptr;
    return WEBKIT_WEB_VIEW(gtk_notebook_get_nth_page(GTK_NOTEBOOK(BrowserApp::instance->notebook), page));
}

// --- History UI ---

static void refresh_history_text(GtkWidget *text_view) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, get_history_content().c_str(), -1);
}

static void on_clear_click(GtkButton*, gpointer data) {
    // data is actually an integer packed into the pointer (0, 1, or 2)
    int timeframe = GPOINTER_TO_INT(data);
    clear_history(timeframe);
    // Note: We can't easily refresh the text view here without passing more complex data structure.
    // For now, user closes/reopens window to see update, or we accept the limitation.
}

static void on_toggle_exit(GtkCheckButton *btn, gpointer) {
    set_clear_on_exit(gtk_check_button_get_active(btn));
}

static void show_history_window() {
    GtkWidget *win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), "History Manager");
    gtk_window_set_default_size(GTK_WINDOW(win), 500, 500);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_window_set_child(GTK_WINDOW(win), box);

    // Buttons Container
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(box), btn_box);

    GtkWidget *b1 = gtk_button_new_with_label("Last Hour");
    GtkWidget *b2 = gtk_button_new_with_label("Today");
    GtkWidget *b3 = gtk_button_new_with_label("Clear All");
    gtk_widget_add_css_class(b3, "destructive-action");

    // Pass integer constants safely
    g_signal_connect(b1, "clicked", G_CALLBACK(on_clear_click), GINT_TO_POINTER(1));
    g_signal_connect(b2, "clicked", G_CALLBACK(on_clear_click), GINT_TO_POINTER(2));
    g_signal_connect(b3, "clicked", G_CALLBACK(on_clear_click), GINT_TO_POINTER(0));

    gtk_box_append(GTK_BOX(btn_box), b1);
    gtk_box_append(GTK_BOX(btn_box), b2);
    gtk_box_append(GTK_BOX(btn_box), b3);

    // Auto-Clear Toggle
    GtkWidget *chk = gtk_check_button_new_with_label("Auto-Clear on Exit");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(chk), get_clear_on_exit());
    g_signal_connect(chk, "toggled", G_CALLBACK(on_toggle_exit), NULL);
    gtk_box_append(GTK_BOX(box), chk);

    // Divider
    gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // Content View
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    GtkWidget *text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text)), get_history_content().c_str(), -1);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), text);
    gtk_box_append(GTK_BOX(box), scroll);

    gtk_window_present(GTK_WINDOW(win));
}

static void on_bookmark_row_activated(GtkListBox*, GtkListBoxRow* row, gpointer win) {
    const char *url = (const char*)g_object_get_data(G_OBJECT(row), "url");
    if(url) BrowserApp::instance->create_tab(url);
    gtk_window_destroy(GTK_WINDOW(win));
}

static void show_bookmarks_window() {
    GtkWidget *win = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(win), "Bookmarks");
    gtk_window_set_default_size(GTK_WINDOW(win), 400, 500);

    GtkWidget *list = gtk_list_box_new();
    const auto& bms = get_bookmarks();

    for(const auto& b : bms) {
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *lbl = gtk_label_new(b.title.c_str());
        gtk_widget_set_margin_start(lbl, 10);
        gtk_widget_set_margin_top(lbl, 10);
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), lbl);

        g_object_set_data(G_OBJECT(row), "url", (gpointer)b.url.c_str());
        gtk_list_box_append(GTK_LIST_BOX(list), row);
    }

    g_signal_connect(list, "row-activated", G_CALLBACK(on_bookmark_row_activated), win);
    gtk_window_set_child(GTK_WINDOW(win), list);
    gtk_window_present(GTK_WINDOW(win));
}

// --- Standard Callbacks ---

static void on_nav_back(GtkButton*, gpointer) {
    WebKitWebView *v = get_current_view();
    if(v && webkit_web_view_can_go_back(v)) webkit_web_view_go_back(v);
}
static void on_nav_fwd(GtkButton*, gpointer) {
    WebKitWebView *v = get_current_view();
    if(v && webkit_web_view_can_go_forward(v)) webkit_web_view_go_forward(v);
}
static void on_nav_refresh(GtkButton*, gpointer) {
    WebKitWebView *v = get_current_view();
    if(v) webkit_web_view_reload(v);
}
static void on_nav_home(GtkButton*, gpointer) {
    WebKitWebView *v = get_current_view();
    if(v) webkit_web_view_load_uri(v, "https://google.com");
}

static void on_pip_click(GtkButton*, gpointer) {
    WebKitWebView *v = get_current_view();
    if(!v) return;
    const char *js = "(function() { var v = Array.from(document.querySelectorAll('video')).sort((a,b) => (b.getBoundingClientRect().width * b.getBoundingClientRect().height) - (a.getBoundingClientRect().width * a.getBoundingClientRect().height))[0]; if(v) { if(document.pictureInPictureElement) document.exitPictureInPicture(); else v.requestPictureInPicture(); } })();";
    webkit_web_view_evaluate_javascript(v, js, -1, NULL, NULL, NULL, NULL, NULL);
}

static void on_star_click(GtkButton*, gpointer) {
    WebKitWebView *v = get_current_view();
    if(!v) return;
    const char *url = webkit_web_view_get_uri(v);
    const char *title = webkit_web_view_get_title(v);
    if(url) {
        toggle_bookmark(url, title ? title : url);
        const char *icon = is_bookmarked(url) ? "starred-symbolic" : "non-starred-symbolic";
        gtk_button_set_icon_name(GTK_BUTTON(BrowserApp::instance->btn_star), icon);
    }
}

static void on_url_activate(GtkEntry *e, gpointer) {
    std::string input = gtk_editable_get_text(GTK_EDITABLE(e));
    std::string url = input;

    if (input.find(" ") != std::string::npos || input.find(".") == std::string::npos) {
        guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(BrowserApp::instance->engine_dropdown));
        if (idx == 1) url = "https://duckduckgo.com/?q=" + input;
        else if (idx == 2) url = "https://www.bing.com/search?q=" + input;
        else url = "https://www.google.com/search?q=" + input;
    } else {
        if (input.find("://") == std::string::npos) url = "https://" + input;
    }

    WebKitWebView *v = get_current_view();
    if(v) webkit_web_view_load_uri(v, url.c_str());
}

static void on_load_changed(WebKitWebView *view, WebKitLoadEvent event, gpointer) {
    if (event == WEBKIT_LOAD_COMMITTED) {
        const char *url = webkit_web_view_get_uri(view);
        const char *title = webkit_web_view_get_title(view);
        if(url) {
            record_history(url, title ? title : url);
            if(view == get_current_view()) {
                gtk_editable_set_text(GTK_EDITABLE(BrowserApp::instance->url_bar), url);
                const char *icon = is_bookmarked(url) ? "starred-symbolic" : "non-starred-symbolic";
                gtk_button_set_icon_name(GTK_BUTTON(BrowserApp::instance->btn_star), icon);
            }
        }
    }
}

static void on_tab_switch(GtkNotebook*, GtkWidget *page, guint, gpointer) {
    WebKitWebView *view = WEBKIT_WEB_VIEW(page);
    const char *url = webkit_web_view_get_uri(view);
    if(url) {
        gtk_editable_set_text(GTK_EDITABLE(BrowserApp::instance->url_bar), url);
        const char *icon = is_bookmarked(url) ? "starred-symbolic" : "non-starred-symbolic";
        gtk_button_set_icon_name(GTK_BUTTON(BrowserApp::instance->btn_star), icon);
    }
}

static void on_tab_close(GtkButton*, gpointer data) {
    GtkWidget *page = GTK_WIDGET(data);
    int num = gtk_notebook_page_num(GTK_NOTEBOOK(BrowserApp::instance->notebook), page);
    if(num != -1) gtk_notebook_remove_page(GTK_NOTEBOOK(BrowserApp::instance->notebook), num);
}

// --- Main App Logic ---

void BrowserApp::create_tab(const char *url) {
    WebKitSettings *settings = webkit_settings_new();
    webkit_settings_set_enable_encrypted_media(settings, TRUE);
    webkit_settings_set_javascript_can_open_windows_automatically(settings, TRUE);

    GtkWidget *view = (GtkWidget*)g_object_new(WEBKIT_TYPE_WEB_VIEW, "settings", settings, NULL);
    g_object_unref(settings);

    WebKitNetworkSession *session = webkit_web_view_get_network_session(WEBKIT_WEB_VIEW(view));
    if (session) {
        webkit_network_session_set_itp_enabled(session, TRUE);
        WebKitCookieManager *cm = webkit_network_session_get_cookie_manager(session);
        webkit_cookie_manager_set_accept_policy(cm, WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY);
    }
    enable_adblock_for_webview(WEBKIT_WEB_VIEW(view));

    GtkWidget *tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *lbl = gtk_label_new("New Tab");
    GtkWidget *close = gtk_button_new_from_icon_name("window-close-symbolic");
    gtk_button_set_has_frame(GTK_BUTTON(close), FALSE);
    g_signal_connect(close, "clicked", G_CALLBACK(on_tab_close), view);

    gtk_box_append(GTK_BOX(tab_box), lbl);
    gtk_box_append(GTK_BOX(tab_box), close);
    gtk_widget_set_visible(tab_box, TRUE);

    int idx = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), view, tab_box);
    gtk_widget_set_visible(view, TRUE);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), idx);

    g_signal_connect(view, "load-changed", G_CALLBACK(on_load_changed), NULL);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(view), url);
}

void BrowserApp::create_window(GtkApplication *app) {
    instance = this;
    window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    gtk_window_set_title(GTK_WINDOW(window), "Malak Browser");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), box);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(box), header);

    // 1. Navigation Buttons
    GtkWidget *b_back = gtk_button_new_from_icon_name("go-previous-symbolic");
    GtkWidget *b_fwd = gtk_button_new_from_icon_name("go-next-symbolic");
    GtkWidget *b_ref = gtk_button_new_from_icon_name("view-refresh-symbolic");
    GtkWidget *b_home = gtk_button_new_from_icon_name("go-home-symbolic");

    g_signal_connect(b_back, "clicked", G_CALLBACK(on_nav_back), NULL);
    g_signal_connect(b_fwd, "clicked", G_CALLBACK(on_nav_fwd), NULL);
    g_signal_connect(b_ref, "clicked", G_CALLBACK(on_nav_refresh), NULL);
    g_signal_connect(b_home, "clicked", G_CALLBACK(on_nav_home), NULL);

    gtk_box_append(GTK_BOX(header), b_back);
    gtk_box_append(GTK_BOX(header), b_fwd);
    gtk_box_append(GTK_BOX(header), b_ref);
    gtk_box_append(GTK_BOX(header), b_home);

    // 2. History Button
    GtkWidget *b_hist = gtk_button_new_from_icon_name("document-open-recent-symbolic");
    gtk_widget_set_tooltip_text(b_hist, "History");
    g_signal_connect(b_hist, "clicked", G_CALLBACK(show_history_window), NULL);
    gtk_box_append(GTK_BOX(header), b_hist);

    // 3. PiP Button
    GtkWidget *b_pip = gtk_button_new_from_icon_name("media-view-subtitles-symbolic");
    gtk_widget_set_tooltip_text(b_pip, "Pop Out Video");
    g_signal_connect(b_pip, "clicked", G_CALLBACK(on_pip_click), NULL);
    gtk_box_append(GTK_BOX(header), b_pip);

    // 4. Engine & URL
    const char *engines[] = { "Google", "DuckDuckGo", "Bing", NULL };
    engine_dropdown = gtk_drop_down_new_from_strings(engines);
    gtk_box_append(GTK_BOX(header), engine_dropdown);

    url_bar = gtk_entry_new();
    gtk_widget_set_hexpand(url_bar, TRUE);
    g_signal_connect(url_bar, "activate", G_CALLBACK(on_url_activate), NULL);
    gtk_box_append(GTK_BOX(header), url_bar);

    // 5. Bookmark Buttons
    btn_star = gtk_button_new_from_icon_name("non-starred-symbolic");
    g_signal_connect(btn_star, "clicked", G_CALLBACK(on_star_click), NULL);
    gtk_box_append(GTK_BOX(header), btn_star);

    GtkWidget *b_bm_list = gtk_button_new_from_icon_name("user-bookmarks-symbolic");
    g_signal_connect(b_bm_list, "clicked", G_CALLBACK(show_bookmarks_window), NULL);
    gtk_box_append(GTK_BOX(header), b_bm_list);

    // 6. New Tab
    GtkWidget *b_add = gtk_button_new_from_icon_name("tab-new-symbolic");
    g_signal_connect(b_add, "clicked", G_CALLBACK(+[](GtkButton*, gpointer){ BrowserApp::instance->create_tab("https://google.com"); }), NULL);
    gtk_box_append(GTK_BOX(header), b_add);

    notebook = gtk_notebook_new();
    gtk_widget_set_vexpand(notebook, TRUE);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(on_tab_switch), NULL);
    gtk_box_append(GTK_BOX(box), notebook);

    create_tab("https://google.com");
    gtk_window_present(GTK_WINDOW(window));
}
