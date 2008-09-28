#include "option.h"
#include "global.h"
#include "hotkey.h"
#include "log.h"
#include <userenv.h>
#include <commctrl.h>
#include <algorithm>

Option option;
Buffer buffer;

// Width and height of the main window
#define GuiCx (option.number * (option.cell_cx + BdrSize) + BdrSize)
#define GuiCy (option.cell_cy + 2 * BdrSize)

// Note: (LONG) is important here
#define MinPositionX (LONG)(-(LONG)GuiCx + 3) // TODO: 3
#define MaxPositionX (LONG)(option.screen_cx - 3)
#define DefPositionX (LONG)(option.screen_cx - GuiCx - 100)

#define MinPositionY (LONG)(-(LONG)GuiCy + 3) // TODO: 3
#define MaxPositionY (LONG)(option.screen_cy - 3)
#define DefPositionY (LONG)(3)

#define DefBrColor 0x808040
#define DefHlColor 0xadad5c
#define DefBgColor 0xffffff

#define DefHkNext (MAKEWORD(VK_RIGHT, HOTKEYF_ALT))
#define DefHkPrev (MAKEWORD(VK_LEFT, HOTKEYF_ALT))
#define DefHkTail (MAKEWORD(VK_DOWN, HOTKEYF_ALT))
#define DefHkHead (MAKEWORD(VK_UP, HOTKEYF_ALT))
#define DefHkCreate (MAKEWORD(VK_INSERT, HOTKEYF_ALT))
#define DefHkRemove (MAKEWORD(VK_DELETE, HOTKEYF_ALT))


#define REG_SAVE_DW(regKey, name, value)\
	RegSetValueEx(regKey, name, 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
#define REG_READ_DW(regKey, name, value, size)\
	RegQueryValueEx(regKey, name, NULL, NULL, (LPBYTE)&value, &size);

#define VALIDATE(var, min, max, def) if (var < min || var > max) { var = def; }
#define VALIDATE_MAX(var, max, def) if (var > max) { var = def; }

void reg_read_hotkey(HKEY hkey, const TCHAR *name, WORD &hk, WORD def)
{
	DWORD temp = 0;
	DWORD size = sizeof DWORD;
	REG_READ_DW(hkey, name, temp, size);
	hk = LOWORD(temp);
	if (!hotkey_is_valid(hk)) { hk = def; }
}
void reg_save_hotkey(HKEY hkey, const TCHAR *name, WORD &hk)
{
	DWORD temp = hk;
	REG_SAVE_DW(hkey, name, temp);
}

void Option::load_except()
{
	TCHAR line[MAX_PATH];
	tifstream fi;

	fi.open(except_app_file_);
	if (!fi.fail()) {
		while(!fi.eof()) {
			fi.getline(line, MAX_PATH);
			tstring temp(line);
			temp = temp.trim();
			if (!temp.empty()) {
				except_app.insert(temp);
			}
		}
		fi.close();
	}

	fi.open(except_cls_file_);
	if (!fi.fail()) {
		while(!fi.eof()) {
			fi.getline(line, MAX_PATH);
			tstring temp(line);
			temp = temp.trim();
			if (!temp.empty()) {
				except_cls.insert(temp);
			}
		}
		fi.close();
	}

	except_cls.insert(_T("SysShadow"));
	except_cls.insert(_T("tooltips_class32"));
}

void Option::load()
{
	init();

	HKEY hkey = NULL;
	if (ERROR_SUCCESS !=
		RegOpenKeyEx(HKEY_CURRENT_USER, config_rkey_, 0, KEY_QUERY_VALUE, &hkey)) {
			useDefault();
			return;
	}

	DWORD size = sizeof DWORD;

	REG_READ_DW(hkey, _T("transparency"), option.transparency, size);
	REG_READ_DW(hkey, _T("number"), option.number, size);
	VALIDATE_MAX(option.transparency, MaxTransparency, DefTransparency);
	VALIDATE(option.number, MinNumber, MaxNumber, DefNumber);

	REG_READ_DW(hkey, _T("cell_cx"), option.cell_cx, size);
	REG_READ_DW(hkey, _T("cell_cy"), option.cell_cy, size);
	VALIDATE(option.cell_cx, MinCellCx, MaxCellCx, DefCellCx);
	VALIDATE(option.cell_cy, MinCellCy, MaxCellCy, DefCellCy);

	REG_READ_DW(hkey, _T("position_x"), option.position_x, size);
	REG_READ_DW(hkey, _T("position_y"), option.position_y, size);
	VALIDATE(option.position_x, MinPositionX, MaxPositionX, DefPositionX);
	VALIDATE(option.position_y, MinPositionY, MaxPositionY, DefPositionY);

	REG_READ_DW(hkey, _T("br_color"), option.br_color, size);
	REG_READ_DW(hkey, _T("hl_color"), option.hl_color, size);
	REG_READ_DW(hkey, _T("bg_color"), option.bg_color, size);
	option.br_color &= 0x00ffffff;
	option.hl_color &= 0x00ffffff;
	option.bg_color &= 0x00ffffff;

	reg_read_hotkey(hkey, _T("hk_next"), option.hk_next, DefHkNext);
	reg_read_hotkey(hkey, _T("hk_prev"), option.hk_prev, DefHkPrev);
	reg_read_hotkey(hkey, _T("hk_tail"), option.hk_tail, DefHkTail);
	reg_read_hotkey(hkey, _T("hk_head"), option.hk_head, DefHkHead);
	reg_read_hotkey(hkey, _T("hk_create"), option.hk_create, DefHkCreate);
	reg_read_hotkey(hkey, _T("hk_remove"), option.hk_remove, DefHkRemove);

	RegCloseKey(hkey);

	load_except();
}

void Option::save()
{
	HKEY hkey = NULL;
	DWORD disp = 0;
	LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, config_rkey_, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &disp);

	if (NOERROR != result) { return; }

	REG_SAVE_DW(hkey, _T("transparency"), option.transparency);
	REG_SAVE_DW(hkey, _T("number"), option.number);

	REG_SAVE_DW(hkey, _T("cell_cx"), option.cell_cx);
	REG_SAVE_DW(hkey, _T("cell_cy"), option.cell_cy);

	REG_SAVE_DW(hkey, _T("position_x"), option.position_x);
	REG_SAVE_DW(hkey, _T("position_y"), option.position_y);

	REG_SAVE_DW(hkey, _T("br_color"), option.br_color);
	REG_SAVE_DW(hkey, _T("hl_color"), option.hl_color);
	REG_SAVE_DW(hkey, _T("bg_color"), option.bg_color);

	reg_save_hotkey(hkey, _T("hk_next"), option.hk_next);
	reg_save_hotkey(hkey, _T("hk_prev"), option.hk_prev);
	reg_save_hotkey(hkey, _T("hk_tail"), option.hk_tail);
	reg_save_hotkey(hkey, _T("hk_head"), option.hk_head);
	reg_save_hotkey(hkey, _T("hk_create"), option.hk_create);
	reg_save_hotkey(hkey, _T("hk_remove"), option.hk_remove);

	RegCloseKey(hkey);

	tofstream fo(except_app_file_);
	if (fo.fail()) {
		CreateDirectory(config_path_, NULL);
		fo.open(except_app_file_);
	}
	std::set<tstring>::const_iterator i;
	for (i = except_app.begin(); i != except_app.end(); ++i) {
		fo << *i << std::endl;
	}
	fo.close();
}

bool Option::is_except(HWND hwnd)
{
	return is_except_cls(hwnd) || is_except_app(hwnd);
}

void Option::except_clr()
{
	except_app.clear();
}

void Option::except_add(const TCHAR *path)
{
	except_app.insert(path);
}


void Option::updatePosition()
{
	RECT rect;
	GetWindowRect(app_hwnd, &rect);
	position_x = rect.left;
	position_y = rect.top;
}

void Option::init()
{
	get_usr_home(config_path_, MAX_PATH);
	_tcscat_s(config_path_, MAX_PATH, _T("\\"));
	_tcscat_s(config_path_, MAX_PATH, app_name);

	_stprintf_s(except_app_file_, MAX_PATH, _T("%s\\except_app.txt"), config_path_);
	_stprintf_s(except_cls_file_, MAX_PATH, _T("%s\\except_cls.txt"), config_path_);

	_stprintf_s(config_rkey_, MAX_PATH, _T("Software\\%s"), app_name);

	screen_cx = GetSystemMetrics(SM_CXSCREEN);
	screen_cy = GetSystemMetrics(SM_CYSCREEN);
}

void Option::useDefault()
{
	option.transparency		= DefTransparency;
	option.number			= DefNumber;

	option.cell_cx			= DefCellCx;
	option.cell_cy			= DefCellCy;

	option.position_x		= DefPositionX;
	option.position_y		= DefPositionY;

	option.br_color			= DefBrColor;
	option.hl_color			= DefHlColor;
	option.bg_color			= DefBgColor;

	option.hk_next			= DefHkNext;
	option.hk_prev			= DefHkPrev;
	option.hk_tail			= DefHkTail;
	option.hk_head			= DefHkHead;
	option.hk_create		= DefHkCreate;
	option.hk_remove		= DefHkRemove;
}

bool Option::is_except_cls(HWND hwnd)
{
	TCHAR cls_name[MAX_PATH];
	GetClassName(hwnd, cls_name, MAX_PATH);
	LOG(cls_name);
	std::set<tstring>::const_iterator i;
	for (i = except_cls.begin(); i != except_cls.end(); ++i) {
		if (0 == _tcsicmp(cls_name, i->c_str())) {
			return true;
		}
	}
	return false;
}

bool Option::is_except_app(HWND hwnd)
{
	TCHAR path[MAX_PATH];
	if (get_app_path(hwnd, path, MAX_PATH)) {
		LOG(path);
		std::set<tstring>::const_iterator i;
		for (i = except_app.begin(); i != except_app.end(); ++i) {
			if (0 == _tcsicmp(path, i->c_str())) {
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

void Buffer::create()
{
	brpen = CreatePen(PS_SOLID | PS_INSIDEFRAME, BdrSize, option.br_color);
	brbrh = CreateSolidBrush(option.br_color);
	bgbrh = CreateSolidBrush(option.bg_color);
	hlbrh = CreateSolidBrush(option.hl_color);

	guicx = option.number * (option.cell_cx + BdrSize) + BdrSize;
	guicy = option.cell_cy + 2 * BdrSize;
}

void Buffer::remove()
{
	DeleteObject(brpen);
	DeleteObject(brbrh);
	DeleteObject(bgbrh);
	DeleteObject(hlbrh);
}

void Buffer::update_br_color()
{
	DeleteObject(brpen);
	DeleteObject(brbrh);
	brpen = CreatePen(PS_SOLID | PS_INSIDEFRAME, BdrSize, option.br_color);
	brbrh = CreateSolidBrush(option.br_color);
}

void Buffer::update_bg_color()
{
	DeleteObject(bgbrh);
	bgbrh = CreateSolidBrush(option.bg_color);
}

void Buffer::update_hl_color()
{
	DeleteObject(hlbrh);
	hlbrh = CreateSolidBrush(option.hl_color);
}

void Buffer::update()
{
	remove();
	create();
}

void Buffer::update_guicx()
{
	guicx = option.number * (option.cell_cx + BdrSize) + BdrSize;
}
void Buffer::update_guicy()
{
	guicx = option.cell_cy + 2 * BdrSize;
}
