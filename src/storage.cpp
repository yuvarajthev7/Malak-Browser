#include "storage.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

static QString getFilePath(const QString &filename) {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) dir.mkpath(".");
    return path + "/" + filename;
}

void StorageManager::init() { getFilePath(""); }

// --- HISTORY & BOOKMARKS (Existing code kept same) ---
void StorageManager::recordHistory(const QString &url, const QString &title) {
    if (url.isEmpty() || url == "about:blank") return;
    QFile file(getFilePath("history.db"));
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << QDateTime::currentSecsSinceEpoch() << "|" << url << "|" << title << "\n";
    }
}

void StorageManager::clearHistory(int mode) {
    QString path = getFilePath("history.db");
    if (mode == 0) { QFile::remove(path); return; }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QStringList keptLines;
    qint64 now = QDateTime::currentSecsSinceEpoch();
    qint64 cutoff = (mode == 1) ? (now - 3600) : (now - 86400);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split("|");
        if (parts.size() >= 1 && parts[0].toLongLong() < cutoff) keptLines.append(line);
    }
    file.close();
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        for (const QString &l : keptLines) out << l << "\n";
    }
}

QString StorageManager::getHistoryContent() {
    QFile file(getFilePath("history.db"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) return QTextStream(&file).readAll();
    return "";
}

void StorageManager::addBookmark(const QString &url, const QString &title) {
    if (isBookmarked(url)) return;
    QFile file(getFilePath("bookmarks.db"));
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << url << "|" << title << "\n";
    }
}

void StorageManager::removeBookmark(const QString &url) {
    QFile file(getFilePath("bookmarks.db"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QStringList kept;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.startsWith(url + "|")) kept.append(line);
    }
    file.close();
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        for (const QString &l : kept) out << l << "\n";
    }
}

bool StorageManager::isBookmarked(const QString &url) {
    QFile file(getFilePath("bookmarks.db"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&file);
    while (!in.atEnd()) if (in.readLine().startsWith(url + "|")) return true;
    return false;
}

QList<Bookmark> StorageManager::getBookmarks() {
    QList<Bookmark> list;
    QFile file(getFilePath("bookmarks.db"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QStringList parts = in.readLine().split("|");
            if (parts.size() >= 2) list.append({parts[0], parts[1]});
        }
    }
    return list;
}

// --- SHORTCUTS (NEW!) ---
QMap<QString, QString> StorageManager::loadShortcuts() {
    QMap<QString, QString> map;
    // Defaults
    map["New Tab"] = "Ctrl+T";
    map["Close Tab"] = "Ctrl+W";
    map["Next Tab"] = "Ctrl+Tab";
    map["Prev Tab"] = "Ctrl+Shift+Tab";
    map["Reload"] = "F5";
    map["Back"] = "Alt+Left";
    map["Forward"] = "Alt+Right";
    map["History"] = "Ctrl+H";
    map["Bookmarks"] = "Ctrl+B";
    map["Bookmark Page"] = "Ctrl+D";
    map["Focus URL"] = "Ctrl+L";
    map["PiP"] = "Alt+P";

    // Load User Overrides
    QFile file(getFilePath("shortcuts.db"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QStringList parts = in.readLine().split("|");
            if (parts.size() >= 2) map[parts[0]] = parts[1];
        }
    }
    return map;
}

void StorageManager::saveShortcuts(const QMap<QString, QString> &shortcuts) {
    QFile file(getFilePath("shortcuts.db"));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        QMapIterator<QString, QString> i(shortcuts);
        while (i.hasNext()) {
            i.next();
            out << i.key() << "|" << i.value() << "\n";
        }
    }
}
