#include "start.h"
#include "system.h"
#include "keyb.h"
#include "title.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {

    AssetCache* assets;
    Bitmap* bmpStart;

    bool drawn;
    i16 timer;

} StartScene;

static StartScene* start = NULL;


static void change_scene_callback(Window* window) {

    if (init_title_screen_scene(window, start->assets) == 1) {

        window_terminate(window);
        return;

    }
    dispose_start_scene();
    register_title_screen_scene(window);
}


static void update_start(Window* window, i16 step) {

    if (keyboard_any_pressed() || (start->timer -= step) <= 0) {

        window_start_transition(window, true, 3, 
            TRANSITION_DARKEN, change_scene_callback);
    }
}


static void draw_start(Canvas* canvas) {

    u16 w, h;
    u16 bw, bh;

    if (start->drawn) return;

    canvas_get_size(canvas, &w, &h);
    bitmap_get_size(start->bmpStart, &bw, &bh);

    canvas_clear(canvas, 0);
    canvas_draw_bitmap_fast(canvas, start->bmpStart,
        (i16)(w/2 - bw/2), (i16)(h/2 - bh/2));

    start->drawn = true;
}


i16 init_start_scene(Window* window, AssetCache* assets) {

    const i16 WAIT_TIME = 140;

    start = (StartScene*) calloc(1, sizeof(StartScene));
    if (start == NULL) {

        m_memory_error();
        return 1;
    }

    start->assets = assets;

    window_draw_loading_screen(window, 0);

    if ((start->bmpStart = load_bitmap("START.BIN")) == NULL) {

        dispose_start_scene();
        return 1;
    }

    start->timer = WAIT_TIME;
    start->drawn = false;

    window_start_transition(window, false, 3, TRANSITION_DARKEN, NULL);

    return 0;
}


void dispose_start_scene() {

    if (start == NULL)
        return;

    dispose_bitmap(start->bmpStart);
    m_free(start);
}


void register_start_scene(Window* window) {
    
    window_register_callback_functions(window,
        update_start, 
        draw_start);
}
