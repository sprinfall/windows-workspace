#include "global.h"
#include "resource.h"
#include "option.h"
#include <psapi.h>
#include <userenv.h>
#include <cstdio>
#include <commctrl.h>
#include <cassert>

//-----------------------------------------------------------------------------
// logging (DEBUG only)
// TODO: put the log into <user home>\<app_name>

#if defined _DEBUG
static const TCHAR *logFile = _T("log.txt");
void log(const tstring &msg)
{
	tofstream logstream(logFile, std::ios_base::app);
	logstream << msg << std::endl;
	logstream.close();
}
#endif // _DEBUG


//-----------------------------------------------------------------------------

// @str will be changed and cannot be a literal string
TCHAR* str_trim(TCHAR *str, TCHAR ch)
{
    assert(str);
    size_t len = _tcslen(str);
    if (len == 0) { return str; }

    TCHAR *end = str + len - 1;
    while (str <= end && *str == ch) { ++str; }
    while (str < end && *end == ch) { --end; }
    *(++end) = _T('\0');

    return str;
}

tstring str_trim(const tstring& str, TCHAR ch)
{
    if (str.empty()) { return str; }

    size_t i = 0, j = str.length();
    while (i < j && str[i] == ch) { ++i; }
    while (i < j-1 && str[j-1] == ch) { --j; }

    return str.substr(i, j-i);
}


//-----------------------------------------------------------------------------

static void set_alpha(HWND hwnd, BYTE value)
{
#ifndef LWA_ALPHA
#define LWA_ALPHA 0x2
#endif

    typedef BOOL (WINAPI *SetLayeredFunc)(HWND, COLORREF, BYTE, DWORD);

    HMODULE user32 = LoadLibraryEx(_T("user32"), NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (! user32) return;

    SetLayeredFunc setLayered =
        (SetLayeredFunc)GetProcAddress(user32, "SetLayeredWindowAttributes");
    if (setLayered) { setLayered(hwnd, 0, value, LWA_ALPHA); }

    FreeLibrary(user32);
}

void set_transparency(HWND hwnd, DWORD value)
{
    if (value > 9) { value = 0; }
    set_alpha(hwnd, (BYTE)(10-value) * 255 / 10);
}


//-----------------------------------------------------------------------------
// hotkey

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


//-----------------------------------------------------------------------------

static TCHAR RunKey[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");

bool autorun_is_set()
{
    HKEY hkey = NULL;
    RegOpenKeyEx(HKEY_CURRENT_USER, RunKey, 0, KEY_QUERY_VALUE, &hkey);

    TCHAR path[MAX_PATH];
    DWORD size = sizeof path;
    LONG result = RegQueryValueEx(hkey, app_name, NULL, NULL, (LPBYTE)path, &size);

    RegCloseKey(hkey);

    if (ERROR_SUCCESS != result) { return false; }

    // the path of this executale?
    TCHAR this_path[MAX_PATH];
    if (0 != GetModuleFileName(NULL, this_path, MAX_PATH)) {
        if (0 != _tcsicmp(path, this_path)) {
            autorun_cancel(); // delete the incorrect item
            return false;
        }
    }

    return true;
}

void autorun_enable()
{
    TCHAR path[MAX_PATH];
    if (0 == GetModuleFileName(NULL, path, MAX_PATH)) {
        return;
    }

    HKEY hkey = NULL;
    DWORD disp = 0;
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, RunKey, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &disp);

    DWORD size = (DWORD)(_tcslen(path) + 1) * sizeof(TCHAR);
    if (ERROR_SUCCESS == result) {
        RegSetValueEx(hkey, app_name, 0, REG_SZ, (LPBYTE)path, size);
        RegCloseKey(hkey);
    }
}

void autorun_cancel()
{
    HKEY hkey = NULL;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, RunKey, 0,
        KEY_ALL_ACCESS | DELETE, &hkey);

    if (ERROR_SUCCESS == result) {
        RegDeleteValue(hkey, app_name);
        RegCloseKey(hkey);
    }
}


//-----------------------------------------------------------------------------

tstring get_module_path(HWND hwnd)
{
    tstring result = _T("");

    DWORD proc_id;
    GetWindowThreadProcessId(hwnd, &proc_id);

    HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE, proc_id);
    if (!hproc) { return result; };

    HMODULE modules[1];
    DWORD needed;
    if (EnumProcessModules(hproc, modules, sizeof(modules), &needed)) {
        TCHAR path[MAX_PATH];
        if (GetModuleFileNameEx(hproc, modules[0], path, MAX_PATH)) {
            result = path;
        }
    }

    CloseHandle(hproc);
    return result;
}

// Get user's home directory
// E.g., "C:\Documents and Option\<user name>" in XP
tstring get_user_home()
{
    HANDLE token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TCHAR userDir[MAX_PATH] = { 0 };
        DWORD len = MAX_PATH;
        GetUserProfileDirectory(token, userDir, &len);
        return userDir;
    }

    return _T("");
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

    cc.lStructSize  = sizeof(CHOOSECOLOR);
    cc.hwndOwner    = parent;
    cc.rgbResult    = origin;
    cc.lpCustColors = (LPDWORD)cust_clr;
    cc.Flags        = CC_RGBINIT | CC_FULLOPEN;

    if (ChooseColor(&cc) == TRUE) return cc.rgbResult;
    return origin;
}

void center_screen(HWND hwnd)
{
    int screen_cx = GetSystemMetrics(SM_CXSCREEN);
    int screen_cy = GetSystemMetrics(SM_CYSCREEN);

    RECT rect;
    GetWindowRect(hwnd, &rect);
    LONG cx = rect.right - rect.left;
    LONG cy = rect.bottom - rect.top;
    SetWindowPos(hwnd, NULL, (screen_cx-cx)/2, (screen_cy-cy)/2, 0, 0, SWP_NOSIZE);
}

void menu_popup(HMENU menu, HWND owner)
{
    POINT pt;
    GetCursorPos(&pt);
    TrackPopupMenuEx(menu, TPM_LEFTALIGN, pt.x, pt.y, owner, NULL);
}
