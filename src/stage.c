#include "stage.h"
#include "system.h"
#include "mathext.h"
#include "keyb.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// This need to be divisible by both 24 and 20!
static const i16 ANIMATION_TIME = 240;
static const i16 DIR_X[] = {1, 0, -1, 0};
static const i16 DIR_Y[] = {0, -1, 0, 1};


typedef enum {

    ACTION_NONE = 0,

    ACTION_RIGHT = 1,
    ACTION_UP = 2,
    ACTION_LEFT = 3,
    ACTION_DOWN = 4

} Action;


typedef struct {

    u16 maxWidth;
    u16 maxHeight;

    u16 width;
    u16 height;

    u8* bottomLayer;
    u8* topLayer;
    u8* redrawBuffer;

    u8** bottomLayerBuffer;
    u8** topLayerBuffer;
    u16 bufferSize;
    u16 bufferPointer;
    u16 undoCount;

    i16 animationTimer;

    Point_i16 animDir;

} _Stage;


static u8** allocate_u8_array_buffer(u16 size, u16 count) {

    u16 i, j;

    u8** out = (u8**) calloc(count, sizeof(u8*));
    if (out == NULL) {

        m_memory_error();
        return NULL;
    }

    for (i = 0; i < count; ++ i) {

        out[i] = (u8*) calloc(size, sizeof(u8));
        if (out[i] == NULL) {

            for (j = 0; j < i; ++ j) {

                m_free(out[j]);
            }
            m_free(out);

            m_memory_error();
            return NULL;
        }
    }

    return out;
}


static void filter_array(u8* arr, const u8* filter, u16 arrLen, u16 filterLen) {

    i16 i;
    i16 j;

    for (i = 0; i < arrLen; ++ i) {

        for (j = 0; j < filterLen; ++ j) {

            if (arr[i] == filter[j]) {

                arr[i] = 0;
                break;
            }
        }
    }
}


static void draw_static_layer(_Stage* stage, Canvas* canvas, Bitmap* staticTiles,
    i16 left, i16 top) {

    i16 x, y;
    i16 i = 0;

    i16 dx, dy;

    for (y = 0; y < stage->height; ++ y) {

        for (x = 0; x < stage->width; ++ x) {

            i = y * stage->width + x;

            if (!stage->redrawBuffer[i]) 
                continue;

            dx = left + x*24;
            dy = top + y*20;

            switch (stage->bottomLayer[i]) {

            // Floor
            case 1:

                canvas_draw_bitmap_region_fast(canvas, staticTiles, 0, 0, 24, 20, dx, dy);
                break;

            // Pure darkness
            default:

                canvas_fill_rect(canvas, dx, dy, 24, 20, 0);
                break;
            }

            // We set this to zero in the dynamic layer operation
            // stage->redrawBuffer[i] = 0;
        }

        if (y >= stage->height)
            break;
    }
}


static void draw_dynamic_layer(_Stage* stage, 
    Canvas* canvas, Bitmap* dynamicTiles,
    i16 left, i16 top) {

    i16 x, y;
    i16 i = 0;

    i16 dx, dy;

    i16 shiftx = 0;
    i16 shifty = 0;

    if (stage->animationTimer > 0) {

        shiftx = -stage->animationTimer/10 * stage->animDir.x;
        shifty = -stage->animationTimer/12 * stage->animDir.y;
    }

    for (y = 0; y < stage->height; ++ y) {

        for (x = 0; x < stage->width; ++ x) {

            i = y * stage->width + x;

            if (!stage->redrawBuffer[i]) 
                continue;

            dx = left + x*24 + shiftx;
            dy = top + y*20 + shifty;

            switch (stage->topLayer[i]) {

            // Player
            case 2:
                canvas_draw_bitmap_region(canvas, dynamicTiles, 
                    0, 0, 24, 20, dx, dy, false);
                break;

            // Imps
            case 4:
            case 5:
            case 6:

                canvas_draw_bitmap_region(canvas, dynamicTiles, 
                    0, 20, 24, 20, dx, dy, false);
                break;

            default:
                break;
            }

            if (stage->animationTimer <= 0)
                stage->redrawBuffer[i] = 0;
        }

        if (y >= stage->height)
            break;
    }
}


static bool is_player_in_direction(_Stage* stage, i16 dx, i16 dy, i16 dirx, i16 diry) {

    u8 top;
    u8 bottom;
    i16 i;

    do {

        i = dy*stage->width+dx;

        bottom = stage->bottomLayerBuffer[stage->bufferPointer][i];
        if (bottom > 0 && bottom != 9 && bottom != 11)
            break;

        top = stage->topLayerBuffer[stage->bufferPointer][i];
        if (top == 0)
            break;

        if (top == 2)
            return true;

        dx += dirx;
        dy += diry;
    }
    while (dx >= 0 && dy >= 0 && dx < stage->width && dy < stage->height);

    return false;
}


static bool is_free_tile_in_direction(_Stage* stage, i16 dx, i16 dy, i16 dirx, i16 diry) {

    u8 top;
    u8 bottom;
    i16 i;

    do {

        i = dy*stage->width+dx;

        top = stage->topLayerBuffer[stage->bufferPointer][i];
        bottom = stage->bottomLayerBuffer[stage->bufferPointer][i];

        if (bottom == 1 && bottom != 9 && bottom != 11)
            return false;

        if (top == 0 && (bottom == 0 || bottom != 9 || bottom != 11))
            return true;

        dx += dirx;
        dy += diry;
    }
    while (dx >= 0 && dy >= 0 && dx < stage->width && dy < stage->height);

    return false;
}


static bool check_movement(_Stage* stage, Action a) {

    i16 x, y;

    i16 dirx = DIR_X[a-1];
    i16 diry = DIR_Y[a-1];

    i16 currentIndex;
    i16 targetIndex;

    u8 id;

    bool ret = false;

    for (y = 0; y < stage->height; ++ y) {

        for (x = 0; x < stage->width; ++ x) {

            currentIndex = y*stage->width + x;

            id = stage->topLayerBuffer[stage->bufferPointer][currentIndex];
            if (id < 2 || id > 7) continue;

            if (is_player_in_direction(stage, x, y, -dirx, -diry)) {

                if (is_free_tile_in_direction(stage, x + dirx, y + diry, dirx, diry)) {

                    targetIndex = (y + diry)*stage->width + x + dirx;

                    stage->redrawBuffer[currentIndex] = true;
                    stage->redrawBuffer[targetIndex] = true;

                    stage->topLayer[targetIndex] = id;
                    if (id == 2)
                        stage->topLayer[currentIndex] = 0;

                    ret = true;
                }
            }
        }   
    }
    return ret;
}


static void control(_Stage* stage) {

    Action a = ACTION_NONE;

    if (keyboard_get_extended_key(KEY_RIGHT) & STATE_DOWN_OR_PRESSED) {

        a = ACTION_RIGHT;
    }
    else if (keyboard_get_extended_key(KEY_UP) & STATE_DOWN_OR_PRESSED) {

        a = ACTION_UP;
    }
    else if (keyboard_get_extended_key(KEY_LEFT) & STATE_DOWN_OR_PRESSED) {

        a = ACTION_LEFT;
    }
    else if (keyboard_get_extended_key(KEY_DOWN) & STATE_DOWN_OR_PRESSED) {

        a = ACTION_DOWN;
    }

    if (a == ACTION_NONE) return;

    if (check_movement(stage, a)) {

        stage->animationTimer = ANIMATION_TIME;

        stage->animDir.x = DIR_X[a-1];
        stage->animDir.y = DIR_Y[a-1];
    }
}


Stage* new_stage(u16 maxWidth, u16 maxHeight) {

    const BUFFER_MAX_COUNT = 8;

    _Stage* stage = (_Stage*) calloc(1, sizeof(_Stage));
    if (stage == NULL) {

        m_memory_error();
        return NULL;
    }

    stage->maxWidth = maxWidth;
    stage->maxHeight = maxHeight;

    stage->redrawBuffer = (u8*) calloc(maxWidth*maxHeight, sizeof(u8));
    if (stage->redrawBuffer == NULL) {

        m_memory_error();

        dispose_stage((Stage*) stage);
        return NULL;
    }

    stage->bottomLayer = (u8*) calloc(maxWidth*maxHeight, sizeof(u8));
    if (stage->bottomLayer == NULL) {

        m_memory_error();

        dispose_stage((Stage*) stage);
        return NULL;
    }

    stage->topLayer = (u8*) calloc(maxWidth*maxHeight, sizeof(u8));
    if (stage->topLayer == NULL) {

        m_memory_error();

        dispose_stage((Stage*) stage);
        return NULL;
    }

    stage->bottomLayerBuffer = allocate_u8_array_buffer(maxWidth*maxHeight, BUFFER_MAX_COUNT);
    if (stage->bottomLayerBuffer == NULL) {

        dispose_stage((Stage*) stage);
        return NULL;
    }

    stage->topLayerBuffer = allocate_u8_array_buffer(maxWidth*maxHeight, BUFFER_MAX_COUNT);
    if (stage->topLayerBuffer == NULL) {

        dispose_stage((Stage*) stage);
        return NULL;
    }

    stage->bufferPointer = 0;
    stage->bufferSize = BUFFER_MAX_COUNT;
    stage->undoCount = 0;

    stage->animationTimer = 0;

    return (Stage*) stage;
}


void dispose_stage(Stage* _stage) {

    i16 i;

    _Stage* stage = (_Stage*) _stage;

    if (stage == NULL) return;

    for (i = 0; i < stage->bufferSize; ++ i) {
        
        m_free(stage->bottomLayerBuffer[i]);
        m_free(stage->topLayerBuffer[i]);
    }
    m_free(stage->bottomLayerBuffer);
    m_free(stage->topLayerBuffer);

    m_free(stage->redrawBuffer);
    m_free(stage->bottomLayer);
    m_free(stage);
}


void stage_init_tilemap(Stage* _stage, Tilemap* tilemap) {

    const u8 BOTTOM_FILTER[] = {2, 3, 4, 5, 6, 7};
    const u8 TOP_FILTER[] = {1, 8, 9, 10, 11};

    _Stage* stage = (_Stage*) _stage;

    u16 w;
    u16 h;

    tilemap_get_size(tilemap, &w, &h);
    tilemap_copy(tilemap, stage->bottomLayer);
    filter_array(stage->bottomLayer, BOTTOM_FILTER, 
        stage->maxWidth*stage->maxHeight, (u16) strlen(BOTTOM_FILTER));

    tilemap_copy(tilemap, stage->topLayer);
    filter_array(stage->topLayer, TOP_FILTER, 
        stage->maxWidth*stage->maxHeight, (u16) strlen(TOP_FILTER));

    stage->width = min_u16(stage->maxWidth, w);
    stage->height = min_u16(stage->maxHeight, h);

    stage->bufferPointer = 0;
    stage->undoCount = 0;
    
    memcpy(stage->bottomLayerBuffer[0], stage->bottomLayer, stage->width*stage->height);
    memcpy(stage->topLayerBuffer[0], stage->topLayer, stage->width*stage->height);

    memset(stage->redrawBuffer, 1, stage->width*stage->height);
}


void stage_update(Stage* _stage, i16 step) {

    const i16 ANIM_SPEED = 24;

    _Stage* stage = (_Stage*) _stage;

    if (stage->animationTimer > 0) {

        stage->animationTimer -= ANIM_SPEED * step;
        if (stage->animationTimer <= 0) {

            stage->bufferPointer = (stage->bufferPointer + 1) % (stage->bufferSize);
            stage->undoCount = min_i16(stage->bufferSize-1, stage->undoCount+1);

            memcpy(stage->bottomLayerBuffer[stage->bufferPointer], 
                stage->bottomLayer, stage->width*stage->height);
            memcpy(stage->topLayerBuffer[stage->bufferPointer], 
                stage->topLayer, stage->width*stage->height);
        }

        return;
    }

    control(stage);
}


void stage_draw(Stage* _stage, Canvas* canvas, 
    Bitmap* staticTiles, Bitmap* dynamicTiles) {

    _Stage* stage = (_Stage*) _stage;

    i16 left = 160 - stage->width*12;
    i16 top = 100 - stage->height*10;
    
    canvas_set_clip_area(canvas, left, top, stage->width*24, stage->height*20);

    canvas_toggle_clipping(canvas, false);
    draw_static_layer(stage, canvas, staticTiles, left, top);

    canvas_toggle_clipping(canvas, true);
    draw_dynamic_layer(stage, canvas, dynamicTiles, left, top);

    canvas_reset_clip_area(canvas);
}
