#include "stage.h"
#include "system.h"
#include "mathext.h"
#include "keyb.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


// This need to be divisible by both 24 and 20!
static const i16 ANIMATION_TIME = 240;
static const i16 DESTROY_TIME = 480;
static const i16 TOGGLE_TIME = 240;
static const i16 DIR_X[] = {1, 0, -1, 0};
static const i16 DIR_Y[] = {0, -1, 0, 1};

static const BUFFER_MAX_COUNT = 33;

static i16 FIBONACCI[90];


static void compute_fibonacci() {

    // Just find a suitable prime here to avoid
    // interger overflow
    const i16 MAX = 251;

    i16 i;
    FIBONACCI[0] = 1;
    FIBONACCI[1] = 1;

    for (i = 2; i < 90; ++ i) {

        FIBONACCI[i] = (FIBONACCI[i-2] + FIBONACCI[i-1]) % MAX;
    }
}


typedef enum {

    ACTION_NONE = 0,

    ACTION_RIGHT = 1,
    ACTION_UP = 2,
    ACTION_LEFT = 3,
    ACTION_DOWN = 4

} Action;


typedef enum {

    ANIMATION_MOVE = 0,
    ANIMATION_DESTROY = 1,
    ANIMATION_TOGGLE_WALLS = 2

} AnimationType;


typedef struct {

    i16 x;
    i16 y;
    i16 timer;
    i16 time;
    bool exist;

} Dust;


typedef struct {

    u16 maxWidth;
    u16 maxHeight;

    u16 width;
    u16 height;

    u8* bottomLayer;
    u8* topLayer;
    bool wallState;

    u8* redrawBuffer;
    u8* connectionBuffer;

    u8** bottomLayerBuffer;
    u8** topLayerBuffer;
    bool* wallStateBuffer;
    u16 bufferSize;
    u16 bufferPointer;
    u16 undoCount;

    i16 animationTimer;
    i16 animDir;
    bool nonPlayerMoved;
    AnimationType animType;

    Dust* dust;
    u16 dustCount;

    Tilemap* baseMap;

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


static void update_dust_particle(Dust* d, _Stage* stage, i16 step) {

    i16 i;

    if (!d->exist) return;

    i = d->y * stage->width + d->x;

    //if (stage->topLayer[i] == 0)
    stage->redrawBuffer[i] = true;

    if ((d->timer -= step) <= 0) {

        d->exist = false;
        return;
    }
}


static void draw_dust(Dust* d, Canvas* canvas, Bitmap* dynamicTiles, i16 left, i16 top) {

    i16 frame;

    if (!d->exist) return;

    frame = max_i16(0, 3 - (d->timer / (d->time/4)) );

    canvas_draw_bitmap_region(canvas, dynamicTiles, 
        frame*24, 120, 24, 20, 
        left + d->x*24,
        top + d->y*20, false);
}


static void draw_static_layer(_Stage* stage, Canvas* canvas, Bitmap* staticTiles,
    i16 left, i16 top) {

    i16 x, y;
    i16 i = 0;

    i16 dx, dy;
    i16 frame;

    u8 id;

    for (y = 0; y < stage->height; ++ y) {

        for (x = 0; x < stage->width; ++ x) {

            i = y * stage->width + x;

            if (!stage->redrawBuffer[i]) 
                continue;

            dx = left + x*24;
            dy = top + y*20;

            id = stage->bottomLayer[i];
            switch (id) {

            // Floor
            case 1:

                canvas_draw_bitmap_region_fast(canvas, staticTiles, 0, 0, 24, 20, dx, dy);
                break;

            // Star
            case 8:

                canvas_draw_bitmap_region_fast(canvas, staticTiles, 48, 0, 24, 20, dx, dy);
                break;

            // Walls, off
            case 9:

                canvas_draw_bitmap_region_fast(canvas, staticTiles, 0, 20, 24, 20, dx, dy);
                break;

            // Walls, on
            // & button
            case 10:
            case 11:

                canvas_draw_bitmap_region_fast(canvas, staticTiles, 24, 20 * (id-10), 24, 20, dx, dy);
                break;

            // Pure darkness
            default:

                frame = (FIBONACCI[i % 90]) % 4;
                if (frame == 0)
                    canvas_fill_rect(canvas, dx, dy, 24, 20, 0);
                else
                    canvas_draw_bitmap_region_fast(canvas, staticTiles, 
                        (frame-1)*24, 40, 24, 20, dx, dy);

                break;
            }

            // We set this to zero in the dynamic layer operation
            // stage->redrawBuffer[i] = 0;
        }

        if (y >= stage->height)
            break;
    }
}


static void draw_moving_object(_Stage* stage, 
    Canvas* canvas, Bitmap* dynamicTiles,
    i16 x, i16 y, i16 id, i16 dx, i16 dy) {
    
    static const i16 PLAYER_FRAME[] = {2, 1, 2, 0};
    static const i16 IMP_FRAME[] = {0, 1, 0, 2, 0};
    static const i16 FRAME_TIME = 120;

    i16 frame = 0;

    switch (id) {

    // Player
    case 2:

        frame = PLAYER_FRAME[stage->animDir];
        canvas_draw_bitmap_region(canvas, dynamicTiles, 
            frame*24, 0, 24, 20, dx, dy, stage->animDir == 2);
        break;

    // Rock
    case 3:
        canvas_draw_bitmap_region(canvas, dynamicTiles, 
            72, 0, 24, 20, dx, dy, false);
        break;

    // Imps
    case 4:
    case 5:
    case 6:
    case 7:

        if (stage->animType == ANIMATION_MOVE) {

            frame = stage->animationTimer / FRAME_TIME;
            if (x % 2 == y % 2)
                frame += 2;
        }
        else {

            frame = 0;
        }

        canvas_draw_bitmap_region(canvas, dynamicTiles, 
            IMP_FRAME[frame]*24, 20 + 20 * (id-4), 
            24, 20, dx, dy, false);
        break;

    // Dying imp
    case 131:
    case 132:
    case 133:
    case 134:

        frame = max_i16(0, 3 - stage->animationTimer / (DESTROY_TIME/4));
        canvas_draw_bitmap_region(canvas, dynamicTiles, 
            frame*24, 100, 24, 20, dx, dy, false);
        break;

    default:
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

    i16 id;

    if (stage->animationTimer > 0 && stage->animType == ANIMATION_MOVE) {

        shiftx = -round_i16(stage->animationTimer, 10) * DIR_X[stage->animDir];
        shifty = -round_i16(stage->animationTimer, 12) * DIR_Y[stage->animDir];
    }

    for (y = 0; y < stage->height; ++ y) {

        for (x = 0; x < stage->width; ++ x) {

            i = y * stage->width + x;

            if (!stage->redrawBuffer[i]) 
                continue;

            dx = left + x*24 + shiftx;
            dy = top + y*20 + shifty;

            id = stage->topLayer[i];
            draw_moving_object(stage, canvas, dynamicTiles, x, y, id, dx, dy);

            // "Looping objects"
            // Left
            if (stage->animDir == 2 && x == stage->width-1) {

                draw_moving_object(stage, canvas, dynamicTiles, 
                    0, y, id, 
                    left - 24 + shiftx, dy);
            }
            // Right
            else if (stage->animDir == 0 && x == 0) {

                draw_moving_object(stage, canvas, dynamicTiles, 
                    stage->width-1, y, id, 
                    left + stage->width*24 + shiftx, dy);
            }
            // Top
            else if (stage->animDir == 1 && y == stage->height-1) {

                draw_moving_object(stage, canvas, dynamicTiles, 
                    x, 0, id, 
                    dx, top - 24 + shifty);
            }
            // Bottom
            else if (stage->animDir == 3 && y == 0) {

                draw_moving_object(stage, canvas, dynamicTiles, 
                    x, stage->height-1, id, 
                    dx, top + stage->height*20 + shifty);
            }

            if (stage->animationTimer <= 0)
                stage->redrawBuffer[i] = 0;
        }

        if (y >= stage->height)
            break;
    }
}


static bool is_player_in_direction(_Stage* stage, i16 x, i16 y, i16 dirx, i16 diry) {

    u8 top;
    u8 bottom;
    i16 i;

    i16 dx = x;
    i16 dy = y;

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

        dx = neg_mod_i16(dx + dirx, stage->width);
        dy = neg_mod_i16(dy + diry, stage->height);
    }
    while (dx != x || dy != y);

    return false;
}


static bool is_free_tile_in_direction(_Stage* stage, 
    i16 x, i16 y, i16 dirx, i16 diry, bool isPlayer) {

    u8 top;
    u8 bottom;
    i16 i;

    i16 dx = x;
    i16 dy = y;

    do {

        i = dy*stage->width+dx;

        top = stage->topLayerBuffer[stage->bufferPointer][i];
        bottom = stage->bottomLayerBuffer[stage->bufferPointer][i];

        if (bottom > 0 && bottom != 9 && bottom != 11 && (!isPlayer || bottom != 8))
            return false;

        if (top == 0 && 
            (bottom == 0 || bottom == 9 || bottom == 11 || 
            (isPlayer && bottom == 8)))
            return true;

        dx = neg_mod_i16(dx + dirx, stage->width);
        dy = neg_mod_i16(dy + diry, stage->height);

        isPlayer = false;
    }
    while (dx != x || dy != y);

    return false;
}


static void spawn_dust_particle(_Stage* stage, i16 x, i16 y) {
    
    const DUST_TIME = 24;

    i16 i;

    for (i = 0; i < stage->dustCount; ++ i) {

        if (!stage->dust[i].exist) {

            stage->dust[i].x = x;
            stage->dust[i].y = y;
            stage->dust[i].time = DUST_TIME;
            stage->dust[i].timer = stage->dust[i].time;

            stage->dust[i].exist = true;

            break;
        }
    }
}


static bool check_movement(_Stage* stage, Action a) {

    i16 x, y;

    i16 dirx = DIR_X[a-1];
    i16 diry = DIR_Y[a-1];

    i16 currentIndex;
    i16 targetIndex;

    i16 dx, dy;

    u8 id;

    bool ret = false;

    stage->nonPlayerMoved = false;

    for (y = 0; y < stage->height; ++ y) {

        for (x = 0; x < stage->width; ++ x) {

            currentIndex = y*stage->width + x;

            id = stage->topLayerBuffer[stage->bufferPointer][currentIndex];
            if (id < 2 || id > 7) continue;

            dx = neg_mod_i16(x + dirx, (i16) stage->width);
            dy = neg_mod_i16(y + diry, (i16) stage->height);

            if (is_player_in_direction(stage, x, y, -dirx, -diry)) {

                if (is_free_tile_in_direction(stage, dx, dy, dirx, diry, id == 2)) {

                    targetIndex = dy*stage->width + dx;

                    stage->redrawBuffer[currentIndex] = true;
                    stage->redrawBuffer[targetIndex] = true;

                    stage->topLayer[targetIndex] = id;
                    if (id == 2) {

                        stage->topLayer[currentIndex] = 0;
                        spawn_dust_particle(stage, x, y);
                    }
                    else {

                        stage->nonPlayerMoved = true;
                    }

                    ret = true;
                }
            }
        }   
    }
    return ret;
}


static void undo(_Stage* stage) {

    if (stage->undoCount == 0) return;

    if (stage->bufferPointer == 0) {

        stage->bufferPointer = stage->bufferSize-1;
    }
    else {

        -- stage->bufferPointer;
    }

    memcpy(stage->bottomLayer, 
        stage->bottomLayerBuffer[stage->bufferPointer], 
        stage->width*stage->height);
    memcpy(stage->topLayer, 
        stage->topLayerBuffer[stage->bufferPointer], 
        stage->width*stage->height);

    stage->wallState = stage->wallStateBuffer[stage->bufferPointer];

    memset(stage->redrawBuffer, 1, stage->width*stage->height);

    -- stage->undoCount;
}


static void control(_Stage* stage) {

    Action a = ACTION_NONE;

    if (keyboard_get_normal_key(KEY_Z) == STATE_PRESSED ||
        keyboard_get_normal_key(KEY_BACKSPACE) == STATE_PRESSED) {

        undo(stage);
        return;
    }

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

        stage->animType = ANIMATION_MOVE;
        stage->animationTimer = ANIMATION_TIME;
        stage->animDir = a-1;
    }
}


static u8 get_upper_tile(_Stage* stage, i16 x, i16 y) {

    if (x < 0 || y < 0 || x >= stage->width || y >= stage->height)
        return 0;

    return stage->topLayer[y * stage->width + x];
}


static u8 get_connection_count(_Stage* stage, i16 x, i16 y) {

    if (x < 0 || y < 0 || x >= stage->width || y >= stage->height)
        return 0;

    return stage->connectionBuffer[y * stage->width + x];
}


static bool check_effects(_Stage* stage, bool nonPlayerMoved) {

    static const i16 MIN_CONNECTION = 2;

    i16 x, y;
    i16 dx, dy;
    u8 id;
    u8 tid;
    i16 i, k;

    bool destroy = false;
    bool buttonWithoutObject = false;

    memset(stage->connectionBuffer, 0, stage->width*stage->height);

    // Step 1: mark connections & check things under the player
    for (y = 0; y < stage->height; ++ y) {

        for (x = 0; x < stage->width; ++ x) {

            i = y*stage->width + x;
            id = stage->topLayer[i];

            if (stage->bottomLayer[i] == 11 && id == 0) {

                buttonWithoutObject = true;
            }

            if (id == 2) {

                if (stage->bottomLayer[i] == 8) {

                    stage->bottomLayer[i] = 0;
                }
                continue;
            }

            if (!nonPlayerMoved || id < 4 || id > 7)
                continue;

            for (k = 0; k < 4; ++ k) {

                dx = neg_mod_i16(x + DIR_X[k], (i16) stage->width);
                dy = neg_mod_i16(y + DIR_Y[k], (i16) stage->height);
                tid = get_upper_tile(stage, dx, dy);

                if (tid == id || (id == 7 && (tid >= 4 && tid <= 7)) || tid == 7) {

                    if ((++ stage->connectionBuffer[i]) >= MIN_CONNECTION) {

                        destroy = true;
                    }
                }
            }
        }
    }

    if (buttonWithoutObject != stage->wallState && !nonPlayerMoved)
        return ANIMATION_DESTROY;

    // Step 2: check things to destroy
    // (and walls to toggle/untoggle)
    for (y = 0; y < stage->height; ++ y) {

        for (x = 0; x < stage->width; ++ x) {

            i = y*stage->width + x;
            id = stage->topLayer[i];

            // Toggle walls
            if (buttonWithoutObject == stage->wallState) {

                if (stage->bottomLayer[i] == 9) {

                    stage->bottomLayer[i] = 10;
                    stage->redrawBuffer[i] = true;
                }
                else if (stage->bottomLayer[i] == 10) {

                    stage->bottomLayer[i] = 9;
                    stage->redrawBuffer[i] = true;
                }
            }

            if (!nonPlayerMoved)
                continue;

            if (stage->connectionBuffer[i] > 0) {

                if (stage->connectionBuffer[i] >= MIN_CONNECTION) {

                    stage->topLayer[i] += 127;
                    stage->redrawBuffer[i] = true;
                }
                else {

                    for (k = 0; k < 4; ++ k) {

                        dx = neg_mod_i16(x + DIR_X[k], (i16) stage->width);
                        dy = neg_mod_i16(y + DIR_Y[k], (i16) stage->height);

                        tid = get_upper_tile(stage, dx, dy);
                        if (tid >= 127)
                            tid -= 127;
                        
                        if ((tid == id || (id == 7 && (tid >= 4 && tid <= 7)) || tid == 7) &&
                            get_connection_count(stage, dx, dy) >= MIN_CONNECTION) {

                            stage->topLayer[i] += 127;
                            stage->redrawBuffer[i] = true;
                            break;
                        }
                    }
                }
            }

            // Mark player(s) for redraw
            if (destroy && stage->topLayer[i] == 2) {
                
                stage->redrawBuffer[i] = true;
            }
        }
    }

    if (buttonWithoutObject == stage->wallState) {

        stage->wallState = !stage->wallState;
        if (destroy) {

            return ANIMATION_DESTROY;
        }
        else {

            return ANIMATION_TOGGLE_WALLS;
        }
    }

    if (destroy) {

        return ANIMATION_DESTROY;
    }

    // Well, actually no animation at all!
    return ANIMATION_MOVE;
}


static bool clear_destroyed_objects(_Stage* stage) {

    i16 i;

    bool buttonWithoutObject = false;

    for (i = 0; i < stage->width*stage->height; ++ i) {

        if (stage->topLayer[i] >= 127) {

            stage->topLayer[i] = 0;
            if (stage->bottomLayer[i] == 11) {

                buttonWithoutObject = true;
            }
        }
    }

    if (buttonWithoutObject) {

        return check_effects(stage, false) == ANIMATION_TOGGLE_WALLS;
    }
    return false;
}


static void store_state(_Stage* stage) {

    stage->bufferPointer = (stage->bufferPointer + 1) % (stage->bufferSize);
    stage->undoCount = min_i16(stage->bufferSize-1, stage->undoCount+1);

    memcpy(stage->bottomLayerBuffer[stage->bufferPointer], 
        stage->bottomLayer, stage->width*stage->height);
    memcpy(stage->topLayerBuffer[stage->bufferPointer], 
        stage->topLayer, stage->width*stage->height);

    stage->wallStateBuffer[stage->bufferPointer] = stage->wallState;
}


static void update_animation(_Stage* stage, i16 step) {

    const i16 ANIM_SPEED = 24;

    bool cloneBuffer = true;

    stage->animationTimer -= ANIM_SPEED * step;
    if (stage->animationTimer <= 0) {

        if (stage->animType == ANIMATION_DESTROY) {

            if (clear_destroyed_objects(stage)) {

                stage->animationTimer = TOGGLE_TIME;
                cloneBuffer = false;
            }
        }
        else if (stage->animType == ANIMATION_MOVE) {

            stage->animType = check_effects(stage, stage->nonPlayerMoved);
  
            if (stage->nonPlayerMoved &&
                stage->animType == ANIMATION_DESTROY) {

                stage->animationTimer = DESTROY_TIME;
                cloneBuffer = false;
            }
            else if (stage->animType == ANIMATION_TOGGLE_WALLS) {

                stage->animationTimer = TOGGLE_TIME;
                cloneBuffer = false;
            }
        }

        if (cloneBuffer) {

            store_state(stage);
        }
    }
}


void init_stage() {

    compute_fibonacci();
}


Stage* new_stage(u16 maxWidth, u16 maxHeight) {

    const u16 MAX_DUST_COUNT = 3;

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

    stage->connectionBuffer = (u8*) calloc(maxWidth*maxHeight, sizeof(u8));
    if (stage->connectionBuffer == NULL) {

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

    stage->wallStateBuffer = (bool*) calloc(maxWidth*maxHeight, sizeof(bool));
    if (stage->wallStateBuffer == NULL) {

        m_memory_error();

        dispose_stage((Stage*) stage);
        return NULL;
    }

    stage->dust = (Dust*) calloc(MAX_DUST_COUNT, sizeof(Dust));
    if (stage->dust == NULL) {

        m_memory_error();

        dispose_stage((Stage*) stage);
        return NULL;
    }
    stage->dustCount = MAX_DUST_COUNT;

    stage->bufferPointer = 0;
    stage->bufferSize = BUFFER_MAX_COUNT;
    stage->undoCount = 0;

    stage->animationTimer = 0;
    stage->animDir = 3;
    stage->nonPlayerMoved = false;
    stage->animType = ANIMATION_MOVE;

    stage->baseMap = NULL;
    stage->wallState = false;

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
    m_free(stage->wallStateBuffer);

    m_free(stage->dust);
    m_free(stage->redrawBuffer);
    m_free(stage->bottomLayer);
    m_free(stage->topLayer);
    m_free(stage->connectionBuffer);
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

    stage->baseMap = tilemap;

    stage->wallState = false;
}


void stage_update(Stage* _stage, i16 step) {

    _Stage* stage = (_Stage*) _stage;
    i16 i;

    if (stage->animationTimer > 0) {

        update_animation(stage, step);
    }
    else {

        control(stage);
    }

    for (i = 0; i < stage->dustCount; ++ i) {

        update_dust_particle(&stage->dust[i], stage, step);
    }
}


void stage_draw(Stage* _stage, Canvas* canvas, 
    Bitmap* staticTiles, Bitmap* dynamicTiles) {

    _Stage* stage = (_Stage*) _stage;

    i16 left = 160 - stage->width*12;
    i16 top = 100 - stage->height*10;
    i16 i;

    canvas_set_clip_area(canvas, left, top, stage->width*24, stage->height*20);

    canvas_toggle_clipping(canvas, false);
    draw_static_layer(stage, canvas, staticTiles, left, top);

    for (i = 0; i < stage->dustCount; ++ i) {

        draw_dust(&stage->dust[i], canvas, dynamicTiles, left, top);
    }

    canvas_toggle_clipping(canvas, true);
    draw_dynamic_layer(stage, canvas, dynamicTiles, left, top);

    canvas_reset_clip_area(canvas);
}


void stage_get_size(Stage* _stage, i16* width, i16* height) {

    _Stage* stage = (_Stage*) _stage;

    *width = stage->width;
    *height = stage->height;
}


bool stage_reset(Stage* _stage) {

    _Stage* stage = (_Stage*) _stage;

    if (stage->animationTimer > 0)
        return false;

    stage_init_tilemap(_stage, stage->baseMap);
    memset(stage->redrawBuffer, 1, stage->width*stage->height);

    stage->animDir = 3;

    return true;
}
