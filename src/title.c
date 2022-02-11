#include "title.h"
#include "system.h"
#include "menu.h"
#include "game.h"
#include "mathext.h"
#include "keyb.h"

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
    Bitmap* bmpLogo;

    Menu* titleMenu;

    AssetCache* assets;

    bool backgroundDrawn;
    bool load;

    i16 logoWave;

    bool enterPressed;
    i16 enterTimer;

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

    const i16 WAVE_SPEED = 2;

    if (title->enterPressed) {
    
        menu_update(title->titleMenu, window);
    }
    else {

        title->enterTimer = (title->enterTimer += step) % 60;

        if (keyboard_get_normal_key(KEY_RETURN) == STATE_PRESSED) {

            title->enterPressed = true;
            title->backgroundDrawn = false;

            audio_play_predefined_sample(
                window_get_audio_system(window), 
                SAMPLE_START_BEEP);
        }
    }

    title->logoWave = (title->logoWave + WAVE_SPEED*step) % 360;
}


static void draw_logo(Canvas* canvas) {

    const i16 AMPLITUDE = 6;

    i16 i;
    u16 bw, bh;
    i16 x, y;
    i16 shift;
    i16 waveStep;
    i16 wave;

    bitmap_get_size(title->bmpLogo, &bw, &bh);

    waveStep = (bh << 6) / 24;

    x = 160 - (i16) (bw/2);
    y = 20;

    for (i = 0; i < (i16) bh; ++ i) {

        wave = waveStep*i;
        wave >>= 6;
        wave += title->logoWave;

        shift = fixed_sin(wave) * AMPLITUDE;
        shift >>= get_fixed_trig_precision();

        canvas_draw_bitmap_region_fast(canvas, title->bmpLogo,
            0, i, bw, 1,
            160-80 + shift, 20 + i);
    }

    // canvas_draw_bitmap_fast(canvas, title->bmpLogo, 160-80, 20);
}


static void draw_title_screen(Canvas* canvas) {

    const u8* ENTER_TEXT = "PRESS ENTER TO START";

    i16 len = (i16) strlen(ENTER_TEXT);

    if (!title->backgroundDrawn) {

        canvas_clear(canvas, 0);

        canvas_draw_text_fast(canvas, title->bmpFontYellow,
            "(c)2022 Jani Nyk@nen", 160, 200-10, 0, 0, ALIGN_CENTER);

        title->backgroundDrawn = true;
    }

    canvas_toggle_clipping(canvas, false);

    draw_logo(canvas);

    if (title->enterPressed) {

        menu_draw(title->titleMenu, canvas,
        title->bmpFont,
        title->bmpFontYellow,
        0, 48, 0, 2);
    }
    else {

        if (title->enterTimer < 30) {

            canvas_draw_text_fast(canvas, title->bmpFont,
                ENTER_TEXT, 160, 144, 0, 0, ALIGN_CENTER);
        }
        else {

            canvas_fill_rect(canvas, 160 - len*4, 144, len*8, 8, 0);
        }
    }

    canvas_toggle_clipping(canvas, true);
}   


i16 init_title_screen_scene(Window* window, AssetCache* assets) {

    title = (TitleScreen*) calloc(1, sizeof(TitleScreen));
    if (title == NULL) {

        m_memory_error();
        return 1;
    }

    title->assets = assets;

    if ((title->bmpLogo = load_bitmap("LOGO.BIN")) == NULL) {

        dispose_title_screen_scene();
        return 1;
    }

    title->bmpFont = asset_cache_get_bitmap(assets, "font_white");
    title->bmpFontYellow = asset_cache_get_bitmap(assets, "font_yellow");

    title->backgroundDrawn = false;
    title->load = false;

    title->logoWave = 0;

    title->enterPressed = false;
    title->enterTimer = 59;

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

    dispose_bitmap(title->bmpLogo);

    dispose_menu(title->titleMenu);
    m_free(title);
}   


void register_title_screen_scene(Window* window) {

    window_register_callback_functions(window,
        update_title_screen,
        draw_title_screen);
}
