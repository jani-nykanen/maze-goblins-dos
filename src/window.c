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

    i32 EDI;
    i32 ESI;
    i32 EBP;
    i32 reserved;
    i32 EBX;
    i32 EDX;
    i32 ECX;
    i32 EAX;
    i16 flags;
    i16 ES,DS,FS,GS,IP,CS,SP,SS;

} RMInfo;


typedef struct {

    Canvas* framebuffer;

    i16 frameCounter;
    i16 frameSkip;

    UpdateCallback update;
    RedrawCallback redraw;
    
} _Window;

/*
static void simulate_real_mode_interrupt(i16 interruptNumber, i32 flags) {

    const i16 INT_NO = 0x31;

    union REGS regs;
    struct SREGS sregs;

    static RMInfo RMI;

    memset(&RMI, 0, sizeof(RMInfo));
    RMI.EAX = flags;
    RMI.DS = 0; // segment;
    RMI.EDX = 0;

    regs.w.ax = 0x0300;
    regs.h.bl = interruptNumber;
    regs.h.bh = 0;
    regs.w.cx = 0;
    sregs.es = FP_SEG(&RMI);
    regs.x.edi = FP_OFF(&RMI);
    int386x(INT_NO, &regs, &regs, &sregs);
}


static void set_video_mode(u16 mode) {

    simulate_real_mode_interrupt(0x10, mode);
}
*/


static void vblank() {

    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}


static void init_graphics() {

    _setvideomode(_MRES256COLOR);
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

    if (window == NULL) return;

    dispose_canvas(window->framebuffer);
    m_free(window);

    reset_keyboard_listener();
    // set_video_mode(0x3);
    _setvideomode(_DEFAULTMODE);
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
