#include "adblock.h"
#include <gtk/gtk.h>
#include <fstream>
#include <sstream>
#include <iostream>

static WebKitUserContentFilterStore *adblock_store = nullptr;
static char *ad_skipper_script = nullptr;

static void load_skipper_script() {
    std::ifstream t("ad_skipper.js");
    if (t.is_open()) {
        std::stringstream buffer;
        buffer << t.rdbuf();
        ad_skipper_script = g_strdup(buffer.str().c_str());
    } else {
        g_print("AdBlock: Warning - ad_skipper.js not found.\n");
    }
}

static void inject_css(WebKitUserContentManager *manager) {
    const char *css = "ytd-banner-promo-renderer, .adsbygoogle { display: none !important; }";
    WebKitUserStyleSheet *style = webkit_user_style_sheet_new(css, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_USER, NULL, NULL);
    webkit_user_content_manager_add_style_sheet(manager, style);
    webkit_user_style_sheet_unref(style);
}

void setup_adblock() {
    char *path = g_build_filename(g_get_user_cache_dir(), "malak_browser", "adblock", NULL);
    adblock_store = webkit_user_content_filter_store_new(path);
    g_free(path);

    std::ifstream t("blocklist.json");
    if (t.is_open()) {
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string rules = buffer.str();
        GBytes *bytes = g_bytes_new(rules.c_str(), rules.length());
        webkit_user_content_filter_store_save(adblock_store, "MalakAdBlock", bytes, NULL, NULL, NULL);
        g_bytes_unref(bytes);
    }
    load_skipper_script();
}

void enable_adblock_for_webview(WebKitWebView *view) {
    WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(view);
    webkit_user_content_filter_store_load(adblock_store, "MalakAdBlock", NULL,
        (GAsyncReadyCallback)+[](GObject *src, GAsyncResult *res, gpointer data){
            WebKitUserContentFilter *filter = webkit_user_content_filter_store_load_finish(WEBKIT_USER_CONTENT_FILTER_STORE(src), res, NULL);
            if(filter) { webkit_user_content_manager_add_filter(WEBKIT_USER_CONTENT_MANAGER(data), filter); g_object_unref(filter); }
        }, manager);

    if (ad_skipper_script) {
        WebKitUserScript *script = webkit_user_script_new(ad_skipper_script, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END, NULL, NULL);
        webkit_user_content_manager_add_script(manager, script);
        webkit_user_script_unref(script);
    }
    inject_css(manager);
}
