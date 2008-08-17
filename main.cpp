#include "resource.h"
#include "desktop-view.h"
#include "option.h"
#include "global.h"
#include <windowsx.h>

#pragma comment(lib, "userenv")
#pragma comment(lib, "psapi")
#pragma comment(lib, "comctl32")
#pragma comment(linker, \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' \
   version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
   language='*'\"")

HWND        app_hwnd = NULL;
HINSTANCE   app_hins = NULL;
TCHAR       app_name[] = _T("Windows Workspace");

int WINAPI _tWinMain(HINSTANCE instance, HINSTANCE, PTSTR cmdline, int cmdshow)
{
    // keep single instance
    HANDLE mutex = CreateMutex(NULL, FALSE, app_name);
    if (mutex && ERROR_ALREADY_EXISTS == GetLastError()) {
        return 1;
    }
    app_hins = instance;

    option.load();
    buffer.create();

    WNDCLASSEX wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc    = DesktopView::wndproc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = instance;
    wcex.hIcon          = LoadIcon(instance, MAKEINTRESOURCE(IDI_APP_32));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = buffer.bgbrh; // configurable bg color
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = app_name;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APP_16));

    if (!RegisterClassEx(&wcex)) { return 1; }

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED 0x80000
#endif
    HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        app_name, app_name,
        WS_POPUP,
        option.position_x, option.position_y, buffer.guicx, buffer.guicy,
        NULL, NULL, instance, NULL);

    if (!hwnd) { return 1; }

    ShowWindow(hwnd, cmdshow);
    UpdateWindow(hwnd);

    set_transparency(hwnd, option.transparency);

    hotkey_register(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    hotkey_unregister(hwnd);

    option.save();
    buffer.remove();

    ReleaseMutex(mutex);
    return (int)msg.wParam;
}
