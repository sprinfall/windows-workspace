#ifndef _OPTION_VIEW_H_
#define _OPTION_VIEW_H_

#include <windows.h>

class OptionView {
public:
    static void create();
    void show();

    static int CALLBACK propSheetProc(HWND hdlg, UINT msg, LPARAM lParam);

    static BOOL CALLBACK page1Proc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    OptionView();
    OptionView(const OptionView&);
    OptionView& operator==(const OptionView &);

    static void centerSheet();

private:
    PROPSHEETPAGE pages_[1];
    PROPSHEETHEADER sheet_;

    TCHAR optionTitle[MAX_PATH];
};

#endif // _OPTION_VIEW_H_
