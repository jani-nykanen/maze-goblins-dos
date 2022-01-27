#include "game.h"
#include "system.h"
#include "tilemap.h"
#include "stage.h"
#include "keyb.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static const i16 WAIT_TIME = 70;


typedef struct {

    Bitmap* bmpStaticTiles;
    Bitmap* bmpDynamicTiles;
    Bitmap* bmpBorders;
    Bitmap* bmpFont;

    TilemapPack* baseLevels;

    Stage* stage;

    bool backgroundDrawn;
    bool pauseDrawn;
    bool paused;

    u16 stageIndex;

    i16 waitTimer;
    bool victory;
    bool waitDrawn;

} Game;

static Game* game = NULL;


static void next_level(Window* window) {

    game->backgroundDrawn = false;

    ++ game->stageIndex;
    stage_init_tilemap(game->stage, 
        tilemap_pack_get_tilemap(game->baseLevels, game->stageIndex),
        false);

    game->victory = false;
    game->waitTimer = WAIT_TIME;
    game->waitDrawn = false;
}


static i16 update_game(Window* window, i16 step) {

    AudioSystem* audio = window_get_audio_system(window);

    if (game->waitTimer > 0) {

        game->waitTimer -= step;
        if (game->waitTimer <= 0) {

            if (game->victory) {
                
                window_start_transition(window, true, 2, next_level);
            }
            else {

                game->backgroundDrawn = false;
                stage_force_redraw(game->stage);
            }
        }
        return 0;
    }

    if (keyboard_get_normal_key(KEY_RETURN) == STATE_PRESSED) {

        audio_play_predefined_sample(audio, SAMPLE_PAUSE);

        game->paused = !game->paused;
        if (game->paused) {

            game->pauseDrawn = false;
        }
        else {

            stage_force_redraw(game->stage);
            game->backgroundDrawn = false;
        }
    }
    if (game->paused) return 0;

    if (stage_update(game->stage, step)) {

        game->victory = true;
        game->waitTimer = WAIT_TIME;
        game->waitDrawn = false;
        return 0;
    }

    if (keyboard_get_normal_key(KEY_R) == STATE_PRESSED) {

        stage_reset(game->stage);
    }

    return 0;
}


static void draw_background(Canvas* canvas) {

    i16 w, h;
    i16 loopx, loopy;
    i16 top, left;
    i16 i;

    stage_get_size(game->stage, &w, &h);

    top = 100 - h*10;
    left = 160 - w*12;

    loopx = w*2;
    loopy = h*2;

    canvas_toggle_clipping(canvas, false);
    canvas_clear(canvas, 181);
        
    for (i = 0; i < loopx; ++ i) {

        canvas_draw_bitmap_region_fast(canvas, game->bmpBorders,
            12, 0, 12, 10,
            left + i*12, top-10);

        canvas_draw_bitmap_region_fast(canvas, game->bmpBorders,
            12, 20, 12, 10,
            left + i*12, top + h*20);
    }

    for (i = 0; i < loopy; ++ i) {

        canvas_draw_bitmap_region_fast(canvas, game->bmpBorders,
            0, 10, 12, 10,
            left - 12, top + i*10);

        canvas_draw_bitmap_region_fast(canvas, game->bmpBorders,
            24, 10, 12, 10,
            left + w*24, top + i*10);
    }

    // Corners
    canvas_draw_bitmap_region_fast(canvas, game->bmpBorders,
            0, 0, 12, 10,
            left-12, top-10);
    canvas_draw_bitmap_region_fast(canvas, game->bmpBorders,
            24, 0, 12, 10,
            left + w*24, top-10);    
    canvas_draw_bitmap_region_fast(canvas, game->bmpBorders,
            0, 20, 12, 10,
            left-12, top + h*20);
    canvas_draw_bitmap_region_fast(canvas, game->bmpBorders,
            24, 20, 12, 10,
            left + w*24, top +h*20);  
}


static void draw_waiting_text(Canvas* canvas) {

    const BOX_OFFSET_X = 8;
    const BOX_OFFSET_Y = 6;

    u16 w, h;
    i16 boxW, boxH;
    i16 x, y;
    char buffer [32];

    canvas_get_size(canvas, &w, &h);
    canvas_toggle_clipping(canvas, false);

    if (game->victory) {

        snprintf(buffer, 32, "STAGE CLEAR");
    }
    else {

        snprintf(buffer, 32, "STAGE %u", game->stageIndex+1);
    }

    boxW = ((i16) strlen(buffer) + 1) * 8 + BOX_OFFSET_X*2;
    boxH = 8 + BOX_OFFSET_Y*2;

    x = w/2 - boxW/2;
    y = h/2 - boxH/2;

    canvas_fill_rect(canvas, x, y, boxW, boxH, 0);
    canvas_fill_rect(canvas, x+1, y+1, boxW-2, boxH-2, 255);
    canvas_fill_rect(canvas, x+2, y+2, boxW-4, boxH-4, 0);

    canvas_darken(canvas, 1);

    if (game->victory) {

        canvas_draw_text_fast(canvas, game->bmpFont,
            "STAGE CLEAR!", w/2, h/2-4, 0, 0, ALIGN_CENTER);
    }
    else {

        canvas_draw_text_fast(canvas, game->bmpFont,
            buffer, w/2, h/2-4, 0, 0, ALIGN_CENTER);
    }

    canvas_toggle_clipping(canvas, true);
}


static void redraw_game(Canvas* canvas) {

    if (game->paused) {

        if (!game->pauseDrawn) {

            canvas_darken(canvas, 2);
            game->pauseDrawn = true;
        }
        return;
    }

    if (!game->backgroundDrawn) {
        
        draw_background(canvas);
        game->backgroundDrawn = true;
    }

    stage_draw(game->stage, canvas, game->bmpStaticTiles, game->bmpDynamicTiles);

    if (game->waitTimer > 0) {

        if (!game->waitDrawn) {

            draw_waiting_text(canvas);
            game->waitDrawn = true;
        }
    }
}


i16 init_game_scene(Window* window) {

    game = (Game*) calloc(1, sizeof(Game));
    if (game == NULL) {

        m_memory_error();
        return 1;
    }

    printf("Loading...\n");

    if ((game->bmpStaticTiles = load_bitmap("STATIC.BIN")) == NULL ||
        (game->bmpDynamicTiles = load_bitmap("DYNAMIC.BIN")) == NULL ||
        (game->bmpFont = load_bitmap("FONT.BIN")) == NULL ||
        (game->bmpBorders = load_bitmap("BORDERS.BIN")) == NULL ||
        (game->baseLevels = load_tilemap_pack("LEVELS.BIN")) == NULL) {

        dispose_game_scene();
        return 1;
    }

    init_stage();
    game->stage = new_stage(10, 9);
    if (game->stage == NULL) {

        dispose_game_scene();
        return 1;
    }

    game->stageIndex = 0;
    stage_init_tilemap(game->stage, 
        tilemap_pack_get_tilemap(game->baseLevels, game->stageIndex),
        false);

    game->backgroundDrawn = false;

    game->victory = false;
    game->waitTimer = WAIT_TIME;
    game->waitDrawn = false;

    window_start_transition(window, false, 2, NULL);

    return 0;
}


void dispose_game_scene() {

    if (game == NULL) return;

    dispose_bitmap(game->bmpStaticTiles);
    dispose_bitmap(game->bmpDynamicTiles);
    dispose_bitmap(game->bmpBorders);
    dispose_bitmap(game->bmpFont);

    dispose_tilemap_pack(game->baseLevels);

    dispose_stage(game->stage);

    m_free(game);
}


void register_game_scene(Window* window) {

    window_register_callback_functions(window,
        update_game, redraw_game);
}
