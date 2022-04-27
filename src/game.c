#include "game.h"
#include "system.h"
#include "tilemap.h"
#include "stage.h"
#include "keyb.h"
#include "menu.h"
#include "title.h"
#include "story.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static const i16 WAIT_TIME = 70;

static const u8* BUTTON_NAMES[] = {

    "RESUME",
    "RESTART",
    "AUDIO: ON ",
    "QUIT GAME"
};

static const u8* YES_NO_NAMES[] = {

    "YES",
    "NO"
};


typedef struct {

    Bitmap* bmpStaticTiles;
    Bitmap* bmpDynamicTiles;
    Bitmap* bmpBorders;
    Bitmap* bmpFont;
    Bitmap* bmpFontYellow;

    TilemapPack* baseLevels;

    Stage* stage;

    Menu* pauseMenu;
    Menu* yesNoMenu;
    i16 quitPhase;
    bool yesNoDrawn;

    bool backgroundDrawn;
    bool bufferRedrawn;
    bool bufferCloned;
    bool paused;
    bool hintDrawn;

    u16 stageIndex;

    i16 waitTimer;
    bool victory;
    bool waitDrawn;

    AssetCache* assets;

} Game;

static Game* game = NULL;


static void go_to_title(Window* window) {

    if (init_title_screen_scene(window, game->assets) != 0) {

        window_terminate(window);
        return;
    }

    dispose_game_scene();
    register_title_screen_scene(window);
}


static void force_pause_redraw() {

    game->bufferRedrawn = false;
}


static i16 save_progress() {

    #define SAVE_PATH "SAVE.DAT"

    u8 out;

    FILE* f = fopen(SAVE_PATH, "wb");
    if (f == NULL) {

        m_throw_error("Failed to create a file in ", SAVE_PATH, NULL);
        return 1;
    }

    out = (u8) game->stageIndex;
    if (fwrite(&out, sizeof(u8), 1, f) != 1) {

        m_throw_error("Failed to write data to a file in ", SAVE_PATH, NULL);
        fclose(f);
        return 1;
    }

    fclose(f);

    return 0;

    #undef SAVE_PATH
}


static void yes_no_callback(Menu* menu, i16 button, Window* window) {

    // Yes
    if (button == 0) {

        if (game->quitPhase == 1) {

            ++ game->quitPhase;
            menu_activate(game->yesNoMenu, 0);

            game->yesNoDrawn = false;
        }
        else {

            if (save_progress() == 1) {

                window_terminate(window);
                return;
            }
            window_start_transition(window, true, 3, TRANSITION_DARKEN, go_to_title);
        }
    }
    // No
    else if (button == 1) {

        if (game->quitPhase == 1) {
            
            game->quitPhase = 0;
            menu_deactivate(game->yesNoMenu);
            menu_activate(game->pauseMenu, 3);

            game->hintDrawn = false;
        }
        else {

            window_start_transition(window, true, 3, TRANSITION_DARKEN, go_to_title);
            return;
        }
    }

    force_pause_redraw();
}


static void menu_callback(Menu* menu, i16 button, Window* window) {

    bool audioState;
    AudioSystem* audio;

    switch (button) {

    case 1:
        stage_reset(game->stage, true);

    // Fall through wheeeeee
    case 0:

        menu_deactivate(menu);
        stage_force_redraw(game->stage);
        game->backgroundDrawn = false;
        game->paused = false;
        break;

    case 2:

        audio = window_get_audio_system(window);
        audioState = audio_is_enabled(audio);

        audio_toggle(audio, !audioState);

        menu_change_button_text(menu, button, 
            !audioState ? "AUDIO: ON " : "AUDIO: OFF");

        break;

    case 3:

        game->quitPhase = 1;
        game->yesNoDrawn = false;

        force_pause_redraw();

        menu_activate(game->yesNoMenu, 1);
        menu_deactivate(game->pauseMenu);

        break;

    default:
        break;
    }
}


static bool is_final_stage() {

    return game->stageIndex == tilemap_pack_get_tilemap_count(game->baseLevels)-1;
}


static void next_level(Window* window) {

    if (is_final_stage()) {

        if (init_story_scene(window, game->assets, 1) != 0) {

            window_terminate(window);
            return;
        }

        dispose_game_scene();
        register_story_scene(window);
        return;
    }

    game->backgroundDrawn = false;

    ++ game->stageIndex;
    stage_init_tilemap(game->stage, 
        tilemap_pack_get_tilemap(game->baseLevels, game->stageIndex),
        false, is_final_stage());

    game->victory = false;
    game->waitTimer = WAIT_TIME;
    game->waitDrawn = false;
}


static void update_game(Window* window, i16 step) {

    AudioSystem* audio = window_get_audio_system(window);

    if (game->waitTimer > 0) {

        game->waitTimer -= step;
        if (game->waitTimer <= 0) {

            if (game->victory) {
                
                window_start_transition(window, true, 3, 
                    is_final_stage() ? TRANSITION_LIGHTEN : TRANSITION_DARKEN,
                    next_level);
            }
            else {

                game->backgroundDrawn = false;
                stage_force_redraw(game->stage);
            }
        }
        return;
    }

    if (!game->paused &&
        keyboard_get_normal_key(KEY_RETURN) == STATE_PRESSED) {

        audio_play_predefined_sample(audio, SAMPLE_PAUSE);

        game->paused = true;
        game->bufferCloned = false;
        game->quitPhase = 0;
        game->hintDrawn = false;
        
        menu_activate(game->pauseMenu, 0);
    }
    else if (game->paused) {

        if (game->quitPhase > 0) {

            menu_update(game->yesNoMenu, window);
        }
        else {

            menu_update(game->pauseMenu, window);
        }
        return;
    }

    if (stage_update(game->stage, audio, step)) {

        game->victory = true;
        game->waitTimer = WAIT_TIME;
        game->waitDrawn = false;
        return;
    }

    if (keyboard_get_normal_key(KEY_R) == STATE_PRESSED) {

        if (stage_reset(game->stage, false)) {

            audio_play_predefined_sample(audio, SAMPLE_SELECT);
        }
    }
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


static void draw_pause(Canvas* canvas) {

    const BOX_OFFSET_X = 6;
    const BOX_OFFSET_Y = 5;

    const u8* MESSAGES[] = {
        "Are you sure? ",
        "Would you like\nto save your\nprogress?"
    };

    i16 x, y;
    i16 dw, dh;
    u16 w, h;

    canvas_get_size(canvas, &w, &h);
    canvas_toggle_clipping(canvas, false);
    
    if (game->quitPhase == 0) {

        menu_draw(game->pauseMenu, canvas, 
            game->bmpFont, game->bmpFontYellow, 
            0, 0, 0, 2);

        if (!game->hintDrawn) {

            canvas_draw_text(canvas, game->bmpFont,
                "HINT: Press Backspace\nor Z to undo a move.",
                80, h-20, 0, 2, ALIGN_LEFT);
            game->hintDrawn = true;
        }
    }
    else {

        if (!game->yesNoDrawn) {

            game->yesNoDrawn = true;

            // TODO: Numeric constants aaaargh

            dw = 14*8; // Length of first message times 8
            dh = (game->quitPhase == 1 ? 1 : 3) * 8;

            x = (i16)(w / 2) - dw/2;
            y = (i16)(h / 2) - dh/2; 

            draw_box(canvas, x - BOX_OFFSET_X, 
                y - BOX_OFFSET_Y,
                dw + BOX_OFFSET_X*2,
                dh + BOX_OFFSET_Y*2);

            canvas_draw_text(canvas, game->bmpFont,
                MESSAGES[game->quitPhase-1],
                x, y, 0, 0, ALIGN_LEFT);
        }
        menu_draw(game->yesNoMenu, canvas, 
            game->bmpFont, game->bmpFontYellow, 
            0, 32, 0, 2);
    }

    canvas_toggle_clipping(canvas, true);
}


static void redraw_game(Canvas* canvas) {

    if (game->paused) {

        if (!game->bufferCloned) {

            canvas_store_to_buffer(canvas);

            game->bufferCloned = true;
            game->bufferRedrawn = true;
        }

        if (!game->bufferRedrawn) {

            canvas_draw_buffered_image(canvas);
            game->bufferRedrawn = true;
        }
        
        draw_pause(canvas);
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


i16 init_game_scene(Window* window, AssetCache* assets, u16 startIndex) {

    game = (Game*) calloc(1, sizeof(Game));
    if (game == NULL) {

        m_memory_error();
        return 1;
    }

    game->assets = assets;

    window_draw_loading_screen(window, 0);

    if ((game->bmpStaticTiles = load_bitmap("STATIC.BIN")) == NULL ||
        (game->bmpDynamicTiles = load_bitmap("DYNAMIC.BIN")) == NULL ||
        (game->bmpBorders = load_bitmap("BORDERS.BIN")) == NULL ||
        (game->baseLevels = load_tilemap_pack("LEVELS.BIN")) == NULL) {

        dispose_game_scene();
        return 1;
    }

    game->bmpFont = asset_cache_get_bitmap(assets, "font_white");
    game->bmpFontYellow = asset_cache_get_bitmap(assets, "font_yellow");

    init_stage();
    game->stage = new_stage(10, 9);
    if (game->stage == NULL) {

        dispose_game_scene();
        return 1;
    }

    game->stageIndex = (u16) startIndex;
    stage_init_tilemap(game->stage, 
        tilemap_pack_get_tilemap(game->baseLevels, game->stageIndex),
        false, is_final_stage());

    game->pauseMenu = new_menu(BUTTON_NAMES, 4, menu_callback);
    if (game->pauseMenu == NULL) {

        return 1;
    } 

    game->yesNoMenu = new_menu(YES_NO_NAMES, 2, yes_no_callback);
    if (game->yesNoMenu == NULL) {

        return 1;
    }

    game->backgroundDrawn = false;

    game->victory = false;
    game->waitTimer = WAIT_TIME;
    game->waitDrawn = false;
    game->quitPhase = 0;
    game->yesNoDrawn = false;
    game->bufferCloned = false;
    game->hintDrawn = false;

    // window_start_transition(window, false, 2, NULL);

    return 0;
}


void dispose_game_scene() {

    if (game == NULL) return;

    dispose_bitmap(game->bmpStaticTiles);
    dispose_bitmap(game->bmpDynamicTiles);
    dispose_bitmap(game->bmpBorders);

    dispose_tilemap_pack(game->baseLevels);

    dispose_stage(game->stage);

    m_free(game);
}


void register_game_scene(Window* window) {

    window_register_callback_functions(window,
        update_game, redraw_game);
}
