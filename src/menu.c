#include "menu.h"
#include "system.h"
#include "mathext.h"
#include "keyb.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {

    char** buttons;
    u16 buttonCount;
    u16 longestButtonNameLength;
    MenuCallback callback;

    i16 cursorPos;
    i16 oldCursorPos;
    bool active;
    bool drawn;

} _Menu;


static u16 find_longest_button_name(_Menu* menu) {

    i16 i;

    u16 len;
    u16 max = (u16) strlen(menu->buttons[0]);

    for (i = 1; i < menu->buttonCount; ++ i) {

        len = (u16) strlen(menu->buttons[i]);
        if (len > max) {

            max = len;
        }
    }

    return max;
}


void draw_box(Canvas* canvas, i16 x, i16 y, i16 w, i16 h) {

    const u8 COLORS[] = {10, 0, 255};

    i16 i;

    for (i = 2; i >= 0; -- i) {

        canvas_fill_rect(canvas, x-i, y-i, w+i*2, h+i*2, COLORS[i]);
    }
}


Menu* new_menu(const char* buttonText[], u16 buttonCount, MenuCallback cb) {

    u16 i;

    _Menu* menu = (_Menu*) calloc(1, sizeof(_Menu));
    if (menu == NULL) {

        m_memory_error();
        return NULL;
    }

    menu->buttons = (char**) calloc(buttonCount, sizeof(char*));
    if (menu->buttons == NULL) {

        m_memory_error();
        dispose_menu((Menu*) menu);
        return NULL;
    }

    for (i = 0; i < buttonCount; ++ i) {

        menu->buttons[i] = (char*) calloc(strlen(buttonText[i]) +1, sizeof(char));
        if (menu->buttons[i] == NULL) {

            m_memory_error();
            dispose_menu((Menu*) menu);
            return NULL;
        }
        strcpy(menu->buttons[i], buttonText[i]);
    }

    menu->buttonCount = buttonCount;
    menu->cursorPos = 0;
    menu->oldCursorPos = 0;
    menu->callback = cb;
    menu->drawn = false;

    menu->longestButtonNameLength = find_longest_button_name(menu);

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


void menu_update(Menu* _menu, Window* window) {

    _Menu* menu = (_Menu*) _menu;
    AudioSystem* audio;

    if (!menu->active) return;

    audio = window_get_audio_system(window);
    menu->oldCursorPos = menu->cursorPos;

    if (keyboard_get_extended_key(KEY_UP) == STATE_PRESSED) {

        -- menu->cursorPos;
    }
    else if (keyboard_get_extended_key(KEY_DOWN) == STATE_PRESSED) {

        ++ menu->cursorPos;
    }
    menu->cursorPos = neg_mod_i16(menu->cursorPos, menu->buttonCount);

    if (menu->oldCursorPos != menu->cursorPos) {

        audio_play_predefined_sample(audio, SAMPLE_CHOOSE);
    }

    if (keyboard_get_normal_key(KEY_RETURN) == STATE_PRESSED) {

        menu->callback((Menu*) menu, menu->cursorPos, window);

        audio_play_predefined_sample(audio, SAMPLE_SELECT);
    }
}


void menu_draw(Menu* _menu, Canvas* canvas, 
    Bitmap* bmpFont, Bitmap* bmpFontYellow, 
    i16 x, i16 y, i16 xoff, i16 yoff) {

    static const i16 BOX_OFFSET_X = 6;
    static const i16 BOX_OFFSET_Y = 5; 

    _Menu* menu = (_Menu*) _menu;
    Bitmap* bmp;

    i16 i;
    u16 w, h;
    i16 bw, bh;
    i16 dx, dy;

    bool redrawButtons = menu->cursorPos != menu->oldCursorPos;

    if (!menu->active) return;

    canvas_get_size(canvas, &w, &h);

    bw = (i16) (menu->longestButtonNameLength * (8 + xoff)) ;
    bh = menu->buttonCount * (8 + yoff);

    // TODO: Do not assume the width of a character, obtain from the 
    // given bitmap(s)
    dx = (i16) (w / 2) - (i16) (bw / 2) + x;
    dy = (i16) (h / 2) - (i16) (bh / 2) + y;

    // Box
    if (!menu->drawn) {

        draw_box(canvas,
            dx - BOX_OFFSET_X, dy - BOX_OFFSET_Y,
            bw + BOX_OFFSET_X*2, bh + BOX_OFFSET_Y*2);
    }   

    for (i = 0; i < menu->buttonCount; ++ i) {

        bmp = i == menu->cursorPos ? bmpFontYellow : bmpFont;

        if (menu->drawn || redrawButtons) {

            canvas_draw_text(canvas, bmp, menu->buttons[i], 
                dx, dy + i * (8 + yoff), xoff, yoff, ALIGN_LEFT);
        }
    }

    menu->drawn = true;
}


void menu_activate(Menu* _menu, i16 cursorPos) {

    _Menu* menu = (_Menu*) _menu;

    menu->active = true;
    menu->cursorPos = clamp_i16(cursorPos, 0, menu->buttonCount-1);
    menu->oldCursorPos = menu->cursorPos;
    menu->drawn = false;
}


void menu_deactivate(Menu* _menu) {

    _Menu* menu = (_Menu*) _menu;

    menu->active = false;
}


bool menu_is_active(Menu* _menu) {

    _Menu* menu = (_Menu*) _menu;

    return menu->active;
}


void menu_change_button_text(Menu* _menu, i16 buttonIndex, const char* newName) {

    _Menu* menu = (_Menu*) _menu;

    if (buttonIndex < 0 ||
        buttonIndex >= menu->buttonCount ||
        strlen(newName) > strlen(menu->buttons[buttonIndex]))
        return;

    strcpy(menu->buttons[buttonIndex], newName);

    menu->drawn = false;
}
