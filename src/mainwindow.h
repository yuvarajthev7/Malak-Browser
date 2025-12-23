#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QWebEngineView>
#include <QProgressBar>
#include <QAction>
#include <QShortcut>
#include <QMap>
#include <functional> // Needed for std::function

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void createNewTab(const QString &url = "https://google.com");
    void closeTab(int index);
    void nextTab();
    void prevTab();
    void handleTabChanged(int index);
    void navigateToUrl();
    void updateUrlBar(const QUrl &url);
    void updateTitle(const QString &title);
    void updateLoading(int progress);
    void handleLoadFinished(bool ok);

    // Feature Slots
    void toggleBookmark();
    void showHistory();
    void showBookmarks();
    void togglePip();
    void showShortcutSettings();
    void focusUrlBar();

private:
    QTabWidget *tabs;
    QLineEdit *urlBar;
    QProgressBar *progressBar;
    QAction *actStar;

    // Shortcuts
    QMap<QString, QShortcut*> shortcuts;
    void setupShortcuts();
    void applyShortcut(const QString &key, const QString &sequence, const std::function<void()> &func);

    // --- THIS WAS MISSING ---
    void setupGlobalScripts();
    // ------------------------

    QWebEngineView* currentView();
    QWebEngineView* createWebEngineView();
};
