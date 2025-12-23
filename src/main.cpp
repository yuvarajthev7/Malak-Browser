#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <QPalette>
#include <QColor>
#include <QFile>
#include "mainwindow.h"
#include <cstdlib>

int main(int argc, char *argv[]) {
    qDebug() << "--- STARTING MALAK BROWSER ---";

    // --- LINUX SPECIFIC FIXES (Ignored on Windows) ---
#ifdef Q_OS_LINUX
    qDebug() << "[Linux] Applying Stealth & Driver Fixes...";

    // 1. Stealth Mode (Fixes DBus/Theme crashes)
    qputenv("XDG_CURRENT_DESKTOP", "");
    qputenv("XDG_SESSION_DESKTOP", "");
    qputenv("DESKTOP_SESSION", "");
    qputenv("GNOME_DESKTOP_SESSION_ID", "");
    qputenv("GTK_MODULES", "");

    // 2. Disable Platform Integration
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_NO_XDG_DESKTOP_PORTAL", "1");
    qputenv("QT_NO_GLIB", "1");

    // 3. Driver Fixes
    qputenv("LIBGL_ALWAYS_SOFTWARE", "1");
    qputenv("LIBVA_DRIVER_NAME", "none");

    // 4. Force X11
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif
    // -------------------------------------------------

    // COMMON SETTINGS (Run everywhere)
    // Chromium Flags (Safe for both, though Windows handles GPU better usually)
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            "--disable-gpu --no-sandbox --enable-smooth-scrolling --disable-gpu-compositing");

    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, false);

    QApplication app(argc, argv);
    QApplication::setApplicationName("Malak Browser");
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // LOAD THEME (If available)
    QFile styleFile("assets/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        app.setStyleSheet(QLatin1String(styleFile.readAll()));
    }

    MainWindow window;
    window.show();

    return app.exec();
}
