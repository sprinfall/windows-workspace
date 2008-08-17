#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <windows.h>
#include "type.h"

extern HWND         app_hwnd;
extern HWND         opt_hwnd;
extern HINSTANCE    app_hins;
extern TCHAR        app_name[];

#if defined _DEBUG
void log(const tstring &msg);
#define LOG(msg) log(msg)
#else
#define LOG(msg)
#endif // _DEBUG

TCHAR*  str_trim(TCHAR *str, TCHAR ch = _T(' '));
tstring str_trim(const tstring &str, TCHAR ch = _T(' '));

void set_transparency(HWND hwnd, DWORD value);

bool hotkey_is_valid(WORD hk);

BYTE hotkey_mod_ctl2api(BYTE ctlmod);
#define Ctl2Api(ctlmod) (hotkey_mod_ctl2api(ctlmod))

void hotkey_register(HWND hwnd);
void hotkey_unregister(HWND hwnd);

bool autorun_is_set();
void autorun_enable();
void autorun_cancel();

tstring get_module_path(HWND hwnd);
tstring get_user_home();
bool is_file_existed(const TCHAR *file, const TCHAR *suffix = NULL);

COLORREF choose_color(HWND parent, COLORREF origin);
void center_screen(HWND hwnd);
void menu_popup(HMENU menu, HWND owner);

#endif // _GLOBAL_H_
