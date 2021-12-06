#include "game.h"
#include "system.h"

#include <stdlib.h>
#include <stdio.h>


typedef struct {

    Bitmap* bmpParrot;
    Bitmap* bmpTest;

} Game;

static Game* game = NULL;


static i16 update_game(i16 step) {

    return 0;
}


static void redraw_game(Canvas* canvas) {

    canvas_clear(canvas, 182);

    canvas_draw_bitmap_fast(canvas, game->bmpParrot, 16, 16);

    canvas_draw_bitmap_region(canvas, game->bmpTest, 16, 16, 32, 32,
        32, 32, true);
}


i16 init_game_scene() {

    game = (Game*) calloc(1, sizeof(Game));
    if (game == NULL) {

        m_memory_error();
        return 1;
    }


    printf("Loading...\n");

    if ((game->bmpParrot = load_bitmap("PARROT.BIN")) == NULL ||
        (game->bmpTest = load_bitmap("TEST.BIN")) == NULL) {

        dispose_game_scene();
        return 1;
    }

    return 0;
}


void dispose_game_scene() {

    if (game == NULL) return;

    dispose_bitmap(game->bmpParrot);
    dispose_bitmap(game->bmpTest);
    m_free(game);
}


void register_game_scene(Window* window) {

    window_register_callback_functions(window,
        update_game, redraw_game);
}
