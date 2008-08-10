#ifndef _OPTION_H_
#define _OPTION_H_

#include "type.h"
#include <windows.h>
#include <list>

#define MinTransparency 0
#define MaxTransparency 9
#define DefTransparency 3

#define MinNumber 2 // since 1 makes no sense
#define MaxNumber 9
#define DefNumber 4

#define MinCellCx 32
#define MaxCellCx 72
#define DefCellCx 36

#define MinCellCy 16
#define MaxCellCy 36
#define DefCellCy 21

#define BdrSize 1 // window border
#define GapSize 4 // gap between border and highlight

class Option {
public:
    DWORD   screen_cx, screen_cy;

    DWORD   transparency; // 0, 1 ~ 9 (0%, 10% ~ 90%)
    DWORD   number;

    DWORD   cell_cx,
            cell_cy;

    LONG    position_x,
            position_y;

    DWORD   br_color, // border
            hl_color, // highlight
            bg_color; // background

public:
    void load();
    void save();

    bool isAlwaysShow(HWND hwnd);

    void updatePosition();

private:
    void init();
    void useDefault();

    bool isAlwaysShowClass(HWND hwnd);
    bool isAlwaysShowProgram(HWND hwnd);

    tstring configPath_; // <User Home>\<app_name>
    // The file which contains the list of exceptional programs
    // Windows of an exceptional program will show in all workspaces
    tstring exceptPath_;

    tstring configRKey_; // HKCU\Software\<app_name>

    std::list<tstring> excepts_;
};

struct Buffer {
    HPEN    brpen;
    HBRUSH  brbrh, bgbrh, hlbrh;
    DWORD   guicx, guicy;

    void create();
    void remove();
    void update();
    void update_guicx();
    void update_guicy();
};

extern Option option;
extern Buffer buffer;

#endif // _OPTION_H_
