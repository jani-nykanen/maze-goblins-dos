#include "title.h"
#include "system.h"
#include "menu.h"
#include "game.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static const u8* BUTTON_NAMES[] = {

    "NEW GAME",
    "LOAD GAME",
    "AUDIO: ON ",
    "QUIT GAME"
};


typedef struct {

    Bitmap* bmpFont;
    Bitmap* bmpFontYellow;

    Menu* titleMenu;

    AssetCache* assets;

    bool backgroundDrawn;
    bool load;

} TitleScreen;

static TitleScreen* title = NULL;


static i16 load_data(const str path){

    u8 b;
    FILE* f = fopen(path, "rb");
    if (f == NULL) {

        return -1;
    }

    fread(&b, 1, 1, f);
    fclose(f);

    return (i16) b;
}


static void start_game_callback(Window* window) {

    i16 index = -1;
    u16 startIndex = 0; // TEMP, normally go to intro if index < 0

    if (title->load) {

        index = load_data("SAVE.DAT");
        if (index >= 0) {

            startIndex = (u16) index;
        }
    }

    if (init_game_scene(window, title->assets, startIndex) != 0) {

        window_terminate(window);
        return;
    }

    dispose_title_screen_scene();
    register_game_scene(window);
}


static void terminate_callback(Window* window) {

    window_terminate(window);
}


static void menu_callback(Menu* menu, i16 button, Window* window) {

    bool audioState;
    AudioSystem* audio;

    title->load = false;

    switch (button) {

    // Load game
    case 1:
        title->load = true;
    // New game
    case 0:

        window_start_transition(window, true, 2, start_game_callback);
        break;

    // Toggle audio
    case 2:

        audio = window_get_audio_system(window);
        audioState = audio_is_enabled(audio);

        audio_toggle(audio, !audioState);

        menu_change_button_text(menu, button, 
            !audioState ? "AUDIO: ON " : "AUDIO: OFF");

        break;

    // Quit
    case 3:

        window_start_transition(window, true, 2, terminate_callback);
        break;

    default:
        break;
    }
}


static void update_title_screen(Window* window, i16 step) {

    menu_update(title->titleMenu, window);
}


static void draw_title_screen(Canvas* canvas) {

    if (!title->backgroundDrawn) {

        canvas_clear(canvas, 0);
        title->backgroundDrawn = true;
    }

    canvas_toggle_clipping(canvas, false);

    menu_draw(title->titleMenu, canvas,
        title->bmpFont,
        title->bmpFontYellow,
        0, 32, 0, 2);

    canvas_toggle_clipping(canvas, true);
}   


i16 init_title_screen_scene(Window* window, AssetCache* assets) {

    title = (TitleScreen*) calloc(1, sizeof(TitleScreen));
    if (title == NULL) {

        m_memory_error();
        return 1;
    }

    title->assets = assets;

    title->bmpFont = asset_cache_get_bitmap(assets, "font_white");
    title->bmpFontYellow = asset_cache_get_bitmap(assets, "font_yellow");

    title->backgroundDrawn = false;
    title->load = false;

    title->titleMenu = new_menu(BUTTON_NAMES, 4, menu_callback);
    if (title->titleMenu == NULL) {

        dispose_title_screen_scene();
        return 1;
    }
    menu_activate(title->titleMenu, 0);

    return 0;
}


void dispose_title_screen_scene() {

    if (title == NULL)
        return;

    dispose_menu(title->titleMenu);
    m_free(title);
}   


void register_title_screen_scene(Window* window) {

    window_register_callback_functions(window,
        update_title_screen,
        draw_title_screen);
}
