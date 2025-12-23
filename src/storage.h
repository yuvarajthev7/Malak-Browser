#pragma once
#include <QString>
#include <QList>
#include <QMap>

struct Bookmark { QString url; QString title; };

class StorageManager {
public:
    static void init();

    // History
    static void recordHistory(const QString &url, const QString &title);
    static void clearHistory(int mode);
    static QString getHistoryContent();

    // Bookmarks
    static void addBookmark(const QString &url, const QString &title);
    static void removeBookmark(const QString &url);
    static bool isBookmarked(const QString &url);
    static QList<Bookmark> getBookmarks();

    // Shortcuts (New!)
    static QMap<QString, QString> loadShortcuts();
    static void saveShortcuts(const QMap<QString, QString> &shortcuts);
};
