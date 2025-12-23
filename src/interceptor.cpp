#include "interceptor.h"
#include <QUrl>
#include <QDebug>

AdBlockInterceptor::AdBlockInterceptor(QObject *parent) : QWebEngineUrlRequestInterceptor(parent) {}

void AdBlockInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info) {
    QString url = info.requestUrl().toString();

    // The "Light" Blocklist logic
    if (url.contains("googleads") ||
        url.contains("doubleclick.net") ||
        url.contains("adservice.google") ||
        url.contains("facebook.com/tr") ||
        url.contains("googletagmanager") ||
        url.contains("analytics") ||
        url.contains("scorecardresearch")) {

        info.block(true);
    }
}
