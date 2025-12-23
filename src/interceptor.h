#pragma once
#include <QWebEngineUrlRequestInterceptor>

class AdBlockInterceptor : public QWebEngineUrlRequestInterceptor {
    Q_OBJECT
public:
    explicit AdBlockInterceptor(QObject *parent = nullptr);
    void interceptRequest(QWebEngineUrlRequestInfo &info) override;
};
