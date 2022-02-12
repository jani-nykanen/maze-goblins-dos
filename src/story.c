#include "story.h"
#include "system.h"
#include "keyb.h"
#include "game.h"
#include "title.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {

    AssetCache* assets;

    Bitmap* bmpFont;
    Bitmap* bmpBanner;

    const char** messages;
    const char* activeMessage;
    i16 phase;
    i16 page;
    u16 msgLength;
    i16 charIndex;
    i16 charTimer;

    bool drawNextCharacter;
    bool bannerDrawn;
    bool drawWholeMessage;

} StoryScene;

static StoryScene* story = NULL;


static const char* STORY_BEGINNING[] = {

"You are a space traveller from a\n"
"far away planet. You are lost in\n"
"a mysterious space maze filled\n"
"with monsters. They seem harmless,\n"
"but you decide to kill them\n"
"anyway."
,
"After all, the monsters possess\n"
"the power stars you can use to\n"
"get back home."
};


static const char* STORY_ENDING[] = {

"Congratulations, you have\n"
"collected enough power stars to\n"
"return home. Too bad you had to\n"
"dozens of innocent space monsters\n"
"for that."
,
"I hope you are proud of yourself\n"
"now."
};


static void go_to_game_callback(Window* window) {

    if (story->phase == 0) {

        if (init_game_scene(window, story->assets, 0) == 1) {

            window_terminate(window);
            return;
        }

        dispose_story_scene();
        register_game_scene(window);
    }
    else {

        if (init_title_screen_scene(window, story->assets) == 1) {

            window_terminate(window);
            return;
        }

        dispose_story_scene();
        register_title_screen_scene(window);
    }
}


static void update_story(Window* window, i16 step) {

    const i16 CHAR_TIME = 4;

    if (story->charIndex >= story->msgLength) {

        if (keyboard_any_pressed()) {

            audio_play_predefined_sample(window_get_audio_system(window), SAMPLE_INTRO_BEEP);

            ++ story->page;
            
            if (story->page == 1) {

                story->activeMessage = story->messages[story->page];
                story->msgLength = (u16) strlen(story->activeMessage);

                story->bannerDrawn = false;
                story->drawNextCharacter = true;

                story->charIndex = 0;
                story->charTimer = 0;
            }
            else {

                window_start_transition(window, true, 2, go_to_game_callback);
                return;
            }
        }
    }
    else {

        if (keyboard_any_pressed()) {

            story->drawWholeMessage = true;
            story->drawNextCharacter = false;

            story->charIndex = story->msgLength;

            audio_play_predefined_sample(window_get_audio_system(window), SAMPLE_CHOOSE);
        }
        else if ((story->charTimer += step) >= CHAR_TIME) {

            story->charTimer -= CHAR_TIME;
            ++ story->charIndex;

            story->drawNextCharacter =  true;

            while (story->charIndex < story->msgLength &&
                (story->activeMessage[story->charIndex] == ' ' ||
                 story->activeMessage[story->charIndex] == '\n')) {

                ++ story->charIndex;
            } 
        }
    }
}


static void draw_story(Canvas* canvas) {

    const i16 STORY_Y = 128;

    u16 w, h;
    u16 bw, bh;

    canvas_get_size(canvas, &w, &h);

    if (!story->bannerDrawn) {

        canvas_clear(canvas, 0);

        bitmap_get_size(story->bmpBanner, &bw, &bh);
        canvas_draw_bitmap_fast(canvas, story->bmpBanner,
            (i16)(w/2 - bw/2), 8);

        story->bannerDrawn = true;
    }
 
    if (story->drawWholeMessage) {

        canvas_draw_text_fast(canvas, story->bmpFont,
            story->activeMessage, 16, STORY_Y, 0, 2, ALIGN_LEFT);

        story->drawWholeMessage = false;
    }
    else if (story->drawNextCharacter) {

        canvas_draw_substr_fast(canvas, story->bmpFont,
            story->activeMessage, story->charIndex, story->charIndex+1,
            16, STORY_Y, 0, 2, ALIGN_LEFT);

        story->drawNextCharacter = false;
    }

    if (story->charIndex >= story->msgLength) {

        canvas_draw_bitmap_region_fast(canvas, 
            story->bmpFont, 8, 0, 8, 8,
            w-16, h-16);
    }
}


i16 init_story_scene(Window* window, AssetCache* assets, bool isEnding) {

    story = (StoryScene*) calloc(1, sizeof(StoryScene));
    if (story == NULL) {

        m_memory_error();
        return 1;
    }

    story->assets = assets;

    window_draw_loading_screen(window);

    if ((story->bmpBanner = load_bitmap(isEnding ? "INTRO2.BIN" : "INTRO1.BIN")) == NULL) {

        dispose_story_scene();
        return 1;
    }

    story->bmpFont = asset_cache_get_bitmap(assets, "font_white");

    story->page = 0;
    story->phase = (i16) isEnding;
    story->charIndex = -1;
    story->charTimer = 0;
    
    story->messages = isEnding ? STORY_ENDING : STORY_BEGINNING;
    story->activeMessage = story->messages[0];
    story->msgLength = (u16) strlen(story->activeMessage);

    story->drawNextCharacter = false;
    story->bannerDrawn = false;
    story->drawWholeMessage = false;

    return 0;
}


void dispose_story_scene() {

    if (story == NULL)
        return;

    dispose_bitmap(story->bmpBanner);
    m_free(story);
}


void register_story_scene(Window* window) {
    
    window_register_callback_functions(window,
        update_story, 
        draw_story);
}
