#ifndef PROJECTNAME_MENU_H
#define PROJECTNAME_MENU_H


#include "types.h"
#include "window.h"


EMPTY_STRUCT(Menu);


typedef void (*MenuCallback)(Menu*, i16 button, Window*);


void draw_box(Canvas* canvas, i16 x, i16 y, i16 w, i16 h);


Menu* new_menu(const char* buttonText[], u16 buttonCount, MenuCallback cb);
void dispose_menu(Menu* menu);

void menu_update(Menu* menu, Window* window);
void menu_draw(Menu* menu, Canvas* canvas, 
    Bitmap* bmpFont, Bitmap* bmpFontYellow,
    i16 x, i16 y, i16 xoff, i16 yoff);

void menu_activate(Menu* menu, i16 cursorPos);
void menu_deactivate(Menu* menu);

bool menu_is_active(Menu* menu);

void menu_change_button_text(Menu* menu, i16 buttonIndex, const char* newName);


#endif // PROJECTNAME_MENU_H

