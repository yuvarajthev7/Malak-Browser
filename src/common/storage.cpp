#include "storage.h"
#include <gtk/gtk.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <algorithm>
#include <vector>

static std::vector<Bookmark> bookmarks;

static std::string get_data_path(const char* file) {
    const char *dir = g_get_user_data_dir();
    std::string path = std::string(dir) + "/malak_browser";
    std::filesystem::create_directories(path);
    return path + "/" + file;
}

// --- History Logic ---

void record_history(const std::string &url, const std::string &title) {
    if (url.empty() || url == "about:blank") return;
    std::ofstream file(get_data_path("history.db"), std::ios::app);
    auto t = std::time(nullptr);
    file << t << "|" << url << "|" << title << "\n";
}

void clear_history(int timeframe) {
    std::string path = get_data_path("history.db");

    // 0 = Clear All (Fastest)
    if (timeframe == 0) {
        std::ofstream file(path, std::ios::trunc);
        return;
    }

    // Partial Clear (Read -> Filter -> Write)
    std::vector<std::string> kept_lines;
    std::ifstream infile(path);
    std::string line;
    long current_time = std::time(nullptr);
    long cutoff = 0;

    if (timeframe == 1) cutoff = current_time - 3600;        // Last Hour
    else if (timeframe == 2) cutoff = current_time - 86400;  // Last 24h

    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string ts_str;
        if (std::getline(ss, ts_str, '|')) {
            try {
                long ts = std::stol(ts_str);
                // Keep entries OLDER than the cutoff (we delete recent stuff)
                if (ts < cutoff) {
                    kept_lines.push_back(line);
                }
            } catch (...) {}
        }
    }

    // Rewrite file
    std::ofstream outfile(path, std::ios::trunc);
    for (const auto &l : kept_lines) outfile << l << "\n";
}

std::string get_history_content() {
    std::ifstream file(get_data_path("history.db"));
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// --- Preferences (Auto-Clear) ---

void set_clear_on_exit(bool enabled) {
    std::string path = get_data_path("clean_exit.flag");
    if (enabled) {
        std::ofstream file(path);
        file << "1";
    } else {
        std::filesystem::remove(path);
    }
}

bool get_clear_on_exit() {
    return std::filesystem::exists(get_data_path("clean_exit.flag"));
}

void perform_exit_cleanup() {
    if (get_clear_on_exit()) {
        clear_history(0); // Wipe all
        g_print("Privacy: History wiped on exit.\n");
    }
}

// --- Bookmarks Logic ---

void load_bookmarks() {
    bookmarks.clear();
    std::ifstream file(get_data_path("bookmarks.db"));
    std::string line;
    while(std::getline(file, line)) {
        std::stringstream ss(line);
        std::string u, t;
        if(std::getline(ss, u, '|') && std::getline(ss, t)) bookmarks.push_back({u, t});
    }
}

void save_bookmarks() {
    std::ofstream file(get_data_path("bookmarks.db"));
    for(const auto& b : bookmarks) file << b.url << "|" << b.title << "\n";
}

void toggle_bookmark(const std::string &url, const std::string &title) {
    auto it = std::find_if(bookmarks.begin(), bookmarks.end(), [&](const Bookmark& b){ return b.url == url; });
    if (it != bookmarks.end()) bookmarks.erase(it);
    else bookmarks.push_back({url, title});
    save_bookmarks();
}

bool is_bookmarked(const std::string &url) {
    return std::any_of(bookmarks.begin(), bookmarks.end(), [&](const Bookmark& b){ return b.url == url; });
}

const std::vector<Bookmark>& get_bookmarks() { return bookmarks; }
