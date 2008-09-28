#ifndef _DESKTOP_VIEW_H_
#define _DESKTOP_VIEW_H_

#include "option.h"
#include "global.h"
#include "menu.h"
#include <windows.h>

class Desktop;

class DesktopView {
public:
    DesktopView();
    ~DesktopView();

    void resize(bool redraw);
    LRESULT dispatch(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void redraw();
    void create();
    void remove();
    void jumpto(int index);

    void updateTrayTip();
    void invalidateThumbnail(int index);
    void invalidateWholeunit(int index);

    bool onlyLastVisible();

private:
    int index_, // index of the current desktop (zero based)
        total_; // total number of the desktops

    Desktop *desktop_;

    PAINTSTRUCT ps_;
    HDC hdc_;
};

inline DWORD BR_X(int i)
{
    return (option.cell_cx + BdrSize) * i + BdrSize / 2;
}
inline DWORD TN_X(int i)
{
    return i * (option.cell_cx + BdrSize);
}
inline DWORD HL_L(int i)
{
    return TN_X(i) + BdrSize + GapSize;
}
inline DWORD HL_R(int i)
{
    return TN_X(i+1) - GapSize;
}

#define HL_T (BdrSize + GapSize)
#define HL_B (option.cell_cy + BdrSize - GapSize)

// Resize the window when create or delete desktop.
inline void DesktopView::resize(bool redraw)
{
    int noRedraw = redraw ? 0 : SWP_NOREDRAW;
    SetWindowPos(app_hwnd, 0, 0, 0, buffer.guicx, buffer.guicy, SWP_NOMOVE | noRedraw);
}

inline void DesktopView::updateTrayTip()
{
    menu_update_tip(index_ + 1, total_);
}

inline void DesktopView::invalidateThumbnail(int index)
{
    RECT rect = {HL_L(index), HL_T, HL_L(index+1), HL_B};
    InvalidateRect(app_hwnd, &rect, TRUE);
}

inline void DesktopView::invalidateWholeunit(int index)
{
    RECT rect = {TN_X(index), 0, TN_X(index + 1), buffer.guicy};
    InvalidateRect(app_hwnd, &rect, TRUE);
}

#endif // _DESKTOP_VIEW_H_
