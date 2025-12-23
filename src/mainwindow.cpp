#include "mainwindow.h"
#include "interceptor.h"
#include "storage.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QFile>
#include <QTextEdit>
#include <QLabel>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QKeySequenceEdit>
#include <QToolButton>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    resize(1200, 800);
    setWindowTitle("Malak Browser");
    StorageManager::init();

    // 1. Setup AdBlocker
    AdBlockInterceptor *interceptor = new AdBlockInterceptor(this);
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(interceptor);

    // 2. SETUP GLOBAL SCRIPTS (The Fix!)
    setupGlobalScripts();

    // 3. Toolbar & UI (Same as before)
    QToolBar *toolbar = addToolBar("Navigation");
    toolbar->setMovable(false);

    toolbar->addAction("<", [this](){ if(currentView()) currentView()->back(); });
    toolbar->addAction(">", [this](){ if(currentView()) currentView()->forward(); });
    toolbar->addAction("R", [this](){ if(currentView()) currentView()->reload(); });

    urlBar = new QLineEdit();
    urlBar->setPlaceholderText("Search or enter address...");
    connect(urlBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);
    toolbar->addWidget(urlBar);

    actStar = toolbar->addAction("★");
    connect(actStar, &QAction::triggered, this, &MainWindow::toggleBookmark);

    QAction *actHist = toolbar->addAction("History");
    connect(actHist, &QAction::triggered, this, &MainWindow::showHistory);

    QAction *actBk = toolbar->addAction("Bookmarks");
    connect(actBk, &QAction::triggered, this, &MainWindow::showBookmarks);

    QAction *actPip = toolbar->addAction("PiP");
    connect(actPip, &QAction::triggered, this, &MainWindow::togglePip);

    QAction *actSettings = toolbar->addAction("⚙");
    connect(actSettings, &QAction::triggered, this, &MainWindow::showShortcutSettings);

    progressBar = new QProgressBar();
    progressBar->setMaximumHeight(2);
    progressBar->setTextVisible(false);

    tabs = new QTabWidget();
    tabs->setTabsClosable(true);
    tabs->setDocumentMode(true);
    connect(tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(tabs, &QTabWidget::currentChanged, this, &MainWindow::handleTabChanged);

    QToolButton *newTabBtn = new QToolButton();
    newTabBtn->setText("+");
    newTabBtn->setToolTip("New Tab");
    newTabBtn->setAutoRaise(true);
    connect(newTabBtn, &QToolButton::clicked, this, [this](){ createNewTab(); });
    tabs->setCornerWidget(newTabBtn, Qt::TopRightCorner);

    QWidget *container = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(progressBar);
    layout->addWidget(tabs);
    setCentralWidget(container);

    createNewTab();
    setupShortcuts();
}

// --- SHORTCUT LOGIC ---

void MainWindow::applyShortcut(const QString &key, const QString &sequence, const std::function<void()> &func) {
    if (shortcuts.contains(key)) {
        shortcuts[key]->setKey(QKeySequence(sequence));
    } else {
        QShortcut *s = new QShortcut(QKeySequence(sequence), this);
        connect(s, &QShortcut::activated, func);
        shortcuts[key] = s;
    }
}

void MainWindow::setupShortcuts() {
    QMap<QString, QString> map = StorageManager::loadShortcuts();

    applyShortcut("New Tab", map["New Tab"], [this](){ createNewTab(); });
    applyShortcut("Close Tab", map["Close Tab"], [this](){ closeTab(tabs->currentIndex()); });
    applyShortcut("Next Tab", map["Next Tab"], [this](){ nextTab(); });
    applyShortcut("Prev Tab", map["Prev Tab"], [this](){ prevTab(); });
    applyShortcut("Reload", map["Reload"], [this](){ if(currentView()) currentView()->reload(); });
    applyShortcut("Back", map["Back"], [this](){ if(currentView()) currentView()->back(); });
    applyShortcut("Forward", map["Forward"], [this](){ if(currentView()) currentView()->forward(); });
    applyShortcut("History", map["History"], [this](){ showHistory(); });
    applyShortcut("Bookmarks", map["Bookmarks"], [this](){ showBookmarks(); });
    applyShortcut("Bookmark Page", map["Bookmark Page"], [this](){ toggleBookmark(); });
    applyShortcut("Focus URL", map["Focus URL"], [this](){ focusUrlBar(); });
    applyShortcut("PiP", map["PiP"], [this](){ togglePip(); });
}

void MainWindow::showShortcutSettings() {
    QDialog dlg(this);
    dlg.setWindowTitle("Keyboard Shortcuts");
    dlg.resize(500, 600);
    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QTableWidget *table = new QTableWidget();
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Action", "Shortcut"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);

    QMap<QString, QString> currentMap = StorageManager::loadShortcuts();
    int row = 0;

    // We want a button for each row to record key
    QMap<QString, QKeySequenceEdit*> editors;

    table->setRowCount(currentMap.size());
    for(auto it = currentMap.begin(); it != currentMap.end(); ++it) {
        QTableWidgetItem *item = new QTableWidgetItem(it.key());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable); // Read only text
        table->setItem(row, 0, item);

        QKeySequenceEdit *edit = new QKeySequenceEdit(QKeySequence(it.value()));
        table->setCellWidget(row, 1, edit);
        editors[it.key()] = edit;

        row++;
    }
    layout->addWidget(table);

    // Save Button
    QPushButton *btnSave = new QPushButton("Save & Apply");
    connect(btnSave, &QPushButton::clicked, [&](){
        QMap<QString, QString> newMap;
        for(auto it = editors.begin(); it != editors.end(); ++it) {
            newMap[it.key()] = it.value()->keySequence().toString();
        }
        StorageManager::saveShortcuts(newMap);
        setupShortcuts(); // Reload immediately
        dlg.accept();
    });
    layout->addWidget(btnSave);

    dlg.exec();
}

void MainWindow::focusUrlBar() {
    urlBar->setFocus();
    urlBar->selectAll();
}

void MainWindow::nextTab() {
    int idx = tabs->currentIndex() + 1;
    if (idx >= tabs->count()) idx = 0;
    tabs->setCurrentIndex(idx);
}

void MainWindow::prevTab() {
    int idx = tabs->currentIndex() - 1;
    if (idx < 0) idx = tabs->count() - 1;
    tabs->setCurrentIndex(idx);
}

// --- STANDARD BROWSER LOGIC (Kept same) ---

// void MainWindow::injectScripts(QWebEngineView *view) {
//     // 1. AdSkipper (Existing)
//     QWebEngineScript script;
//     QFile file("assets/ad_skipper.js");
//     if (file.open(QIODevice::ReadOnly)) {
//         script.setSourceCode(file.readAll());
//         script.setInjectionPoint(QWebEngineScript::DocumentReady);
//         script.setWorldId(QWebEngineScript::MainWorld);
//         view->page()->scripts().insert(script);
//     }
//
//     // 2. CSS Fixes (Existing)
//     QWebEngineScript css;
//     css.setSourceCode("const s=document.createElement('style');s.innerHTML='ytd-banner-promo-renderer,.adsbygoogle{display:none!important;}';document.head.appendChild(s);");
//     css.setInjectionPoint(QWebEngineScript::DocumentReady);
//     view->page()->scripts().insert(css);
//
//     // 3. PHYSICS SCROLL ENGINE (NEW!)
//     // We inject this at "DocumentCreation" so it starts working immediately
//     QWebEngineScript smoothScroll;
//     QFile smoothFile("assets/smooth.js");
//     if (smoothFile.open(QIODevice::ReadOnly)) {
//         smoothScroll.setSourceCode(smoothFile.readAll());
//         smoothScroll.setInjectionPoint(QWebEngineScript::DocumentCreation); // Earlier injection
//         smoothScroll.setWorldId(QWebEngineScript::MainWorld);
//         view->page()->scripts().insert(smoothScroll);
//     }
// }

void MainWindow::setupGlobalScripts() {
    auto profile = QWebEngineProfile::defaultProfile();
    if (!profile) {
        qDebug() << "CRITICAL ERROR: Default Profile is null!";
        return;
    }

    QWebEngineScriptCollection *scripts = profile->scripts();

    // 1. AdSkipper
    QFile file("assets/ad_skipper.js");
    if (file.open(QIODevice::ReadOnly)) {
        QWebEngineScript script;
        script.setSourceCode(file.readAll());
        script.setInjectionPoint(QWebEngineScript::DocumentReady);
        script.setWorldId(QWebEngineScript::MainWorld);
        script.setName("AdSkipper");
        scripts->insert(script);
        qDebug() << "   -> Injected AdSkipper";
    } else {
        qDebug() << "   -> WARNING: Could not find assets/ad_skipper.js";
    }

    // 2. CSS Fixes
    QWebEngineScript css;
    css.setSourceCode("const s=document.createElement('style'); s.textContent='ytd-banner-promo-renderer, ytd-display-ad-renderer, .adsbygoogle, div[id^=\"google_ads_\"], .ytp-ad-module { display: none !important; }'; document.head.appendChild(s);");
    css.setInjectionPoint(QWebEngineScript::DocumentReady);
    css.setName("AdCSS");
    scripts->insert(css);

    // 3. Smooth Scroll
    QFile smoothFile("assets/smooth.js");
    if (smoothFile.open(QIODevice::ReadOnly)) {
        QWebEngineScript smoothScroll;
        smoothScroll.setSourceCode(smoothFile.readAll());
        smoothScroll.setInjectionPoint(QWebEngineScript::DocumentCreation);
        smoothScroll.setWorldId(QWebEngineScript::MainWorld);
        smoothScroll.setName("SmoothScroll");
        scripts->insert(smoothScroll);
        qDebug() << "   -> Injected SmoothScroll";
    }
}

// --- UPDATED CREATE VIEW (Simpler now) ---
QWebEngineView* MainWindow::createWebEngineView() {
    QWebEngineView *view = new QWebEngineView();
    // No need to injectScripts(view) here anymore! The Profile handles it.

    view->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    view->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    view->settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);

    connect(view, &QWebEngineView::urlChanged, this, &MainWindow::updateUrlBar);
    connect(view, &QWebEngineView::titleChanged, this, &MainWindow::updateTitle);
    connect(view, &QWebEngineView::loadProgress, this, &MainWindow::updateLoading);
    connect(view, &QWebEngineView::loadFinished, this, &MainWindow::handleLoadFinished);
    return view;
}

void MainWindow::createNewTab(const QString &url) {
    QWebEngineView *view = createWebEngineView();
    view->setUrl(QUrl(url));
    int index = tabs->addTab(view, "New Tab");
    tabs->setCurrentIndex(index);
}

void MainWindow::closeTab(int index) {
    if (tabs->count() > 1) tabs->removeTab(index);
    else close();
}

void MainWindow::handleTabChanged(int index) {
    if (index >= 0 && currentView()) {
        updateUrlBar(currentView()->url());
        updateTitle(currentView()->title());
    }
}

void MainWindow::navigateToUrl() {
    QString text = urlBar->text();
    QUrl url = QUrl::fromUserInput(text);
    if (!text.contains(".") && !text.contains("://")) url = QUrl("https://www.google.com/search?q=" + text);
    if (currentView()) currentView()->setUrl(url);
}

void MainWindow::updateUrlBar(const QUrl &url) {
    if (currentView() == sender()) {
        urlBar->setText(url.toString());
        urlBar->setCursorPosition(0);
        if (StorageManager::isBookmarked(url.toString())) actStar->setText("★"); else actStar->setText("☆");
    }
}

void MainWindow::updateTitle(const QString &title) {
    if (currentView() == sender()) {
        int index = tabs->indexOf(qobject_cast<QWidget*>(sender()));
        if (index != -1) tabs->setTabText(index, title.left(20) + (title.length()>20 ? "..." : ""));
        if (index == tabs->currentIndex()) setWindowTitle(title + " - Malak Browser");
        StorageManager::recordHistory(currentView()->url().toString(), title);
    }
}

void MainWindow::updateLoading(int progress) {
    if (currentView() == sender()) {
        progressBar->setValue(progress);
        progressBar->setVisible(progress < 100);
    }
}

void MainWindow::handleLoadFinished(bool) {
    if (currentView() == sender()) progressBar->setVisible(false);
}

// --- FEATURE DIALOGS ---

void MainWindow::toggleBookmark() {
    if (!currentView()) return;
    QString url = currentView()->url().toString();
    QString title = currentView()->title();
    if (StorageManager::isBookmarked(url)) {
        StorageManager::removeBookmark(url);
        actStar->setText("☆");
    } else {
        StorageManager::addBookmark(url, title);
        actStar->setText("★");
    }
}

void MainWindow::showHistory() {
    QDialog dlg(this);
    dlg.setWindowTitle("History");
    dlg.resize(600, 400);
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QTextEdit *text = new QTextEdit();
    text->setReadOnly(true);
    text->setText(StorageManager::getHistoryContent());
    layout->addWidget(text);
    QPushButton *btnClear = new QPushButton("Clear All History");
    connect(btnClear, &QPushButton::clicked, [&](){ StorageManager::clearHistory(0); text->clear(); });
    layout->addWidget(btnClear);
    dlg.exec();
}

void MainWindow::showBookmarks() {
    QDialog dlg(this);
    dlg.setWindowTitle("Bookmarks");
    dlg.resize(400, 500);
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QListWidget *list = new QListWidget();
    QList<Bookmark> bms = StorageManager::getBookmarks();
    for (const auto &b : bms) {
        QListWidgetItem *item = new QListWidgetItem(b.title);
        item->setData(Qt::UserRole, b.url);
        list->addItem(item);
    }
    connect(list, &QListWidget::itemClicked, [&](QListWidgetItem *item){
        createNewTab(item->data(Qt::UserRole).toString());
        dlg.close();
    });
    layout->addWidget(list);
    dlg.exec();
}

void MainWindow::togglePip() {
    if (!currentView()) return;
    currentView()->page()->runJavaScript(
        "(function() { var v = Array.from(document.querySelectorAll('video')).sort((a,b) => (b.getBoundingClientRect().width * b.getBoundingClientRect().height) - (a.getBoundingClientRect().width * a.getBoundingClientRect().height))[0]; if(v) { if(document.pictureInPictureElement) document.exitPictureInPicture(); else v.requestPictureInPicture(); } })();"
    );
}

QWebEngineView* MainWindow::currentView() {
    return qobject_cast<QWebEngineView*>(tabs->currentWidget());
}
