#include "hotkey.h"
#include "option.h"
#include "resource.h"
#include <commctrl.h>

// TODO: get rid of HOTKEYF_EXT ...
bool hotkey_is_valid(WORD hk)
{
	const BYTE mod_mask = HOTKEYF_EXT | HOTKEYF_ALT | HOTKEYF_CONTROL;
	BYTE mod = HIBYTE(hk);
	BYTE vkc = LOBYTE(hk);
	bool is_mod_valid = (mod & mod_mask) && (mod & ~mod_mask) == 0;
	bool is_vkc_valid = vkc > 0 && vkc < 0xFF;

	return is_mod_valid && is_vkc_valid;
}

// modifers used in [RegisterHotKey] are different from
// the ones used in [hotkey control]
BYTE hotkey_mod_ctl2api(BYTE ctlmod)
{
	BYTE apimod = 0;
	if (ctlmod & HOTKEYF_ALT) { apimod |= MOD_ALT; }
	if (ctlmod & HOTKEYF_CONTROL) { apimod |= MOD_CONTROL; }
	//if (ctlmod & HOTKEYF_SHIFT) { apimod |= MOD_SHIFT; }
	return apimod;
}

void hotkey_register(HWND hwnd)
{
	RegisterHotKey(hwnd, HK_CREATE, Ctl2Api(HIBYTE(option.hk_create)), LOBYTE(option.hk_create));
	RegisterHotKey(hwnd, HK_REMOVE, Ctl2Api(HIBYTE(option.hk_remove)), LOBYTE(option.hk_remove));
	RegisterHotKey(hwnd, HK_JUMPTO_NEXT, Ctl2Api(HIBYTE(option.hk_next)), LOBYTE(option.hk_next));
	RegisterHotKey(hwnd, HK_JUMPTO_PREV, Ctl2Api(HIBYTE(option.hk_prev)), LOBYTE(option.hk_prev));
	RegisterHotKey(hwnd, HK_JUMPTO_TAIL, Ctl2Api(HIBYTE(option.hk_tail)), LOBYTE(option.hk_tail));
	RegisterHotKey(hwnd, HK_JUMPTO_HEAD, Ctl2Api(HIBYTE(option.hk_head)), LOBYTE(option.hk_head));
}

void hotkey_unregister(HWND hwnd)
{
	UnregisterHotKey(hwnd, HK_CREATE);
	UnregisterHotKey(hwnd, HK_REMOVE);
	UnregisterHotKey(hwnd, HK_JUMPTO_NEXT);
	UnregisterHotKey(hwnd, HK_JUMPTO_PREV);
	UnregisterHotKey(hwnd, HK_JUMPTO_TAIL);
	UnregisterHotKey(hwnd, HK_JUMPTO_HEAD);
}
