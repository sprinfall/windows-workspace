#ifndef _OPTION_H_
#define _OPTION_H_

#include "type.h"
#include "tstring.h"
#include <windows.h>
#include <set>

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
#define DefCellCy 18

#define BdrSize 1 // window border
#define GapSize 4 // gap between border and highlight

class Option {
public:
	DWORD	screen_cx, screen_cy;

	DWORD	transparency; // 0, 1 ~ 9 (0%, 10% ~ 90%)
	DWORD	number;

	DWORD	cell_cx,
			cell_cy;

	LONG	position_x,
			position_y;

	DWORD	br_color, // border
			hl_color, // highlight
			bg_color; // background

	WORD	hk_next,
			hk_prev,
			hk_tail,
			hk_head,
			hk_create,
			hk_remove;

	std::set<tstring>	except_app,
						except_cls;

public:
	void load();
	void save();

	bool is_except_cls(HWND hwnd);
	bool is_except_app(HWND hwnd);
	bool is_except(HWND hwnd);

	void except_clr();
	void except_add(const TCHAR *path);

	void updatePosition();

private:
	void init();
	void useDefault();

	void load_except();

	TCHAR config_path_[MAX_PATH]; // <user_home>\<app_name>
	TCHAR config_rkey_[MAX_PATH]; // HKCU\Software\<app_name>
	TCHAR except_app_file_[MAX_PATH]; // <config_path_>\<except_app.txt>
	TCHAR except_cls_file_[MAX_PATH]; // <config_path_>\<except_cls.txt>
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
	void update_br_color();
	void update_bg_color();
	void update_hl_color();
};

extern Option option;
extern Buffer buffer;

#endif // _OPTION_H_
