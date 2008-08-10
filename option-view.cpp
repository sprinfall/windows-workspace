#include "option-view.h"
#include "resource.h"
#include "option.h"
#include "global.h"
#include <commctrl.h>

HWND opt_hwnd = NULL;

OptionView::OptionView()
{
    pages_[0].dwSize = sizeof(PROPSHEETPAGE);
    pages_[0].dwFlags = PSP_USETITLE;
    pages_[0].hInstance = app_hins;
    pages_[0].pszTemplate = MAKEINTRESOURCE(IDD_PAGE_1);
    pages_[0].pszIcon = NULL;
    pages_[0].pfnDlgProc = page1Proc;
    pages_[0].pszTitle = _T("外观");
    pages_[0].lParam = 0;
    pages_[0].pfnCallback = NULL;

    _stprintf_s(optionTitle, MAX_PATH, _T("%s - %s"), _T("选项"), app_name);

    sheet_.dwSize = sizeof(PROPSHEETHEADER);
    sheet_.dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP | PSH_USECALLBACK;// | PSH_MODELESS;
    sheet_.hwndParent = app_hwnd;
    sheet_.hInstance = app_hins;
    sheet_.pszIcon = MAKEINTRESOURCE(IDI_APP_NORMAL);
    sheet_.pszCaption = optionTitle;
    sheet_.nPages = sizeof(pages_) / sizeof(PROPSHEETPAGE);
    sheet_.nStartPage = 0;
    sheet_.ppsp = (LPCPROPSHEETPAGE)&pages_;
    sheet_.pfnCallback = propSheetProc;
}

void OptionView::centerSheet()
{
    RECT rect;
    GetWindowRect(opt_hwnd, &rect);
    LONG cx = rect.right - rect.left;
    LONG cy = rect.bottom - rect.top;
    SetWindowPos(opt_hwnd, NULL, (option.screen_cx-cx)/2, (option.screen_cy-cy)/2,
        0, 0, SWP_NOSIZE);
}

void OptionView::show()
{
    PropertySheet(&sheet_); // block
    opt_hwnd = NULL; // reset
}

void OptionView::create()
{
    OptionView optView;
    optView.show();
}

int CALLBACK OptionView::propSheetProc(HWND hdlg, UINT msg, LPARAM lParam)
{
    if (msg == PSCB_INITIALIZED) { opt_hwnd = hdlg; }
    return 0;
}

static void correctDlgItemInt(HWND hdlg, int item, UINT min, UINT max, UINT def)
{
    UINT size = GetDlgItemInt(hdlg, item, 0, FALSE);
    if (size < min || size > max) { SetDlgItemInt(hdlg, item, def, FALSE); }
}

static const TCHAR* TransNames[] = {
    _T("10"), _T("20"), _T("30"), _T("40"), _T("50"),
    _T("60"), _T("70"), _T("80"), _T("90")
};

// Page1: Appearance
BOOL CALLBACK OptionView::page1Proc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND sizeX, sizeY;
    static HWND spinSizeX, spinSizeY;

    static COLORREF brColor = option.br_color;
    static COLORREF hlColor = option.hl_color;
    static COLORREF bgColor = option.bg_color;

    static HWND checkTrans, comboTrans;

    switch (msg) {

    case WM_INITDIALOG:
        centerSheet(); // Move option window to screen center

        sizeX = GetDlgItem(hdlg, IDC_SIZE_X);
        sizeY = GetDlgItem(hdlg, IDC_SIZE_Y);

        spinSizeX = GetDlgItem(hdlg, IDC_SPIN_SIZE_X);
        spinSizeY = GetDlgItem(hdlg, IDC_SPIN_SIZE_Y);

        checkTrans = GetDlgItem(hdlg, IDC_CHECK_TRANS);
        comboTrans = GetDlgItem(hdlg, IDC_COMBO_TRANS);

        // Limit text for edit controls
        SendMessage(sizeX, EM_SETLIMITTEXT, 2, 0);
        SendMessage(sizeY, EM_SETLIMITTEXT, 2, 0);

        // Set range for edit controls
        SendMessage(spinSizeX, UDM_SETRANGE, 0, (LPARAM)MAKELONG(MaxCellCx, MinCellCx));
        SendMessage(spinSizeY, UDM_SETRANGE, 0, (LPARAM)MAKELONG(MaxCellCy, MinCellCy));

        // Set buddies for up-down controls
        SendMessage(spinSizeX, UDM_SETBUDDY, (WPARAM)sizeX, 0);
        SendMessage(spinSizeY, UDM_SETBUDDY, (WPARAM)sizeY, 0);

        // Init values
        SetDlgItemInt(hdlg, IDC_SIZE_X, option.cell_cx, FALSE);
        SetDlgItemInt(hdlg, IDC_SIZE_Y, option.cell_cy, FALSE);

        // Add items to transparency combobox
        for (int i = 0; i < MaxTransparency; ++i) {
            SendMessage(comboTrans, CB_ADDSTRING, 0, (LPARAM)TransNames[i]);
        }

        if (option.transparency == 0) {
            SendDlgItemMessage(hdlg, IDC_CHECK_TRANS, BM_SETCHECK, BST_UNCHECKED, 0);
            EnableWindow(comboTrans, FALSE); // Disable transparency combobox
            SetWindowText(comboTrans, TransNames[2]); // default 30%
        } else {
            SendDlgItemMessage(hdlg, IDC_CHECK_TRANS, BM_SETCHECK, BST_CHECKED, 0);
            EnableWindow(comboTrans, TRUE); // Enable transparency combobox
            SetWindowText(comboTrans, TransNames[option.transparency - 1]);
        }

        break;

    case WM_NOTIFY:
        {
        LPNMHDR nmhdr = (LPNMHDR)lParam;

        switch (nmhdr->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_KILLACTIVE:
            // Validate changes
            SetWindowLong(hdlg, DWL_MSGRESULT, TRUE);
            break;

        case PSN_APPLY:
            // Apply changes
            option.cell_cx = GetDlgItemInt(hdlg, IDC_SIZE_X, 0, FALSE);
            option.cell_cy = GetDlgItemInt(hdlg, IDC_SIZE_Y, 0, FALSE);

            {
                DWORD newTrans = 0;
                LRESULT checked = SendDlgItemMessage(hdlg, IDC_CHECK_TRANS, BM_GETCHECK, 0, 0);
                if (checked == BST_CHECKED) { // Use transparency
                    TCHAR transStr[3];
                    GetWindowText(comboTrans, transStr, 8);
                    newTrans = (DWORD)(_ttoi(transStr) / 10); // "x0" -> x
                } // else: newTrans = 0

                if (newTrans != option.transparency) {
                    // Update trans menu check status
                    // option.transparency will be updated in Menuer
                    // TODO: refine
                    SendMessage(app_hwnd, WM_COMMAND, (LPARAM)MAKELONG(IDM_TRANS_0 + newTrans, 0), 0);
                }
            }

            option.br_color = brColor;
            option.hl_color = hlColor;
            option.bg_color = bgColor;

            option.save();
            buffer.update();

            InvalidateRect(app_hwnd, NULL, TRUE);
            SetWindowLong(hdlg, DWL_MSGRESULT, PSNRET_INVALID); //PSNRET_NOERROR
            break;

        //case PSN_RESET: // Cancel button is clicked
        //   break;
        }
        }
        break;

    case WM_COMMAND:

        switch (HIWORD(wParam)) {
        case EN_UPDATE:
            break;

        case EN_CHANGE:
            PropSheet_Changed(opt_hwnd, hdlg);
            break;

        case EN_KILLFOCUS:
            correctDlgItemInt(hdlg, IDC_SIZE_X, MinCellCx, MaxCellCx, DefCellCx);
            correctDlgItemInt(hdlg, IDC_SIZE_Y, MinCellCy, MaxCellCy, DefCellCy);
            break;

        case CBN_SELCHANGE: // combobox select change
            PropSheet_Changed(opt_hwnd, hdlg);
            break;

        case BN_CLICKED: // Some button/checkbox has been clicked
            // The lower word of wParam holds the controls ID
            if (LOWORD(wParam) == IDC_CHECK_TRANS) {
                LRESULT checked = SendDlgItemMessage(hdlg, IDC_CHECK_TRANS, BM_GETCHECK, 0, 0);
                // Enable/Disable IDC_COMBO_TRANS
                EnableWindow(comboTrans, checked == BST_CHECKED);
                PropSheet_Changed(opt_hwnd, hdlg);
            }
            break;
        }

        switch (LOWORD(wParam)) {
        case IDC_SIZE_USEDEF:
            SetDlgItemInt(hdlg, IDC_SIZE_X, DefCellCx, FALSE);
            SetDlgItemInt(hdlg, IDC_SIZE_Y, DefCellCy, FALSE);
            PropSheet_Changed(opt_hwnd, hdlg);
            break;

        case IDC_COLOR_HL:
            {
                // Enable "Apply" only when color changes
                COLORREF oldColor = hlColor;
                hlColor = choose_color(hdlg, hlColor);
                if (hlColor != oldColor) { PropSheet_Changed(opt_hwnd, hdlg); }
            }
            break;

        case IDC_COLOR_BR:
            {
                // Enable "Apply" only when color changes
                COLORREF oldColor = brColor;
                brColor = choose_color(hdlg, brColor);
                if (brColor != oldColor) { PropSheet_Changed(opt_hwnd, hdlg); }
            }
            break;

        case IDC_COLOR_BG:
            {
                // Enable "Apply" only when color changes
                COLORREF oldColor = bgColor;
                bgColor = choose_color(hdlg, bgColor);
                if (bgColor != oldColor) { PropSheet_Changed(opt_hwnd, hdlg); }
            }
            break;
        }

        break;
    }

    return FALSE;
}
