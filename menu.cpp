#include "menu.h"
#include "global.h"
#include "option.h"
#include "resource.h"
#include "autorun.h"
#include <iostream>

#define TIP_SIZE 64

static HMENU menu_tray, menu_float;
static NOTIFYICONDATA nid;

static const DWORD TransMap[] = {
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

static void menu_check(DWORD item, bool checked);
static void menu_load();

void menu_init()
{
	nid.cbSize		= sizeof(nid);
	nid.hWnd		= app_hwnd;
	nid.uID			= IDI_APP_16;
	nid.uFlags		= NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_USER;
	nid.hIcon		= LoadIcon(app_hins, MAKEINTRESOURCE(IDI_APP_16));
	Shell_NotifyIcon(NIM_ADD, &nid);

	menu_load();
}

void menu_fini()
{
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

void menu_pop_tray()
{
	bool opt_shown = opt_hwnd && IsWindowVisible(opt_hwnd);
	EnableMenuItem(menu_tray, IDM_OPTION, MF_BYCOMMAND | (opt_shown ? MF_GRAYED : MF_ENABLED));

	// Fix the issue that the menu doesn't disappear when user presses
	// ESCAPE key or clicks somewhere out of the menu
	SetForegroundWindow(app_hwnd);
	menu_popup(menu_tray, app_hwnd);
}

void menu_pop_float()
{
	EnableMenuItem(menu_float, IDM_CREATE, MF_BYCOMMAND | (option.number < MaxNumber ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(menu_float, IDM_REMOVE, MF_BYCOMMAND | (option.number > MinNumber ? MF_ENABLED : MF_GRAYED));
	menu_popup(menu_float, app_hwnd);
}

void menu_update_tip(size_t current, size_t total)
{
	TCHAR tip[TIP_SIZE];
	_stprintf_s(tip, TIP_SIZE, _T("%s - %d / %d"), app_name, current, total);
	_tcscpy_s(nid.szTip, TIP_SIZE, tip);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}


void menu_check_autorun()
{
	UINT state = GetMenuState(menu_tray, IDM_AUTORUN, MF_BYCOMMAND);
	bool to_check = !(state & MF_CHECKED); // revert

	menu_check(IDM_AUTORUN, to_check);

	if (to_check) { autorun_enable(); }
	else { autorun_cancel(); }
}


static void menu_load()
{
	menu_tray = GetSubMenu(LoadMenu(app_hins, MAKEINTRESOURCE(IDR_MENU_TRAY)), 0);
	menu_float = GetSubMenu(LoadMenu(app_hins, MAKEINTRESOURCE(IDR_MENU_FLOAT)), 0);

	if (autorun_is_set()) { menu_check(IDM_AUTORUN, true); }
	menu_check(TransMap[option.transparency], true);
}

static void menu_check(DWORD item, bool checked)
{
	CheckMenuItem(menu_tray, item, MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
}
