#include "menuer.h"
#include "global.h"
#include "option.h"
#include "resource.h"
#include <iostream>

static DWORD TransMap[] = {
    IDM_TRANS_0,
    IDM_TRANS_1,
    IDM_TRANS_2,
    IDM_TRANS_3,
    IDM_TRANS_4,
    IDM_TRANS_5,
    IDM_TRANS_6,
    IDM_TRANS_7,
    IDM_TRANS_8,
    IDM_TRANS_9
};

Menuer::Menuer()
{
}

Menuer::~Menuer()
{
    Shell_NotifyIcon(NIM_DELETE, &nid_);
}

void Menuer::init()
{
    nid_.cbSize = sizeof(nid_);
    nid_.hWnd = app_hwnd;
    nid_.uID = IDI_APP_16;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_USER;
    nid_.hIcon = LoadIcon(app_hins, MAKEINTRESOURCE(IDI_APP_16));
    // nid_.szTip, see updateTip()
    Shell_NotifyIcon(NIM_ADD, &nid_);

    loadMenu();
}

void Menuer::popupTray()
{
    bool optShown = opt_hwnd && IsWindowVisible(opt_hwnd);
    EnableMenuItem(trayMenu_, IDM_OPTION, MF_BYCOMMAND |
        (optShown ? MF_GRAYED : MF_ENABLED));
    //EnableMenuItem(trayMenu_, IDM_TRANS, MF_BYCOMMAND |
     //   (optShown ? MF_GRAYED : MF_ENABLED));

    // Fix the issue that the menu doesn't disappear when user presses
    // ESCAPE key or clicks somewhere out of the menu
    SetForegroundWindow(app_hwnd);

    menu_popup(trayMenu_, app_hwnd);
}

void Menuer::popupFloat()
{
    EnableMenuItem(floatMenu_, IDM_CREATE, MF_BYCOMMAND |
        (option.number < MaxNumber ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(floatMenu_, IDM_REMOVE, MF_BYCOMMAND |
        (option.number > MinNumber ? MF_ENABLED : MF_GRAYED));

    menu_popup(floatMenu_, app_hwnd);
}

void Menuer::updateTip(size_t current, size_t total)
{
    TCHAR tip[TIP_SIZE];
    _stprintf_s(tip, TIP_SIZE, _T("%s - %d / %d"), app_name, current, total);
    _tcscpy_s(nid_.szTip, TIP_SIZE, tip);
    Shell_NotifyIcon(NIM_MODIFY, &nid_);
}

void Menuer::loadMenu()
{
    trayMenu_ = GetSubMenu(LoadMenu(app_hins, MAKEINTRESOURCE(IDR_MENU_TRAY)), 0);
    floatMenu_ = GetSubMenu(LoadMenu(app_hins, MAKEINTRESOURCE(IDR_MENU_FLOAT)), 0);

    if (autorun_is_set()) { checkMenu(IDM_AUTORUN, true); }
    checkMenu(TransMap[option.transparency], true);
}

void Menuer::checkMenu(DWORD item, bool checked)
{
    CheckMenuItem(trayMenu_, item, MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
}

//void Menuer::checkTransMenu(DWORD newValue)
//{
//    if (option.transparency == newValue) { return; }
//
//    set_transparency(app_hwnd, newValue);
//    checkMenu(TransMap[option.transparency], false);
//    option.transparency = newValue;
//    checkMenu(TransMap[option.transparency], true);
//}

void Menuer::checkAutorunMenu()
{
    UINT state = GetMenuState(trayMenu_, IDM_AUTORUN, MF_BYCOMMAND);
    bool toCheck = !(state & MF_CHECKED); // revert

    checkMenu(IDM_AUTORUN, toCheck);

    if (toCheck) { autorun_enable(); }
    else { autorun_cancel(); }
}
