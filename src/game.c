#include "game.h"
#include "system.h"
#include "tilemap.h"
#include "stage.h"

#include <stdlib.h>
#include <stdio.h>


typedef struct {

    Bitmap* bmpStaticTiles;
    Bitmap* bmpFont;

    TilemapPack* baseLevels;

    Stage* stage;

    bool backgroundDrawn;

} Game;

static Game* game = NULL;


static i16 update_game(i16 step) {

    return 0;
}


static void redraw_game(Canvas* canvas) {

    if (!game->backgroundDrawn) {

        canvas_clear(canvas, 182);
        game->backgroundDrawn = true;

        canvas_toggle_clipping(canvas, false);
    }

    stage_draw(game->stage, canvas, game->bmpStaticTiles, NULL);
}


i16 init_game_scene() {

    game = (Game*) calloc(1, sizeof(Game));
    if (game == NULL) {

        m_memory_error();
        return 1;
    }


    printf("Loading...\n");

    if ((game->bmpStaticTiles = load_bitmap("STATIC.BIN")) == NULL ||
        (game->bmpFont = load_bitmap("FONT.BIN")) == NULL ||
        (game->baseLevels = load_tilemap_pack("LEVELS.BIN")) == NULL) {

        dispose_game_scene();
        return 1;
    }

    game->stage = new_stage(10, 9);
    if (game->stage == NULL) {

        dispose_game_scene();
        return 1;
    }

    stage_init_tilemap(game->stage, 
        tilemap_pack_get_tilemap(game->baseLevels, 0));

    game->backgroundDrawn = false;

    return 0;
}


void dispose_game_scene() {

    if (game == NULL) return;

    dispose_bitmap(game->bmpStaticTiles);
    dispose_bitmap(game->bmpFont);

    dispose_tilemap_pack(game->baseLevels);

    dispose_stage(game->stage);

    m_free(game);
}


void register_game_scene(Window* window) {

    window_register_callback_functions(window,
        update_game, redraw_game);
}
