#include "browser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

// Link to the same assets used in Linux
#include "../../src/blob_skipper.h"
#include "../../src/blob_blocklist.h"

BrowserApp* BrowserApp::instance = nullptr;

// Helper: Convert std::string to std::wstring (Windows loves Wide Strings)
std::wstring ToWString(const std::string& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    std::wstring r(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
    return r;
}

void BrowserApp::Initialize(HINSTANCE hInstance, int nCmdShow) {
    instance = this;

    // 1. Create Main Window Class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = [](HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT {
        if (msg == WM_DESTROY) PostQuitMessage(0);
        if (msg == WM_SIZE && BrowserApp::instance) {
            RECT r; GetClientRect(hWnd, &r);
            BrowserApp::instance->Resize(r.right, r.bottom);
        }
        return DefWindowProc(hWnd, msg, wp, lp);
    };
    wc.lpszClassName = L"MalakBrowserWin";
    RegisterClassEx(&wc);

    // 2. Create Window
    hWndMain = CreateWindow(L"MalakBrowserWin", L"Malak Browser (Windows)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 1200, 800, NULL, NULL, hInstance, NULL);

    // 3. Create UI Controls (Simple Native Buttons)
    hBtnBack = CreateWindow(L"BUTTON", L"<", WS_CHILD | WS_VISIBLE, 10, 10, 30, 30, hWndMain, (HMENU)1, hInstance, NULL);
    hBtnFwd  = CreateWindow(L"BUTTON", L">", WS_CHILD | WS_VISIBLE, 50, 10, 30, 30, hWndMain, (HMENU)2, hInstance, NULL);
    hUrlBar  = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 90, 10, 800, 30, hWndMain, NULL, hInstance, NULL);
    hBtnGo   = CreateWindow(L"BUTTON", L"Go", WS_CHILD | WS_VISIBLE, 900, 10, 50, 30, hWndMain, (HMENU)3, hInstance, NULL);

    ShowWindow(hWndMain, nCmdShow);

    // 4. Initialize Engine
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(hWndMain,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (controller) {
                                webviewController = controller;
                                webviewController->get_CoreWebView2(&webview);

                                // Setup AdBlock immediately
                                InjectAdBlock();

                                // Navigate Home
                                Navigate(L"https://www.google.com");
                                Resize(1200, 800); // Trigger layout
                            }
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

void BrowserApp::InjectAdBlock() {
    if (!webview) return;

    // 1. Inject the CSS (Cosmetic)
    // Note: WebView2 doesn't have a direct "UserStyleSheet" API like WebKit,
    // so we inject a script that adds a <style> tag.
    std::wstring css = L"const style = document.createElement('style');"
                       L"style.innerHTML = 'ytd-banner-promo-renderer, .adsbygoogle { display: none !important; }';"
                       L"document.head.appendChild(style);";
    webview->AddScriptToExecuteOnDocumentCreated(css.c_str(), nullptr);

    // 2. Inject the Skipper Script (From embedded asset)
    // We convert the char* array from blob_skipper.h to wstring
    std::string script_ansi = std::string((char*)ad_skipper_js, ad_skipper_js_len);
    std::wstring script_wide = ToWString(script_ansi);
    webview->AddScriptToExecuteOnDocumentCreated(script_wide.c_str(), nullptr);

    // 3. Network Filtering (The Blocklist)
    // WebView2 uses 'AddWebResourceRequestedFilter' to intercept requests.
    // For a simple version, we block known ad domains.
    webview->AddWebResourceRequestedFilter(L"*googleads*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
    webview->AddWebResourceRequestedFilter(L"*doubleclick*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);

    // Hook the event to cancel blocked requests
    webview->add_WebResourceRequested(
        Callback<ICoreWebView2WebResourceRequestedEventHandler>(
            [](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
                // Determine if we should block
                // (In a full build, we would parse blocklist.json here)

                // Block by creating an empty response
                ComPtr<ICoreWebView2WebResourceResponse> response;
                // ... create empty response ...
                args->put_Response(response.Get());
                return S_OK;
            }).Get(), nullptr);
}

void BrowserApp::Navigate(const std::wstring& url) {
    if (webview) webview->Navigate(url.c_str());
}

void BrowserApp::Resize(int width, int height) {
    if (webviewController) {
        RECT bounds = { 0, 50, width, height }; // Leave space for toolbar
        webviewController->put_Bounds(bounds);
    }
}
