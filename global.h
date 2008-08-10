#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <windows.h>
#include <commdlg.h>
#include "log.h"
#include "type.h"

extern HWND         app_hwnd;
extern HWND         opt_hwnd;
extern HINSTANCE    app_hins;
extern TCHAR        app_name[];

tstring strim(const tstring& value, TCHAR ch);

void set_transparency(HWND hwnd, DWORD value);

void hotkey_register(HWND hwnd);
void hotkey_unregister(HWND hwnd);

bool autorun_check();
bool autorun_enable();
void autorun_cancel();

COLORREF choose_color(HWND parent, COLORREF init);

tstring get_module_path(HWND hwnd);
tstring get_user_home();

#endif // _GLOBAL_H_
