#include "global.h"
#include "resource.h"
#include "option.h"
#include <psapi.h>
#include <userenv.h>
#include <cstdio>
#include <commctrl.h>
#include <cassert>

//-----------------------------------------------------------------------------

#ifndef LWA_ALPHA
#define LWA_ALPHA 0x2
#endif

static
void set_alpha(HWND hwnd, BYTE value)
{
	typedef BOOL (WINAPI *SetLayeredFunc)(HWND, COLORREF, BYTE, DWORD);

	HMODULE user32 = LoadLibraryEx(_T("user32"), NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (! user32) return;

	SetLayeredFunc setLayered =
		(SetLayeredFunc)GetProcAddress(user32, "SetLayeredWindowAttributes");
	if (setLayered) { setLayered(hwnd, 0, value, LWA_ALPHA); }

	FreeLibrary(user32);
}

void set_trans(HWND hwnd, DWORD value)
{
	if (value > 9) { value = 0; }
	set_alpha(hwnd, (BYTE)(10-value) * 255 / 10);
}


//-----------------------------------------------------------------------------

bool get_app_path(HWND hwnd, TCHAR *path, DWORD maxlen)
{
	assert(path);
	path[0] = _T('\0');

	DWORD proc_id;
	GetWindowThreadProcessId(hwnd, &proc_id);

	HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, proc_id);
	if (!hproc) { return false; };

	HMODULE modules[1];
	DWORD needed;
	if (EnumProcessModules(hproc, modules, sizeof(modules), &needed)) {
		if (!GetModuleFileNameEx(hproc, modules[0], path, maxlen)) {
			return false;
		}
	}

	CloseHandle(hproc);
	return true;
}

// Get user's home directory
// E.g., "C:\Documents and Option\<user name>" in XP
bool get_usr_home(TCHAR *path, DWORD maxlen)
{
	assert(path);
	path[0] = _T('\0');
	HANDLE token;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
		if (GetUserProfileDirectory(token, path, &maxlen)) {
			return true;
		}
	}
	return false;
}


bool is_file_existed(const TCHAR *file, const TCHAR *suffix)
{
	if (suffix) {
		size_t flen = _tcslen(file);
		size_t slen = _tcslen(suffix);
		if (flen > slen && _tcsicmp(file + flen - slen, suffix) == 0) {
			FILE *fo = NULL;
			return 0 == _tfopen_s(&fo, file, _T("r"));
		}
	}
	return false;
}


//-----------------------------------------------------------------------------

COLORREF choose_color(HWND parent, COLORREF origin)
{
	static COLORREF cust_clr[16] = {};

	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(cc));

	cc.lStructSize	= sizeof(CHOOSECOLOR);
	cc.hwndOwner	= parent;
	cc.rgbResult	= origin;
	cc.lpCustColors	= (LPDWORD)cust_clr;
	cc.Flags		= CC_RGBINIT | CC_FULLOPEN;

	if (ChooseColor(&cc) == TRUE) { return cc.rgbResult; }
	return origin;
}

void menu_popup(HMENU menu, HWND owner)
{
	POINT pt;
	GetCursorPos(&pt);
	TrackPopupMenuEx(menu, TPM_LEFTALIGN, pt.x, pt.y, owner, NULL);
}
