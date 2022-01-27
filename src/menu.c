#include "menu.h"
#include "system.h"
#include "mathext.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {

    char** buttons;
    u16 buttonCount;

    i16 cursorPos;
    bool active;

} _Menu;


Menu* new_menu(const char** buttonText, u16 buttonCount) {

    u16 i;

    _Menu* menu = (_Menu*) calloc(1, sizeof(_Menu));
    if (menu == NULL) {

        m_memory_error();
        return NULL;
    }

    menu->buttons = (char**) calloc(buttonCount, sizeof(char*));
    if (menu->buttons == NULL) {

        m_memory_error();
        dispose_menu(menu);
        return NULL;
    }

    for (i = 0; i < buttonCount; ++ i) {

        menu->buttons[i] = (char*) calloc(strlen(buttonText[i]) +1, sizeof(char));
        if (menu->buttons[i] == NULL) {

            m_memory_error();
            dispose_menu(menu);
            return NULL;
        }
        strcpy(menu->buttons[i], buttonText[i]);
    }

    menu->buttonCount = buttonCount;
    menu->cursorPos = 0;

    return (Menu*) menu;
}


void dispose_menu(Menu* _menu) {

    _Menu* menu = (_Menu*) _menu;
    u16 i;

    if (menu == NULL) return;

    if (menu->buttons != NULL) {

        for (i = 0; i < menu->buttonCount; ++ i) {

            m_free(menu->buttons[i]);
        }
        m_free(menu->buttons);
    }
    m_free(menu);
}


void menu_update(Menu* _menu, i16 step) {

    _Menu* menu = (_Menu*) _menu;
}


void menu_draw(Menu* _menu, Canvas* canvas, 
    Bitmap* bmpFont, Bitmap* bmpFontYellow, 
    i16 x, i16 y) {

    _Menu* menu = (_Menu*) _menu;
}


void menu_activate(Menu* _menu, i16 cursorPos) {

    _Menu* menu = (_Menu*) _menu;

    menu->active = true;
    menu->cursorPos = clamp_i16(cursorPos, 0, menu->buttonCount-1);
}


void menu_deactivate(Menu* _menu) {

    _Menu* menu = (_Menu*) _menu;

    menu->active = false;
}


bool menu_is_active(Menu* _menu) {

    _Menu* menu = (_Menu*) _menu;

    return menu->active;
}
