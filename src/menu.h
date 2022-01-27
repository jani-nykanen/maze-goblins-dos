#ifndef PROJECTNAME_MENU_H
#define PROJECTNAME_MENU_H


#include "types.h"
#include "window.h"


EMPTY_STRUCT(Menu);


typedef void (*MenuCallback)(Menu*, Window*);


Menu* new_menu(const char** buttonText, u16 buttonCount);
void dispose_menu(Menu* menu);

void menu_update(Menu* menu, i16 step);
void menu_draw(Menu* menu, Canvas* canvas, Bitmap* bmpFont, Bitmap* bmpFontYellow);

void menu_activate(Menu* menu, i16 cursorPos);
void menu_deactivate(Menu* menu);


#endif // PROJECTNAME_MENU_H

