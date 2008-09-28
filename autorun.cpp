#include "autorun.h"
#include "global.h"

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

bool autorun_enable()
{
	TCHAR path[MAX_PATH];
	if (0 == GetModuleFileName(NULL, path, MAX_PATH)) {
		return false;
	}

	HKEY hkey = NULL;
	DWORD disp = 0;
	LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, RunKey, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &disp);

	DWORD size = (DWORD)(_tcslen(path) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS == result) {
		if (ERROR_SUCCESS == RegSetValueEx(hkey, app_name, 0, REG_SZ, (LPBYTE)path, size)) {
			return true;
		}
		RegCloseKey(hkey);
	}

	return false;
}

bool autorun_cancel()
{
	HKEY hkey = NULL;
	LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, RunKey, 0, KEY_ALL_ACCESS | DELETE, &hkey);
	if (ERROR_SUCCESS == result) {
		if (ERROR_SUCCESS == RegDeleteValue(hkey, app_name)) {
			return true;
		}
		RegCloseKey(hkey);
	}
	return false;
}


//-----------------------------------------------------------------------------
