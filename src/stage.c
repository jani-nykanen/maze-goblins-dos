#include "stage.h"
#include "system.h"
#include "mathext.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {

    u16 maxWidth;
    u16 maxHeight;

    u16 width;
    u16 height;

    u8* bottomLayer;
    u8* topLayer;
    u8* redrawBuffer;

} _Stage;


static void draw_static_layer(_Stage* stage, Canvas* canvas, Bitmap* staticTiles,
    i16 left, i16 top) {

    i16 x, y;
    i16 i = 0;

    i16 dx, dy;

    i16 tileID;

    for (y = 0; y < stage->maxHeight; ++ y) {

        for (x = 0; x < stage->maxWidth; ++ x) {

            i = y * stage->maxWidth + x;

            if (!stage->redrawBuffer[i] ||
                x >= stage->width) 
                continue;

            dx = left + x*24;
            dy = top + y*20;

            switch (stage->bottomLayer[i]) {

            // Tile floor
            case 1:

                canvas_draw_bitmap_region_fast(canvas, staticTiles, 0, 0, 24, 20, dx, dy);
                break;

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

    i16 tileID;

    for (y = 0; y < stage->maxHeight; ++ y) {

        for (x = 0; x < stage->maxWidth; ++ x) {

            i = y * stage->maxWidth + x;

            if (!stage->redrawBuffer[i] ||
                x >= stage->width) 
                continue;

            dx = left + x*24;
            dy = top + y*20;

            switch (stage->bottomLayer[i]) {

            // Imps
            case 4:
            case 5:
            case 6:

                canvas_draw_bitmap_region(canvas, dynamicTiles, 
                    0, 0, 24, 20, dx, dy, false);
                break;

            default:
                break;
            }

            stage->redrawBuffer[i] = 0;
        }

        if (y >= stage->height)
            break;
    }
}


Stage* new_stage(u16 maxWidth, u16 maxHeight) {

    _Stage* stage = (_Stage*) calloc(1, sizeof(_Stage));
    if (stage == NULL) {

        m_memory_error();
        return NULL;
    }

    stage->maxWidth = maxWidth;
    stage->maxHeight = maxHeight;

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

    stage->redrawBuffer = (u8*) calloc(maxWidth*maxHeight, sizeof(u8));
    if (stage->redrawBuffer == NULL) {

        m_memory_error();

        dispose_stage((Stage*) stage);
        return NULL;
    }

    return (Stage*) stage;
}


void dispose_stage(Stage* _stage) {

    _Stage* stage = (_Stage*) _stage;

    if (stage == NULL) return;

    m_free(stage->redrawBuffer);
    m_free(stage->bottomLayer);
    m_free(stage);
}


void stage_init_tilemap(Stage* _stage, Tilemap* tilemap) {

    _Stage* stage = (_Stage*) _stage;

    u16 w;
    u16 h;

    tilemap_get_size(tilemap, &w, &h);
    tilemap_copy(tilemap, stage->bottomLayer, stage->maxWidth);
    tilemap_copy(tilemap, stage->topLayer, stage->maxWidth);

    memset(stage->redrawBuffer, 1, stage->maxWidth*stage->maxHeight);

    stage->width = min_u16(stage->maxWidth, w);
    stage->height = min_u16(stage->maxHeight, h);
}


void stage_update(Stage* _stage, i16 step) {

    _Stage* stage = (_Stage*) _stage;
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
