#pragma once
#include <string>
#include <vector>

struct Bookmark { std::string url; std::string title; };

// History
void record_history(const std::string &url, const std::string &title);
void clear_history(int timeframe); // 0=All, 1=Hour, 2=Today
std::string get_history_content();

// Preferences
void set_clear_on_exit(bool enabled);
bool get_clear_on_exit();
void perform_exit_cleanup();

// Bookmarks
void load_bookmarks();
void save_bookmarks();
void toggle_bookmark(const std::string &url, const std::string &title);
bool is_bookmarked(const std::string &url);
const std::vector<Bookmark>& get_bookmarks();
