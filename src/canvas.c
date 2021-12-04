#include "canvas.h"
#include "system.h"

#include <stdlib.h>
#include <string.h>


typedef struct {

    u16 width;
    u16 height;
    u8* pixels;

} _Canvas;


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

    return (Canvas*) c;
}


void dispose_canvas(Canvas* _canvas) {

    _Canvas* canvas = (_Canvas*) _canvas;

    if (canvas == NULL) return;

    m_free(canvas->pixels);
    m_free(canvas);
}


void canvas_clear(Canvas* _canvas, u8 color) {

    _Canvas* canvas = (_Canvas*) _canvas;

    memset(canvas->pixels, color, canvas->width*canvas->height);
}


void canvas_copy_to_memory_location(Canvas* _canvas, u32 loc) {

    _Canvas* canvas = (_Canvas*) _canvas;

    memcpy((void*)(size_t)loc, canvas->pixels, canvas->width*canvas->height);
}
