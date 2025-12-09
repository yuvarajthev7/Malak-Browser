#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <wrl.h>
#include <wil/com.h> // Windows Implementation Library (Standard for WebView2)
#include "WebView2.h"

using namespace Microsoft::WRL;

class BrowserApp {
public:
    // Window Handles
    HWND hWndMain;
    HWND hUrlBar;
    HWND hBtnBack, hBtnFwd, hBtnGo, hBtnPip;

    // WebView2 Pointers (The Engine)
    ComPtr<ICoreWebView2Controller> webviewController;
    ComPtr<ICoreWebView2> webview;

    // Singleton instance
    static BrowserApp* instance;

    // Lifecycle
    void Initialize(HINSTANCE hInstance, int nCmdShow);
    void Resize(int width, int height);

    // Browser Functions
    void Navigate(const std::wstring& url);
    void GoBack();
    void GoForward();
    void Reload();
    void InjectAdBlock(); // Win32 equivalent of our Linux AdBlock
};
