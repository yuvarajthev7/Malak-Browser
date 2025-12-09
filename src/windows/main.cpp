#include "browser.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    BrowserApp app;
    app.Initialize(hInstance, nCmdShow);

    // Main Message Loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
