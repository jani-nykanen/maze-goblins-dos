#include "canvas.h"
#include "system.h"
#include "palette.h"

#include <stdlib.h>
#include <string.h>


#define TEXT_FUNCTION(func) DirectBitmap* bmp = (DirectBitmap*) _bmp;\ 
\ 
    i16 dx, dy;\ 
\ 
    i16 charw = bmp->width / 16;\ 
    i16 charh = charw;\ 
\ 
    i16 chr;\ 
    i16 i = 0;\ 
\ 
    i16 sx;\ 
    i16 sy;\ 
\ 
    switch (align) {\ 
    \ 
    case ALIGN_RIGHT:\ 
        x -= strlen(text) * (charw + xoff);\ 
        break;\ 
\ 
    case ALIGN_CENTER:\ 
        x -= strlen(text) * (charw + xoff) / 2;\ 
        break;\ 
    \ 
    default:\ 
        break;\ 
    }\ 
\ 
    dx = x;\ 
    dy = y;\ 
\ 
    while ((i16)(chr = text[i ++]) != '\0') {\ 
\ 
        if (chr == '\n') {\ 
\ 
            dy += charh + yoff;\ 
            dx = x;\ 
            continue;\ 
        }\ 
\ 
        sx = chr % 16;\ 
        sy = chr / 16;\ 
\ 
        func;\ 
\ 
        dx += charw + xoff;\ 
    }


typedef struct {

    u16 width;
    u16 height;
    u8* pixels;
    u8* mask;

} DirectBitmap;


typedef struct {

    u16 width;
    u16 height;
    u8* pixels;

    bool clippingEnabled;
    Rect_i16 clipArea;

    i16 hue;

} _Canvas;


static bool clip_rect(_Canvas* canvas, 
    i16* x, i16* y, 
    i16* w, i16* h) {

    // Left
    if (*x < canvas->clipArea.x) {

        *w -= (canvas->clipArea.x - (*x));
        *x = canvas->clipArea.x;
    }
    // Right
    if (*x+*w >= canvas->clipArea.x + canvas->clipArea.w) {

        *w -= (*x+*w) - (canvas->clipArea.x + canvas->clipArea.w);
    }

    // Top
    if (*y < canvas->clipArea.y) {

        *h -= (canvas->clipArea.y - (*y));
        *y = canvas->clipArea.y;
    }
    // Bottom
    if (*y+*h >= canvas->clipArea.y + canvas->clipArea.h) {

        *h -= (*y + *h) - (canvas->clipArea.y + canvas->clipArea.h);
    }

    return *w > 0 && *h > 0;
}


static bool clip_rect_region(_Canvas* canvas, 
    i16* sx, i16* sy, i16* sw, i16* sh, 
    i16* dx, i16* dy, bool flip) {

    i16 ow, oh;

    // Left
    ow = *sw;
    if(*dx < canvas->clipArea.x) {

        *sw -= canvas->clipArea.x - (*dx);
        if(!flip)
            *sx += ow-(*sw);

        *dx = canvas->clipArea.x;
    }
    // Right
    if(*dx+*sw >= canvas->clipArea.x + canvas->clipArea.w) {

         *sw -= (*dx + *sw) - (canvas->clipArea.x + canvas->clipArea.w); 
         if(flip)
            *sx += ow-(*sw);
    }

    // Top
    oh = *sh;
    if(*dy < canvas->clipArea.y) {

        *sh -= canvas->clipArea.y - (*dy);
        *sy += oh-*sh;
        *dy = canvas->clipArea.y;
    }
    // Bottom
    if(*dy+*sh >= canvas->clipArea.y + canvas->clipArea.h) {

        *sh -= (*dy + *sh) - (canvas->clipArea.y + canvas->clipArea.h);
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
    c->clipArea = rect_i16(0, 0, 320, 200);

    c->hue = 0;

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

    if (canvas->hue == 0) {

        memcpy((void*)(size_t)loc, canvas->pixels, canvas->width*canvas->height);
    }
    else {

        copy_hued_data_to_location(canvas->pixels, loc, 
             (u32) (canvas->width*canvas->height), canvas->hue);
    }
}


void canvas_fill_rect(Canvas* _canvas, i16 dx, i16 dy, i16 dw, i16 dh, u8 color) {

    _Canvas* canvas = (_Canvas*) _canvas;

    u32 dest;
    i16 y;

    if (canvas->clippingEnabled && 
        clip_rect(canvas, &dx, &dy, &dw, &dh))
        return;

    dest = dy * canvas->width + dx;

    for (y = 0; y < dh; ++ y) {
        
        memset(canvas->pixels + dest, color, dw);
        dest += (u32) canvas->width;
    }
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


void canvas_draw_text_fast(Canvas* canvas, Bitmap* _bmp,
    str text, i16 x, i16 y, i16 xoff, i16 yoff, TextAlign align) {

    TEXT_FUNCTION(canvas_draw_bitmap_region_fast(canvas, _bmp, 
            sx*charw, sy*charh, charw, charh, dx, dy));
}


void canvas_draw_text(Canvas* canvas, Bitmap* _bmp,
    str text, i16 x, i16 y, i16 xoff, i16 yoff, TextAlign align) {

    TEXT_FUNCTION(canvas_draw_bitmap_region(canvas, _bmp, 
            sx*charw, sy*charh, charw, charh, dx, dy, false));
}


void canvas_reset_clip_area(Canvas* canvas) {

    canvas_set_clip_area(canvas, 0, 0, 320, 200);
}


void canvas_set_clip_area(Canvas* _canvas, i16 x, i16 y, i16 w, i16 h) {

    _Canvas* canvas = (_Canvas*) _canvas;

    canvas->clipArea = rect_i16(x, y, w, h);
}


void canvas_darken(Canvas* _canvas, i16 amount) {

    _Canvas* canvas = (_Canvas*) _canvas;

    u16 i;

    for (i = 0; i < canvas->width*canvas->height; ++ i) {

        canvas->pixels[i] = darken_color(canvas->pixels[i], amount);
    }
}


void canvas_set_global_hue(Canvas* _canvas, i16 hue) {

    _Canvas* canvas = (_Canvas*) _canvas;

    canvas->hue = hue;
}


void canvas_get_size(Canvas* _canvas, u16* w, u16* h) {

    _Canvas* canvas = (_Canvas*) _canvas;

    *w = canvas->width;
    *h = canvas->height;
}
