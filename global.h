#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "type.h"
#include "tstring.h"
#include <windows.h>
#include <cassert>

extern HWND         app_hwnd;
extern HWND         opt_hwnd;
extern HINSTANCE    app_hins;
extern TCHAR        app_name[];

void set_trans(HWND hwnd, DWORD value);

bool get_app_path(HWND hwnd, TCHAR *path, DWORD maxlen);
bool get_usr_home(TCHAR *path, DWORD maxlen);

bool is_file_existed(const TCHAR *file, const TCHAR *suffix = NULL);

COLORREF choose_color(HWND parent, COLORREF origin);

void menu_popup(HMENU menu, HWND owner);

#endif // _GLOBAL_H_
