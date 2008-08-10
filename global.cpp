#include "global.h"
#include "resource.h"
#include <psapi.h>
#include <userenv.h>

//-----------------------------------------------------------------------------

tstring strim(const tstring& value, TCHAR ch)
{
    tstring trimed = _T("");
    if (value.empty()) { return trimed; }

    const tstring::size_type npos = -1;
    tstring::size_type begin = value.find_first_not_of(ch);
    tstring::size_type end = value.find_last_not_of(ch);

    if (begin != npos && end != npos && begin <= end) {
        trimed = value.substr(begin, end - begin + 1);
    }
    return trimed;
}

//-----------------------------------------------------------------------------

#ifndef LWA_ALPHA
#define LWA_ALPHA 0x2
#endif

static void set_alpha(HWND hwnd, BYTE value)
{
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

void hotkey_register(HWND hwnd)
{
    RegisterHotKey(hwnd, HK_CREATE, MOD_ALT, VK_INSERT);
    RegisterHotKey(hwnd, HK_REMOVE, MOD_ALT, VK_DELETE);
    RegisterHotKey(hwnd, HK_JUMPTO_NEXT, MOD_ALT, VK_RIGHT);
    RegisterHotKey(hwnd, HK_JUMPTO_PREV, MOD_ALT, VK_LEFT);
    RegisterHotKey(hwnd, HK_JUMPTO_HEAD, MOD_ALT, VK_UP);
    RegisterHotKey(hwnd, HK_JUMPTO_TAIL, MOD_ALT, VK_DOWN);
}

void hotkey_unregister(HWND hwnd)
{
    UnregisterHotKey(hwnd, HK_CREATE);
    UnregisterHotKey(hwnd, HK_REMOVE);
    UnregisterHotKey(hwnd, HK_JUMPTO_NEXT);
    UnregisterHotKey(hwnd, HK_JUMPTO_PREV);
    UnregisterHotKey(hwnd, HK_JUMPTO_HEAD);
    UnregisterHotKey(hwnd, HK_JUMPTO_TAIL);
}

//-----------------------------------------------------------------------------

COLORREF choose_color(HWND parent, COLORREF origin)
{
    static COLORREF custClr[16] = {};

    CHOOSECOLOR cc;
    ZeroMemory(&cc, sizeof(cc));

    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.hwndOwner = parent;
    cc.rgbResult = origin;
    cc.lpCustColors = (LPDWORD)custClr;
    cc.Flags = CC_RGBINIT | CC_FULLOPEN;

    if (ChooseColor(&cc) == TRUE) return cc.rgbResult;
    return origin;
}

//-----------------------------------------------------------------------------

static const TCHAR *RegKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");

bool autorun_check()
{
    HKEY key = NULL;
    RegOpenKeyEx(HKEY_CURRENT_USER, RegKey, 0, KEY_QUERY_VALUE, &key);

    TCHAR value[MAX_PATH];
    DWORD valueSize = MAX_PATH * sizeof(TCHAR);
    LONG result = RegQueryValueEx(key,
        app_name,
        NULL,
        NULL,
        (LPBYTE)value,
        &valueSize);

    RegCloseKey(key);
    if (ERROR_FILE_NOT_FOUND == result) {
        return false;
    }

    return ERROR_SUCCESS == result;
}

bool autorun_enable()
{
    TCHAR exePath[MAX_PATH];
    if (0 == GetModuleFileName(NULL, exePath, MAX_PATH)) {
        return false;
    }

    HKEY hkey = NULL;
    DWORD disp = 0;
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, RegKey, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &disp);

    if (NOERROR == result) {
        RegSetValueEx(hkey, app_name, 0, REG_SZ, (LPBYTE)exePath,
            (lstrlen(exePath) + 1) * sizeof(TCHAR));
        RegCloseKey(hkey);
    }

    return true;
}

void autorun_cancel()
{
    HKEY hkey = NULL;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, RegKey, 0,
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

    DWORD procId;
    GetWindowThreadProcessId(hwnd, &procId);

    HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procId);
    if (!hproc) { return result; };

    HMODULE modules[1];
    DWORD needed;
    if (EnumProcessModules(hproc, modules, sizeof(modules), &needed))
    {
        TCHAR modulePath[MAX_PATH];

        if (GetModuleFileNameEx(hproc, modules[0], modulePath, MAX_PATH)) {
            result = modulePath;
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
