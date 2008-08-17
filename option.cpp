#include "option.h"
#include "global.h"
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

void Option::load()
{
    init();

    HKEY hkey = NULL;
    if (ERROR_SUCCESS !=
        RegOpenKeyEx(HKEY_CURRENT_USER, configRKey_.c_str(), 0, KEY_QUERY_VALUE, &hkey)) {
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

    tifstream fi(exceptPath_.c_str());
    if (fi.fail()) { return; }
    tstring line;
    while (getline(fi, line)) {
        line = str_trim(line);
        if (!line.empty()) { excepts_.push_back(line); }
    }
    fi.close();
}

void Option::save()
{
    HKEY hkey = NULL;
    DWORD disp = 0;
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, configRKey_.c_str(), 0, NULL,
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

    tofstream fo(exceptPath_.c_str());
    if (fo.fail()) {
        CreateDirectory(configPath_.c_str(), NULL);
        fo.open(exceptPath_.c_str());
    }
    std::list<tstring>::const_iterator i;
    for (i = excepts_.begin(); i != excepts_.end(); ++i) {
        fo << *i << std::endl;
    }
    fo.close();
}

bool Option::isAlwaysShow(HWND hwnd)
{
    return isAlwaysShowClass(hwnd) || isAlwaysShowProgram(hwnd);
}

bool Option::isExcept(const tstring &path)
{
    std::list<tstring>::const_iterator i;
    for (i = excepts_.begin(); i != excepts_.end(); ++i) {
        if (path == *i) { return true; }
    }
    return false;
}

void Option::except_clr()
{
    excepts_.clear();
}

void Option::except_add(const tstring &path)
{
    excepts_.push_back(path);
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
    configPath_ = get_user_home() + _T("\\") + app_name;
    exceptPath_ = configPath_ + _T("\\Always show.txt");
    configRKey_ = _T("Software\\");
    configRKey_ += app_name;

    screen_cx = GetSystemMetrics(SM_CXSCREEN);
    screen_cy = GetSystemMetrics(SM_CYSCREEN);
}

void Option::useDefault()
{
    option.transparency = DefTransparency;
    option.number = DefNumber;

    option.cell_cx = DefCellCx;
    option.cell_cy = DefCellCy;

    option.position_x = DefPositionX;
    option.position_y = DefPositionY;

    option.br_color = DefBrColor;
    option.hl_color = DefHlColor;
    option.bg_color = DefBgColor;

    option.hk_next = DefHkNext;
    option.hk_prev = DefHkPrev;
    option.hk_tail = DefHkTail;
    option.hk_head = DefHkHead;
    option.hk_create = DefHkCreate;
    option.hk_remove = DefHkRemove;
}


static TCHAR* AlwaysShowClasses[] = {
    _T("SysShadow"), // shadow of window
    _T("tooltips_class32"), // tooltip
    _T("TTPlayer_PlayerWnd"),
    _T("TFrmSkin") // skin of SnowFox DesktopSprite
};

bool Option::isAlwaysShowClass(HWND hwnd)
{
    TCHAR className[MAX_PATH];
    GetClassName(hwnd, className, MAX_PATH);

    for (size_t i = 0; i < sizeof(AlwaysShowClasses)/sizeof(TCHAR*); ++i) {
        if (0 == _tcsicmp(className, AlwaysShowClasses[i])) {
            return true;
        }
    }
    return false;
}

bool Option::isAlwaysShowProgram(HWND hwnd)
{
    tstring prog = get_module_path(hwnd);
    if (!prog.empty()) {
        std::list<tstring>::const_iterator i;
        for (i = excepts_.begin(); i != excepts_.end(); ++i) {
            if (0 == _tcsicmp(prog.c_str(), i->c_str())) {
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
