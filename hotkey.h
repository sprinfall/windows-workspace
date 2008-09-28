#ifndef _HOTKEY_H_
#define _HOTKEY_H_

#include <windows.h>

bool hotkey_is_valid(WORD hk);

BYTE hotkey_mod_ctl2api(BYTE ctlmod);
#define Ctl2Api(ctlmod) (hotkey_mod_ctl2api(ctlmod))

void hotkey_register(HWND hwnd);
void hotkey_unregister(HWND hwnd);

#endif // _HOTKEY_H_
