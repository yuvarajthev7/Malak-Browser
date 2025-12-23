#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <QPalette>
#include <QColor>
#include <QFile> // Added this!
#include "mainwindow.h"
#include <cstdlib>

int main(int argc, char *argv[]) {
    qDebug() << "--- STARTING MALAK BROWSER (MODERN UI) ---";

    // 1. STEALTH & DRIVER FLAGS (Keep these exact!)
    qputenv("XDG_CURRENT_DESKTOP", "");
    qputenv("GTK_MODULES", "");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_NO_XDG_DESKTOP_PORTAL", "1");
    qputenv("QT_NO_GLIB", "1");
    qputenv("QT_XKB_CONFIG_ROOT", "/usr/share/X11/xkb");
    qputenv("LIBGL_ALWAYS_SOFTWARE", "1");
    qputenv("LIBVA_DRIVER_NAME", "none");
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            "--disable-gpu --no-sandbox --enable-smooth-scrolling --disable-gpu-compositing");
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, false);
    qputenv("QT_QPA_PLATFORM", "xcb");

    QApplication app(argc, argv);
    QApplication::setApplicationName("Malak Browser");
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // 2. LOAD CUSTOM THEME (New Code)
    QFile styleFile("assets/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
        qDebug() << "[UI] Modern Theme Loaded";
    } else {
        qDebug() << "[UI] WARNING: Could not load assets/style.qss";
    }

    qDebug() << "[1] System Initialized.";
    MainWindow window;
    window.show();

    qDebug() << "[2] Browser Running.";
    return app.exec();
}
