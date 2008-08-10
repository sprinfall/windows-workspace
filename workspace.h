#ifndef _WORKSPACE_H_
#define _WORKSPACE_H_

#include <list>
#include <windows.h>

class Workspace {
public:
    Workspace();
    ~Workspace();

    void enumWindows();
    void addWindow(HWND hwnd);

    void merge(const Workspace& other);

    void hide();
    void show();

    static bool init();

private:
    static void hideWindow(HWND hwnd);
    static void showWindow(HWND hwnd);

    static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM IParam);

    // Handles that should be skipped.
    static HWND progman_; // Program manager
    static HWND taskbar_; // Task bar

    HWND focus_; // Foreground window if exists
    std::list<HWND> windows_;
};

inline void Workspace::hideWindow(HWND hwnd)
{
    if (IsWindowVisible(hwnd)) { ShowWindow(hwnd, SW_HIDE); }
}

inline void Workspace::showWindow(HWND hwnd)
{
    if (! IsWindowVisible(hwnd)) { ShowWindow(hwnd, SW_SHOW); }
}

#endif // _WORKSPACE_H_
