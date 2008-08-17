
-- Logging
MessageBox(NULL, _T("test"), _T("test"), 0);

-- Menu bitmap

HBITMAP bmCreate = LoadBitmap(app_hins, MAKEINTRESOURCE(IDBM_CREATE));
HBITMAP bmRemove = LoadBitmap(app_hins, MAKEINTRESOURCE(IDBM_REMOVE));
SetMenuItemBitmaps(floatMenu_, IDM_CREATE, MF_BYCOMMAND, bmCreate, bmCreate);
SetMenuItemBitmaps(floatMenu_, IDM_REMOVE, MF_BYCOMMAND, bmRemove, bmRemove);


-- Handle slider control message
BOOL CALLBACK dlgproc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {

    case WM_HSCROLL:
        switch (LOWORD(wParam)) {
        case TB_ENDTRACK:
        case TB_THUMBTRACK:
            {
            DWORD value = SendDlgItemMessage(hdlg, IDC_SLIDER_ALPHA, TBM_GETPOS, 0, 0);
            SetDlgItemInt(hdlg, IDC_ALPHA_VALUE, value, FALSE);
            PropSheet_Changed(prop_, hdlg);
            }
            break;
        }
        break;
...
}

-- Handle notify message

...
    switch (msg) {
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

        case PSN_RESET: // Cancel button is clicked
            break;
        }
        break;

