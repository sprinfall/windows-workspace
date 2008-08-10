#include "desktop-view.h"
#include "desktop.h"
#include "option-view.h"
#include "global.h"
#include "resource.h"
#include "workspace.h"

DesktopView::DesktopView()
: index_(0), total_(option.number),
  menuer_(new Menuer), desktop_(new Desktop)
{
    updateTrayTip();
}

DesktopView::~DesktopView()
{
    delete menuer_;
    delete desktop_;
}

//-----------------------------------------------------------------------------

#define __CASE(condition, function) case condition: function; break;

LRESULT DesktopView::dispatch(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {

    case WM_CREATE:
        app_hwnd = hwnd;
        Workspace::init();
        menuer_->init();
        break;

    case WM_USER:
        if (lParam == WM_RBUTTONUP) { menuer_->popupTray(); }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_EXIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case IDM_CREATE:
            create();
            break;
        case IDM_REMOVE:
            remove();
            break;
        case IDM_OPTION:
            OptionView::create();
            break;
        case IDM_AUTORUN:
            menuer_->checkAutorunMenu();
            break;
        default:
            {
                // Transparency
                WORD cmd = LOWORD(wParam);
                if (cmd >= IDM_TRANS_0 && cmd <= IDM_TRANS_9 ) {
                    menuer_->checkTransMenu(cmd - IDM_TRANS_0);
                }
            }
            break;
        }
        break;

    case WM_HOTKEY:
        switch (LOWORD(wParam)) {
        case HK_CREATE:
            create();
            break;
        case HK_REMOVE:
            remove();
            break;
        case HK_JUMPTO_NEXT:
            jumpto(index_ + 1);
            break;
        case HK_JUMPTO_PREV:
            jumpto(index_ - 1);
            break;
        case HK_JUMPTO_HEAD:
            jumpto(0);
            break;
        case HK_JUMPTO_TAIL:
            jumpto(total_ - 1);
            break;
        }
        break;

    case WM_LBUTTONDBLCLK:
        jumpto(LOWORD(lParam) / (option.cell_cx + BdrSize));
        break;

    case WM_RBUTTONUP:
        menuer_->popupFloat();
        break;

    case WM_LBUTTONDOWN:
        // http://www.vckbase.com/document/viewdoc/?id=464
        SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
        option.updatePosition(); // after moving
        break;

    case WM_PAINT:
        redraw();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

//-----------------------------------------------------------------------------

// Windows procedure
LRESULT CALLBACK DesktopView::wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static DesktopView dtView;
    return dtView.dispatch(hwnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------

void DesktopView::redraw()
{
    hdc_ = BeginPaint(app_hwnd, &ps_);

    // Resize the window
    MoveWindow(app_hwnd, option.position_x, option.position_y, buffer.guicx, buffer.guicy, TRUE);

    SelectObject(hdc_, buffer.bgbrh);
    SelectObject(hdc_, buffer.brpen);

    RECT rect = {0, 0, buffer.guicx, buffer.guicy};
    Rectangle(hdc_, rect.left, rect.top, rect.right, rect.bottom);

    // Draw the bars
    for (int i = 1; i < total_; ++i) {
        DWORD barx = BR_X(i);
        MoveToEx(hdc_, barx, BdrSize, NULL);
        LineTo(hdc_, barx, option.cell_cy + BdrSize);
    }

    SetRect(&rect, HL_L(index_), HL_T, HL_R(index_), HL_B);
    FillRect(hdc_, &rect, buffer.hlbrh);

    EndPaint(app_hwnd, &ps_);
}

//-----------------------------------------------------------------------------

void DesktopView::create()
{
    if (total_ >= MaxNumber) { return; } // ceiling

    int from = index_;
    index_ = total_++;

    option.number = total_;
    buffer.update_guicx();

    resize(false);
    invalidateThumbnail(from);
    invalidateWholeunit(total_ - 1);

    desktop_->create();
    updateTrayTip();
}

void DesktopView::remove()
{
    if (2 == total_) { return; } // at least two workspaces
    if (index_ > 0) { --index_; }
    --total_;

    option.number = total_;
    buffer.update_guicx();

    if (onlyLastVisible()) { // need to adjust the window or it'll be hidden
        option.position_x += option.cell_cx + BdrSize + 3; // extra 3 for notice
        InvalidateRect(app_hwnd, NULL, TRUE);
    } else {
        SetWindowPos(app_hwnd, 0, 0, 0, buffer.guicx, buffer.guicy, SWP_NOMOVE);
    }

    desktop_->remove();
    updateTrayTip();
}

void DesktopView::jumpto(int index)
{
    if (index >= total_ || index < 0 || index == index_) { return; }

    invalidateThumbnail(index_);
    index_ = index;
    invalidateThumbnail(index_);

    desktop_->jumpto(index);
    updateTrayTip();
}

//-----------------------------------------------------------------------------

bool DesktopView::onlyLastVisible()
{
    RECT rect;
    GetWindowRect(app_hwnd, &rect);
    return (DWORD)rect.right < option.cell_cx + BdrSize;
}
