#include "canvas.h"
#include "system.h"

#include <stdlib.h>
#include <string.h>


typedef struct {

    u16 width;
    u16 height;
    u8* pixels;
    u8* mask;

} DirectBitmap;


typedef struct {

    u16 width;
    u16 height;
    u16 count;

    DirectBitmap** sprites;

} DirectSpriteSheet;


typedef struct {

    u16 width;
    u16 height;
    u8* pixels;

    bool clippingEnabled;

} _Canvas;


static bool clip_rect(_Canvas* canvas, 
    i16* x, i32* y, 
    i16* w, i32* h) {

    // Left
    if (*x < 0) {

        *w -= 0 - (*x);
        *x = 0;
    }
    // Right
    if (*x+*w >= canvas->width) {

        *w -= (*x+*w) - canvas->width;
    }

    // Top
    if (*y < 0) {

        *h -= 0 - (*y);
        *y = 0;
    }
    // Bottom
    if (*y+*h >= canvas->height) {

        *h -= (*y + *h) - canvas->height;
    }

    return *w > 0 && *h > 0;
}


static bool clip_rect_region(_Canvas* canvas, 
    i16* sx, i16* sy, i16* sw, i16* sh, 
    i16* dx, i16* dy, bool flip) {

    i16 ow, oh;

    // Left
    ow = *sw;
    if(*dx < 0) {

        *sw += (*dx);
        if(!flip)
            *sx += ow-(*sw);

        *dx = 0;
    }
    // Right
    if(*dx+*sw >= canvas->width) {

         *sw -= (*dx + *sw) - canvas->width; 
         if(flip)
            *sx += ow-(*sw);
    }

    // Top
    oh = *sh;
    if(*dy < 0) {

        *sh += (*dy);
        *sy += oh-*sh;
        *dy = 0;
    }
    // Bottom
    if(*dy+*sh >= canvas->height) {

        *sh -= (*dy + *sh) - canvas->height;
    }

    return *sw > 0 && *sh > 0;
}


Canvas* new_canvas(u16 width, u16 height) {

    _Canvas* c = (_Canvas*) calloc(1, sizeof(_Canvas));
    if (c == NULL) {

        m_memory_error();
        return NULL;
    }

    c->width = width;
    c->height = height;

    c->pixels = (u8*) calloc(width*height, sizeof(u8));
    if (c->pixels == NULL) {

        m_memory_error();
        dispose_canvas((Canvas*) c);
        return NULL;
    }

    c->clippingEnabled = true;

    return (Canvas*) c;
}


void dispose_canvas(Canvas* _canvas) {

    _Canvas* canvas = (_Canvas*) _canvas;

    if (canvas == NULL) return;

    m_free(canvas->pixels);
    m_free(canvas);
}


void canvas_toggle_clipping(Canvas* _canvas, bool state) {

    _Canvas* canvas = (_Canvas*) _canvas;

    canvas->clippingEnabled = state;
}


void canvas_clear(Canvas* _canvas, u8 color) {

    _Canvas* canvas = (_Canvas*) _canvas;

    memset(canvas->pixels, color, canvas->width*canvas->height);
}


void canvas_copy_to_memory_location(Canvas* _canvas, u32 loc) {

    _Canvas* canvas = (_Canvas*) _canvas;

    memcpy((void*)(size_t)loc, canvas->pixels, canvas->width*canvas->height);
}


void canvas_draw_bitmap_region_fast(Canvas* _canvas, Bitmap* _bmp,
    i16 sx, i16 sy, i16 sw, i16 sh, i16 dx, i16 dy) {

    _Canvas* canvas = (_Canvas*) _canvas;
    DirectBitmap* bmp = (DirectBitmap*) _bmp;

    u32 dest;
    u32 src;
    i16 y;

    if (canvas->clippingEnabled &&
        !clip_rect_region(canvas, &sx, &sy, &sw, &sh, &dx, &dy, false))
        return;

    dest = dy * canvas->width + dx;
    src = sy * bmp->width + sx;

    for (y = 0; y < sh; ++ y) {
        
        memcpy(canvas->pixels + dest, bmp->pixels + src, sw);
        src += (u32) bmp->width;
        dest += (u32) canvas->width;
    }
}


void canvas_draw_bitmap_fast(Canvas* canvas, Bitmap* _bmp, i16 dx, i16 dy) {

    DirectBitmap* bmp = (DirectBitmap*) _bmp;

    canvas_draw_bitmap_region_fast(canvas, _bmp, 
        0, 0, bmp->width, bmp->height, dx, dy);
}


void canvas_draw_bitmap_region(Canvas* _canvas, Bitmap* _bmp,
    i16 sx, i16 sy, i16 sw, i16 sh, i16 dx, i16 dy, bool flip) {

    _Canvas* canvas = (_Canvas*) _canvas;
    DirectBitmap* bmp = (DirectBitmap*) _bmp;

    u32 dest;
    u32 src;
    i16 x, y;
    i16 dir = 1 - 2 * flip;

    if (canvas->clippingEnabled &&
        !clip_rect_region(canvas, &sx, &sy, &sw, &sh, &dx, &dy, flip))
        return;

    // TODO: Do something when no mask is given and want to flip?
    if (bmp->mask == NULL) {

        if (!flip) {

            canvas_draw_bitmap_region_fast(_canvas, _bmp, sx, sy, sw, sh, dx, dy);
        }
        return;
    }

    src = sy * bmp->width + sx;
    if (flip)
        src += sw - 1;

    dest = dy * canvas->width + dx;

    for (y = 0; y < sh; ++ y) {

        for (x = 0; x < sw; ++ x) {

            canvas->pixels[dest] = (bmp->pixels[src] & bmp->mask[src]) | 
                        (canvas->pixels[dest] & (~bmp->mask[src]));

            ++ dest;
            src += dir;
        }

        dest += canvas->width - sw;
        src += bmp->width - sw * dir;
    }
}


void canvas_draw_bitmap(Canvas* canvas, Bitmap* _bmp, i16 dx, i16 dy, bool flip) {

    DirectBitmap* bmp = (DirectBitmap*) _bmp;

    canvas_draw_bitmap_region(canvas, _bmp, 
        0, 0, bmp->width, bmp->height, dx, dy, flip);
}


void canvas_draw_sprite_fast(Canvas* _canvas, SpriteSheet* _sheet,
    i16 frame, i16 dx, i16 dy, bool flip) {

    _Canvas* canvas = (_Canvas*) _canvas;
    DirectSpriteSheet* sheet = (DirectSpriteSheet*) sheet;

    u32 dest;
    u32 src;
    i16 y;

    // If clipping is enabled, just draw the sprite normally, because
    // there is no speed benefits anyway
    if (canvas->clippingEnabled) {

        canvas_draw_bitmap_region_fast(_canvas, sheet->sprites[frame], 
            0, 0, sheet->width, sheet->height, dx, dy);
        return;
    }

    // Otherwise draw things in a slightly faster way
    
    dest = dy * canvas->width + dx;
    src = 0;

    for (y = 0; y < sheet->height; ++ y) {
        
        memcpy(canvas->pixels + dest, sheet->sprites[frame]->pixels + src, sheet->width);
        src += (u32) sheet->width;
        dest += (u32) canvas->width;
    }
}   


void canvas_draw_text_fast(Canvas* canvas, Bitmap* _bmp,
    str text, i16 x, i16 y, i16 xoff, i16 yoff, TextAlign align) {

    DirectBitmap* bmp = (DirectBitmap*) _bmp;

    i16 dx, dy;

    i16 charw = bmp->width / 16;
    i16 charh = charw;

    i16 chr;
    i16 i = 0;

    i16 sx;
    i16 sy;

    switch (align) {
    
    case ALIGN_RIGHT:
        x -= strlen(text) * (charw + xoff);
        break;

    case ALIGN_CENTER:
        x -= strlen(text) * (charw + xoff) / 2;
        break;
    
    default:
        break;
    }

    dx = x;
    dy = y;

    while ((i16)(chr = text[i ++]) != '\0') {

        if (chr == '\n') {

            dy += charh + yoff;
            dx = x;
            continue;
        }

        sx = chr % 16;
        sy = chr / 16;

        canvas_draw_bitmap_region_fast(canvas, _bmp, 
            sx*charw, sy*charh, charw, charh, dx, dy);

        dx += charw + xoff;
    }
}


void canvas_draw_text(Canvas* canvas, Bitmap* bmp,
    str text, i16 x, i16 y, i16 xoff, i16 yoff, TextAlign align) {

}
