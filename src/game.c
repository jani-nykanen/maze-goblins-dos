#include "game.h"
#include "system.h"
#include "tilemap.h"
#include "stage.h"
#include "keyb.h"

#include <stdlib.h>
#include <stdio.h>


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

} Game;

static Game* game = NULL;


static void next_level(Window* window) {

    game->backgroundDrawn = false;

    ++ game->stageIndex;
    stage_init_tilemap(game->stage, 
        tilemap_pack_get_tilemap(game->baseLevels, game->stageIndex),
        false);
}


static i16 update_game(Window* window, i16 step) {

    if (keyboard_get_normal_key(KEY_RETURN) == STATE_PRESSED) {

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

        window_start_transition(window, true, 2, next_level);
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
}


i16 init_game_scene() {

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
