#include "global.h"
#include "workspace.h"
#include <algorithm>
#include <commctrl.h>
#include <psapi.h>
#include "type.h"
#include "option.h"

HWND Workspace::progman_ = NULL;
HWND Workspace::taskbar_ = NULL;

Workspace::Workspace() : focus_(NULL)
{
}

Workspace::~Workspace()
{
}

// Enumerate windows, save their handles if necessary.
void Workspace::enumWindows()
{
    windows_.clear();

    focus_ = GetForegroundWindow();
    if (!focus_) { LOG(_T("GetForegroundWindow failed")); }
    EnumWindows(&Workspace::enumWindowsProc, (LPARAM)this);
}

void Workspace::addWindow(HWND hwnd)
{
    // TODO: O(n) time, to be improved?
    if (windows_.end() == std::find(windows_.begin(), windows_.end(), hwnd)) {
        windows_.push_back(hwnd);
    }
}

// Merge the windows from other workspace
void Workspace::merge(const Workspace& other)
{
    windows_.insert(windows_.end(), other.windows_.begin(), other.windows_.end());
    // TODO: focused window
}

// Hide all windows in the workspace.
void Workspace::hide()
{
    std::for_each(windows_.begin(), windows_.end(), Workspace::hideWindow);
}

// Show all windows in the workspace.
void Workspace::show()
{
    std::for_each(windows_.begin(), windows_.end(), Workspace::showWindow);
    if (focus_) { SetForegroundWindow(focus_); } // TODO
}

// Callback for Win API EnumWindows()
BOOL CALLBACK Workspace::enumWindowsProc(HWND hwnd, LPARAM IParam)
{
    if (!IsWindowVisible(hwnd)) { return TRUE; }

    if (hwnd == progman_ || hwnd == taskbar_ ||
        hwnd == app_hwnd || hwnd == opt_hwnd) {
        return TRUE;
    }

    if (option.isAlwaysShow(hwnd)) {
        return true;
    }

    ((Workspace *)IParam)->addWindow(hwnd);
    return TRUE;
}


// ISSUE: If explorer crashes ...
bool Workspace::init()
{
    progman_ = FindWindow(_T("Progman"), NULL);
    taskbar_ = FindWindow(_T("Shell_TrayWnd"), NULL);

    if (!progman_ || !taskbar_) { // TODO:
        return false;
    }

    return true;
}
