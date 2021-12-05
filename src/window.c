#include "window.h"
#include "system.h"
#include "canvas.h"
#include "keyb.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dos.h>
#include <conio.h>
#include <i86.h>
#include <graph.h>


typedef struct {

    Canvas* framebuffer;

    i16 frameCounter;
    i16 frameSkip;

    UpdateCallback update;
    RedrawCallback redraw;

} _Window;



static void vblank() {

    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}


static void set_palette() {

    const u32 PALETTE_INDEX = 0x03c8;
    const u32 PALETTE_DATA = 0x03c9;

    u8 i = 0;
    u8 r, g, b;

    outp(PALETTE_INDEX,0);
    do {

        r = i >> 5;
        g = i << 3;
        g >>= 5;
        b = i << 6;
        b >>= 6;

        r *= 31;
        g *= 31;
        b *= 85;

        if (r >= 248) r = 255;
        if (g >= 248) g = 255;

        outp(PALETTE_DATA, r / 4);
        outp(PALETTE_DATA, g / 4);
        outp(PALETTE_DATA, b / 4);

        
    }
    while ((++ i) != 0);
}


static void init_graphics() {

    _setvideomode(_MRES256COLOR);
    set_palette();
}


static bool check_default_key_shortcuts() {

    if (keyboard_get_normal_key(KEY_Q) == STATE_PRESSED &&
        (keyboard_get_normal_key(KEY_LCTRL) & STATE_DOWN_OR_PRESSED)) {

        return true;
    }
    return false;
}


static void copy_canvas_to_screen(_Window* window) {

    static const u32 SCREEN_LIN_ADDR = 0xA000 << 4;
    
    canvas_copy_to_memory_location(window->framebuffer, SCREEN_LIN_ADDR);
}


static bool loop(_Window* window) {

    if ( (window->frameCounter ++) == window->frameSkip) {

        window->frameCounter = 0;
    
        if (window->update != NULL) {

            if (window->update(window->frameSkip+1))
                return true;
        }

        if (check_default_key_shortcuts())
                return true;
        keyboard_update();

        if (window->redraw != NULL) {

            window->redraw(window->framebuffer);
        }

        vblank();  
        copy_canvas_to_screen(window);
    }
    else {

        vblank();
    }

    return false;
}


Window* new_window(u16 width, u16 height, str caption, i16 frameSkip) {

    _Window* w = (_Window*) calloc(1, sizeof(_Window));
    if (w == NULL) {

        m_memory_error();
        return NULL;
    }

    w->framebuffer = new_canvas(320, 200);
    if (w->framebuffer == NULL) {

        dispose_window((Window*) w);
        return NULL;
    }
    canvas_clear(w->framebuffer, 32);

    init_graphics();
    init_keyboard_listener();

    w->frameCounter = 0;
    w->frameSkip = frameSkip;

    w->update = NULL;
    w->redraw = NULL;

    return (Window*) w;
}


void dispose_window(Window* _window) {

    _Window* window = (_Window*) _window;

    reset_keyboard_listener();
    _setvideomode(_DEFAULTMODE);

    if (window == NULL) return;

    dispose_canvas(window->framebuffer);
    m_free(window);
}


void window_register_callback_functions(Window* _window,
    UpdateCallback update,
    RedrawCallback redraw) {

    _Window* window = (_Window*) _window;

    window->update = update;
    window->redraw = redraw;
}


void window_make_active(Window* _window) {

    _Window* window = (_Window*) _window;

    while (!loop(window));
}
