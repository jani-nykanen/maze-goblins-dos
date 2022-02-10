#include "window.h"
#include "system.h"
#include "keyb.h"
#include "palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dos.h>
#include <conio.h>
#include <i86.h>
#include <graph.h>


// Around a second
static const i16 TRANSITION_TIME = 70; 


typedef struct {

    Canvas* framebuffer;
    AudioSystem* audio;

    i16 frameCounter;
    i16 frameSkip;
    bool running;

    UpdateCallback update;
    RedrawCallback redraw;

    i16 transitionTimer;
    i16 transitionSpeed;
    bool fadingOut;
    TransitionCallback transitionCb;

    Bitmap* loadingBitmap;

} _Window;



static void vblank() {

    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}


static void init_graphics() {

    _setvideomode(_MRES256COLOR);
    init_palette();
}


static void check_default_key_shortcuts(_Window* window) {

    if (keyboard_get_normal_key(KEY_Q) == STATE_PRESSED &&
        (keyboard_get_normal_key(KEY_LCTRL) & STATE_DOWN_OR_PRESSED)) {

        window->running = false;
    }
}


static void copy_canvas_to_screen(_Window* window) {

    static const u32 SCREEN_LIN_ADDR = 0xA000 << 4;
    
    canvas_copy_to_memory_location(window->framebuffer, SCREEN_LIN_ADDR);
}


static void update_transition(_Window* window, i16 step) {

    i16 hue;

    if (window->transitionTimer <= 0) {

        canvas_set_global_hue(window->framebuffer, 0);
        return;
    }

    if ((window->transitionTimer -= window->transitionSpeed * step) <= 0) {

        if (window->fadingOut) {

            window->fadingOut = false;
            if (window->transitionCb != NULL) {

                window->transitionCb((Window*) window);
            }

            window->transitionTimer += TRANSITION_TIME;
        }
    }

    hue = (HUE_COUNT-1) * window->transitionTimer;
    hue /= TRANSITION_TIME;

    if (window->fadingOut)
        hue = (HUE_COUNT-1) - hue;

    canvas_set_global_hue(window->framebuffer, hue);
}


static void loop(_Window* window) {

    if ( (window->frameCounter ++) == window->frameSkip) {

        window->frameCounter = 0;
    
        if (window->update != NULL && window->transitionTimer <= 0) {

            window->update((Window*) window, window->frameSkip+1);
        }

        update_transition(window, window->frameSkip+1);
        audio_update(window->audio, window->frameSkip+1);

        check_default_key_shortcuts(window);
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

    w->audio = new_audio_system(64);
    if (w->audio == NULL) {

        dispose_window((Window*) w);
        return NULL;
    }

    init_graphics();
    init_keyboard_listener();

    w->frameCounter = 0;
    w->frameSkip = frameSkip;

    w->update = NULL;
    w->redraw = NULL;

    w->transitionCb = NULL;
    w->transitionTimer = 0;
    w->fadingOut = false;
    w->transitionSpeed = 1;

    w->running = false;

    w->loadingBitmap = NULL;

    return (Window*) w;
}


void dispose_window(Window* _window) {

    _Window* window = (_Window*) _window;

    reset_keyboard_listener();
    _setvideomode(_DEFAULTMODE);

    if (window == NULL) return;

    dispose_canvas(window->framebuffer);
    dispose_audio_system(window->audio);
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

    window->running = true;

    while (window->running) {

        loop(window);
    }
}


void window_start_transition(Window* _window,
    bool fadeOut, i16 speed, TransitionCallback cb) {

    _Window* window = (_Window*) _window;

    window->transitionSpeed = speed;
    window->fadingOut = fadeOut;
    window->transitionTimer = TRANSITION_TIME;
    window->transitionCb = cb;
}


AudioSystem* window_get_audio_system(Window* window) {

    return ((_Window*) window)->audio;
}


void window_terminate(Window* _window) {

    _Window* window = (_Window*) _window;

    window->running = false;
}   


void window_bind_loading_bitmap(Window* _window, Bitmap* bmp) {

    _Window* window = (_Window*) _window;

    window->loadingBitmap = bmp;
}


void window_draw_loading_screen(Window* _window) {

    _Window* window = (_Window*) _window;

    u16 w, h;
    u16 bw, bh;

    canvas_set_global_hue(window->framebuffer, 0);

    bitmap_get_size(window->loadingBitmap, &bw, &bh);
    canvas_get_size(window->framebuffer, &w, &h);

    canvas_clear(window->framebuffer, 0);
    canvas_draw_bitmap_fast(window->framebuffer, 
        window->loadingBitmap,
        w/2 - bw/2, h/2 - bh/2);

    // vblank();  
    copy_canvas_to_screen(window);
}   
