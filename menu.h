#ifndef _MENU_H
#define _MENU_H

void menu_init();
void menu_fini();
void menu_pop_tray();
void menu_pop_float();
void menu_update_tip(size_t current, size_t total);

void menu_check_autorun();

#endif // _MENU_H
