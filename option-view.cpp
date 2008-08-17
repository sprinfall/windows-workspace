#include "resource.h"
#include "option.h"
#include "global.h"
#include <commctrl.h>
#include <list>

HWND opt_hwnd = NULL;

static BOOL CALLBACK prop_sheet_proc(HWND hdlg, UINT msg, LPARAM lParam);
static BOOL CALLBACK prop_page1_proc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK prop_page2_proc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK prop_page3_proc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

static void
fill_propsheet_page(PROPSHEETPAGE &page,
                    DWORD id,
                    DLGPROC proc,
                    const TCHAR *title)
{
    page.dwSize        = sizeof(PROPSHEETPAGE);
    page.dwFlags       = PSP_USEICONID | PSP_USETITLE;
    page.hInstance     = app_hins;
    page.pszTemplate   = MAKEINTRESOURCE(id);
    page.pszIcon       = NULL;
    page.pfnDlgProc    = proc;
    page.pszTitle      = title;
    page.lParam        = 0;
    page.pfnCallback   = NULL;
}

void
option_view_create()
{
    hotkey_unregister(app_hwnd);

    PROPSHEETPAGE pages[3];
    PROPSHEETHEADER sheet;

    fill_propsheet_page(pages[0], IDD_PAGE_1, prop_page1_proc, _T("外观"));
    fill_propsheet_page(pages[1], IDD_PAGE_2, prop_page2_proc, _T("例外"));
    fill_propsheet_page(pages[2], IDD_PAGE_3, prop_page3_proc, _T("热键"));

    TCHAR title[MAX_PATH];
    _stprintf_s(title, MAX_PATH, _T("选项 - %s"), app_name);

    sheet.dwSize       = sizeof(PROPSHEETHEADER);
    sheet.dwFlags      = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP | PSH_USECALLBACK;
    sheet.hwndParent   = app_hwnd;
    sheet.hInstance    = app_hins;
    sheet.pszIcon      = MAKEINTRESOURCE(IDI_APP_16);
    sheet.pszCaption   = title;
    sheet.nPages       = sizeof(pages) / sizeof(PROPSHEETPAGE);
    sheet.nStartPage   = 0;
    sheet.ppsp         = (LPCPROPSHEETPAGE)&pages;
    sheet.pfnCallback  = prop_sheet_proc;

    PropertySheet(&sheet);

    option.save();
    hotkey_register(app_hwnd);
}

static BOOL CALLBACK
prop_sheet_proc(HWND hdlg, UINT msg, LPARAM lParam)
{
    if (msg == PSCB_INITIALIZED) { opt_hwnd = hdlg; }
    return 0;
}

//static void
//correctDlgItemInt(HWND hdlg, int item, UINT min, UINT max, UINT def)
//{
//    UINT size = GetDlgItemInt(hdlg, item, 0, FALSE);
//    if (size < min || size > max) { SetDlgItemInt(hdlg, item, def, FALSE); }
//}

inline
static void ctrl_enable(HWND ctrl, bool enabled)
{
    EnableWindow(ctrl, (BOOL)enabled);
}

inline
static void ctrl_set_check(HWND hdlg, DWORD check_id, bool checked)
{
    SendDlgItemMessage(hdlg, check_id, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
}


//-----------------------------------------------------------------------------
// option page 1

static const TCHAR* TransNames[] = {
    _T("10"), _T("20"), _T("30"), _T("40"), _T("50"),
    _T("60"), _T("70"), _T("80"), _T("90")
};

static BOOL CALLBACK
prop_page1_proc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND edit_cellcx, edit_cellcy;
    static HWND spin_cellcx, spin_cellcy;
    static HWND push_celldc;
    static HWND check_trans, combo_trans;

    static COLORREF br_color = option.br_color,
                    hl_color = option.hl_color,
                    bg_color = option.bg_color;

    switch (msg) {

    case WM_INITDIALOG:
         // move option window to screen center
        center_screen(opt_hwnd);

        edit_cellcx = GetDlgItem(hdlg, IDC_EDIT_CELL_CX);
        edit_cellcy = GetDlgItem(hdlg, IDC_EDIT_CELL_CY);
        spin_cellcx = GetDlgItem(hdlg, IDC_SPIN_CELL_CX);
        spin_cellcy = GetDlgItem(hdlg, IDC_SPIN_CELL_CY);
        push_celldc = GetDlgItem(hdlg, IDC_PUSH_CELL_DC);

        check_trans = GetDlgItem(hdlg, IDC_CHECK_TRANS);
        combo_trans = GetDlgItem(hdlg, IDC_COMBO_TRANS);

        // limit text user can input for edit control
        SendMessage(edit_cellcx, EM_SETLIMITTEXT, 2, 0);
        SendMessage(edit_cellcy, EM_SETLIMITTEXT, 2, 0);

        // set range for edit control
        SendMessage(spin_cellcx, UDM_SETRANGE, 0, (LPARAM)MAKELONG(MaxCellCx, MinCellCx));
        SendMessage(spin_cellcy, UDM_SETRANGE, 0, (LPARAM)MAKELONG(MaxCellCy, MinCellCy));

        // set buddies for up-down control
        SendMessage(spin_cellcx, UDM_SETBUDDY, (WPARAM)edit_cellcx, 0);
        SendMessage(spin_cellcy, UDM_SETBUDDY, (WPARAM)edit_cellcy, 0);

        // initialize
        SetDlgItemInt(hdlg, IDC_EDIT_CELL_CX, option.cell_cx, FALSE);
        SetDlgItemInt(hdlg, IDC_EDIT_CELL_CY, option.cell_cy, FALSE);

        // add items to transparency combobox
        for (int i = 0; i < MaxTransparency; ++i) {
            SendMessage(combo_trans, CB_ADDSTRING, 0, (LPARAM)TransNames[i]);
        }
        {
            bool enabled = option.transparency > 0;
            ctrl_set_check(hdlg, IDC_CHECK_TRANS, enabled);
            ctrl_enable(combo_trans, enabled);
            // default trans: 30%
            SetWindowText(combo_trans, enabled ? TransNames[option.transparency-1] : TransNames[2]);
        }

        ctrl_enable(push_celldc, option.cell_cx != DefCellCx || option.cell_cy != DefCellCy);

        break;

    case WM_NOTIFY:
        {
            LPNMHDR nmhdr = (LPNMHDR)lParam;
            switch (nmhdr->code) {
            //case PSN_SETACTIVE:
            //    break;

            case PSN_KILLACTIVE: // validate changes
                SetWindowLong(hdlg, DWL_MSGRESULT, TRUE);
                break;

            case PSN_APPLY: // apply changes
                option.cell_cx = GetDlgItemInt(hdlg, IDC_EDIT_CELL_CX, 0, FALSE);
                option.cell_cy = GetDlgItemInt(hdlg, IDC_EDIT_CELL_CY, 0, FALSE);

                {
                    DWORD trans = 0;
                    LRESULT checked = SendDlgItemMessage(hdlg, IDC_CHECK_TRANS, BM_GETCHECK, 0, 0);
                    if (checked == BST_CHECKED) {
                        TCHAR str_trans[3];
                        GetWindowText(combo_trans, str_trans, 8);
                        trans = (DWORD)(_ttoi(str_trans) / 10); // "x0" -> x
                    }
                    if (trans != option.transparency) {
                        set_transparency(app_hwnd, trans);
                        option.transparency = trans;
                    }
                }

                if (br_color != option.br_color) {
                    option.br_color = br_color;
                    buffer.update_br_color();
                }
                if (hl_color != option.hl_color) {
                    option.hl_color = hl_color;
                    buffer.update_hl_color();
                }
                if (bg_color != option.bg_color) {
                    option.bg_color = bg_color;
                    buffer.update_bg_color();
                }

                buffer.update();

                InvalidateRect(app_hwnd, NULL, TRUE);
                SetWindowLong(hdlg, DWL_MSGRESULT, PSNRET_INVALID); //PSNRET_NOERROR
                break;
            }
        }
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam)) {
        // the lower word of wParam holds the control id

        case EN_UPDATE:
            if (LOWORD(wParam) == IDC_EDIT_CELL_CX || LOWORD(wParam) == IDC_EDIT_CELL_CY) {
                UINT cellcx = GetDlgItemInt(hdlg, IDC_EDIT_CELL_CX, 0, FALSE);
                UINT cellcy = GetDlgItemInt(hdlg, IDC_EDIT_CELL_CY, 0, FALSE);
                EnableWindow(push_celldc, cellcx != DefCellCx || cellcy != DefCellCy);
            }
            break;

        case EN_CHANGE: // edit control text change
            PropSheet_Changed(opt_hwnd, hdlg);
            break;

        // Since ES_READONLY is used for the edit control, it should be
        // unnecessary to do correction any more
        //case EN_KILLFOCUS:
        //    correctDlgItemInt(hdlg, IDC_EDIT_CELL_CX, MinCellCx, MaxCellCx, DefCellCx);
        //    correctDlgItemInt(hdlg, IDC_EDIT_CELL_CY, MinCellCy, MaxCellCy, DefCellCy);
        //    break;

        case CBN_SELCHANGE: // combobox select change
            PropSheet_Changed(opt_hwnd, hdlg);
            break;

        case BN_CLICKED: // button/checkbox clicked
            if (LOWORD(wParam) == IDC_CHECK_TRANS) {
                LRESULT checked = SendDlgItemMessage(hdlg, IDC_CHECK_TRANS, BM_GETCHECK, 0, 0);
                ctrl_enable(combo_trans, checked == BST_CHECKED);
                PropSheet_Changed(opt_hwnd, hdlg);
            }
            break;
        }

        switch (LOWORD(wParam)) {
        case IDC_PUSH_CELL_DC:
            SetDlgItemInt(hdlg, IDC_EDIT_CELL_CX, DefCellCx, FALSE);
            SetDlgItemInt(hdlg, IDC_EDIT_CELL_CY, DefCellCy, FALSE);
            PropSheet_Changed(opt_hwnd, hdlg);
            break;

        // enable "Apply" only when color changes
        case IDC_COLOR_HL:
            {
                COLORREF old_color = hl_color;
                hl_color = choose_color(hdlg, hl_color);
                if (hl_color != old_color) { PropSheet_Changed(opt_hwnd, hdlg); }
            }
            break;
        case IDC_COLOR_BR:
            {
                COLORREF old_color = br_color;
                br_color = choose_color(hdlg, br_color);
                if (br_color != old_color) { PropSheet_Changed(opt_hwnd, hdlg); }
            }
            break;
        case IDC_COLOR_BG:
            {
                COLORREF old_color = bg_color;
                bg_color = choose_color(hdlg, bg_color);
                if (bg_color != old_color) { PropSheet_Changed(opt_hwnd, hdlg); }
            }
            break;
        }

        break;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// option page 2

static void
list_add_item(HWND lv, const tstring& text)
{
    static int i = 0;

    TCHAR tmp[MAX_PATH];
    _tcscpy_s(tmp, MAX_PATH, text.c_str());

    LV_ITEM lvi;
    lvi.mask        = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvi.state       = 0;
    lvi.stateMask   = 0;
    lvi.iSubItem    = 0;
    lvi.iItem       = i++;
    lvi.pszText     = tmp;
    ListView_InsertItem(lv, &lvi);
}

// TODO: remove?
static void
list_add_item(HWND lv, int index, const tstring& text)
{
    TCHAR tmp[MAX_PATH];
    _tcscpy_s(tmp, MAX_PATH, text.c_str());

    LV_ITEM lvi;
    lvi.mask        = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvi.state       = 0;
    lvi.stateMask   = 0;
    lvi.iSubItem    = 0;
    lvi.iItem       = index;
    lvi.pszText     = tmp;
    ListView_InsertItem(lv, &lvi);
}

static WNDPROC list_def_proc;

static LRESULT
list_except_proc(HWND hlv, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HMENU menu = GetSubMenu(LoadMenu(app_hins, MAKEINTRESOURCE(IDR_MENU_EXCEPT_FLOAT)), 0);

    switch (message) {
    //case WM_CREATE: // seems no such message
    //    break;

    case WM_RBUTTONDOWN:
        // call default proc to select item
        CallWindowProc(list_def_proc, hlv, message, wParam, lParam);
        if (ListView_GetSelectedCount(hlv) > 0) {
            menu_popup(menu, hlv); // pop only when item(s) selected
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_EXCEPT_REMOVE:
            // let parent do the job
            SendMessage(GetParent(hlv), WM_COMMAND, (WPARAM)MAKELONG(IDM_EXCEPT_REMOVE, 0), 0);
            break;
        }

    default:
        // let default proc handle others
        return CallWindowProc(list_def_proc, hlv, message, wParam, lParam);
    }

    return 0;
}


static void
list_init(HWND lv)
{
    ListView_SetExtendedListViewStyle(lv, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    list_def_proc = (WNDPROC)SetWindowLong(lv, GWL_WNDPROC, (LONG)list_except_proc);

    LV_COLUMN lvc;
    lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
    lvc.fmt     = LVCFMT_LEFT;
    lvc.cx      = 280;
    ListView_InsertColumn(lv, 0, &lvc);

    // TODO: private option.excepts_ ?
    std::list<tstring>::const_iterator i;
    for (i = option.excepts_.begin(); i != option.excepts_.end(); ++i) {
        list_add_item(lv, *i);
    }
}

static BOOL CALLBACK
prop_page2_proc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND list_ex;
    static HWND push_ex_add, edit_ex_add;

    static bool is_ex_changed = false;

    switch (message) {

    case WM_INITDIALOG:
        list_ex = GetDlgItem(hdlg, IDC_LIST_EXCEPT);
        list_init(list_ex);

        push_ex_add = GetDlgItem(hdlg, IDC_PUSH_EXCEPT_ADD);
        EnableWindow(push_ex_add, FALSE); // by default

        edit_ex_add = GetDlgItem(hdlg, IDC_EDIT_EXCEPT_ADD);
        // limit text user can input
        SendMessage(edit_ex_add, EM_SETLIMITTEXT, MAX_PATH, 0);

        break;

    case WM_NOTIFY:
        {
        LPNMHDR nmhdr = (LPNMHDR)lParam;

        switch (nmhdr->code) {
        //case PSM_CHANGED:
        //    break;

        //case PSN_SETACTIVE:
        //    break;

        case PSN_APPLY:
            if (is_ex_changed) {
                option.except_clr();
                for (int i = 0; i < ListView_GetItemCount(list_ex); ++i) {
                    TCHAR text[MAX_PATH];
                    ListView_GetItemText(list_ex, i, 0, text, MAX_PATH);
                    option.except_add(text);
                }
                is_ex_changed = false;
            }
            break;
       }
       }
       break;

    case WM_COMMAND:
        switch (HIWORD(wParam)) {
        case EN_UPDATE:
            if (LOWORD(wParam) == IDC_EDIT_EXCEPT_ADD) {
                // text changed; if it is a valid path, enalbe ADD button
                TCHAR text[MAX_PATH];
                GetDlgItemText(hdlg, IDC_EDIT_EXCEPT_ADD, text, MAX_PATH);
                if (_tcslen(text) > 0 && is_file_existed(text, _T(".exe")) &&
                    !option.isExcept(text)) { // already exists
                    EnableWindow(push_ex_add, TRUE);
                } else {
                    EnableWindow(push_ex_add, FALSE);
                }
            }
            break;

        case EN_CHANGE:
            if (LOWORD(wParam) != IDC_EDIT_EXCEPT_ADD) {
                PropSheet_Changed(opt_hwnd, hdlg);
            }
            break;

        //case EN_KILLFOCUS:
        //    break;

        case BN_CLICKED: // some button/checkbox has been clicked
            if (LOWORD(wParam) == IDC_PUSH_EXCEPT_ADD) {
                UINT index = ListView_GetItemCount(list_ex);
                TCHAR text[MAX_PATH];
                GetDlgItemText(hdlg, IDC_EDIT_EXCEPT_ADD, text, MAX_PATH);

                list_add_item(list_ex, text);
                SetDlgItemText(hdlg, IDC_EDIT_EXCEPT_ADD, _T(""));
                ctrl_enable(push_ex_add, false);
                is_ex_changed = true;
                PropSheet_Changed(opt_hwnd, hdlg);
            }
            break;
        }

        switch (LOWORD(wParam)) {
        case IDM_EXCEPT_REMOVE:
            {
                if (ListView_GetSelectedCount(list_ex) > 0) {
                    for (int i = ListView_GetItemCount(list_ex)-1; i >= 0; --i) {
                        UINT state = ListView_GetItemState(list_ex, i, LVIS_SELECTED);
                        if (state & LVIS_SELECTED) {
                            ListView_DeleteItem(list_ex, i);
                        }
                    }
                    is_ex_changed = true;
                    PropSheet_Changed(opt_hwnd, hdlg);
                }
            }
            break;
        }

        break;
    }

    return FALSE;
}

static BOOL CALLBACK
prop_page3_proc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hk_next, hk_prev, hk_tail, hk_head;
    static HWND hk_create, hk_remove;

    switch (message) {

    case WM_INITDIALOG:
        hk_next = GetDlgItem(hdlg, IDC_HOTKEY_NEXT);
        hk_prev = GetDlgItem(hdlg, IDC_HOTKEY_PREV);
        hk_tail = GetDlgItem(hdlg, IDC_HOTKEY_TAIL);
        hk_head = GetDlgItem(hdlg, IDC_HOTKEY_HEAD);
        hk_create = GetDlgItem(hdlg, IDC_HOTKEY_CREATE);
        hk_remove = GetDlgItem(hdlg, IDC_HOTKEY_REMOVE);

        {
        // set hotkey rules
        WPARAM wp = (WPARAM)HKCOMB_NONE | HKCOMB_S;
        LPARAM lp = MAKELPARAM(HOTKEYF_ALT, 0);
        SendMessage(hk_next,    HKM_SETRULES, wp, lp); // no shift; alt default
        SendMessage(hk_prev,    HKM_SETRULES, wp, lp);
        SendMessage(hk_tail,    HKM_SETRULES, wp, lp);
        SendMessage(hk_head,    HKM_SETRULES, wp, lp);
        SendMessage(hk_create,  HKM_SETRULES, wp, lp);
        SendMessage(hk_remove,  HKM_SETRULES, wp, lp);
        }

#define SetHotkey(hk, value)\
SendMessage(hk, HKM_SETHOTKEY,\
    (WPARAM)MAKEWORD(LOBYTE(value), HOTKEYF_EXT | HIBYTE(value)),0);

        SetHotkey(hk_next, option.hk_next);
        SetHotkey(hk_prev, option.hk_prev);
        SetHotkey(hk_tail, option.hk_tail);
        SetHotkey(hk_head, option.hk_head);
        SetHotkey(hk_create, option.hk_create);
        SetHotkey(hk_remove, option.hk_remove);

        break;

    case WM_NOTIFY:
        {
        LPNMHDR nmhdr = (LPNMHDR)lParam;

        switch (nmhdr->code) {
        //case PSM_CHANGED:
        //    break;

        //case PSN_SETACTIVE:
        //    break;

        case PSN_APPLY:
            {
#define HkFilter(hk) (MAKEWORD(LOBYTE(hk), HIBYTE(hk) & ~HOTKEYF_EXT))

                WORD hk;
                hk = 0xffff & SendMessage(hk_next, HKM_GETHOTKEY, 0, 0);
                option.hk_next = HkFilter(hk);

                hk = 0xffff & SendMessage(hk_prev, HKM_GETHOTKEY, 0, 0);
                option.hk_prev = HkFilter(hk);

                hk = 0xffff & SendMessage(hk_tail, HKM_GETHOTKEY, 0, 0);
                option.hk_tail = HkFilter(hk);

                hk = 0xffff & SendMessage(hk_head, HKM_GETHOTKEY, 0, 0);
                option.hk_head = HkFilter(hk);

                hk = 0xffff & SendMessage(hk_create, HKM_GETHOTKEY, 0, 0);
                option.hk_create = HkFilter(hk);

                hk = 0xffff & SendMessage(hk_remove, HKM_GETHOTKEY, 0, 0);
                option.hk_remove = HkFilter(hk);
            }
            break;
       }
       }
       break;

    case WM_COMMAND:
        switch (HIWORD(wParam)) {
        //case EN_UPDATE:
        //    break;

        case EN_CHANGE:
            PropSheet_Changed(opt_hwnd, hdlg);
            break;

        //case EN_KILLFOCUS:
        //    break;

        //case BN_CLICKED:
        //    break;
        }

        break;
    }

    return FALSE;
}
