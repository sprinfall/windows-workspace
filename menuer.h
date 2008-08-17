#ifndef _MENUER_H
#define _MENUER_H

#include <windows.h>

class Menuer {
public:
    Menuer();
    ~Menuer();

    void init();
    void popupTray();
    void popupFloat();
    void updateTip(size_t current, size_t total);

    //void checkTransMenu(DWORD newValue);
    void checkAutorunMenu();

private:
    void checkMenu(DWORD item, bool checked);
    void loadMenu();

private:
    enum {TIP_SIZE = 64};

    HMENU trayMenu_, floatMenu_;
    NOTIFYICONDATA nid_;
};

#endif // _MENUER_H
