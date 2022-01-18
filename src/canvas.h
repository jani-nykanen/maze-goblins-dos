#ifndef PROJECTNAME_CANVAS_H
#define PROJECTNAME_CANVAS_H


#include "types.h"
#include "bitmap.h"


typedef enum {

    ALIGN_LEFT = 0,
    ALIGN_CENTER = 1,
    ALIGN_RIGHT = 2

} TextAlign;


EMPTY_STRUCT(Canvas);


Canvas* new_canvas(u16 width, u16 height);
void dispose_canvas(Canvas* canvas);

void canvas_toggle_clipping(Canvas* canvas, bool state);

void canvas_clear(Canvas* canvas, u8 color);

void canvas_copy_to_memory_location(Canvas* canvas, u32 loc);

void canvas_fill_rect(Canvas* canvas, i16 dx, i16 dy, i16 dw, i16 dh, u8 color);

void canvas_draw_bitmap_region_fast(Canvas* canvas, Bitmap* bmp,
    i16 sx, i16 sy, i16 sw, i16 sh, i16 dx, i16 dy);
void canvas_draw_bitmap_fast(Canvas* canvas, Bitmap* bmp, i16 dx, i16 dy);

void canvas_draw_bitmap_region(Canvas* canvas, Bitmap* bmp,
    i16 sx, i16 sy, i16 sw, i16 sh, i16 dx, i16 dy, bool flip);
void canvas_draw_bitmap(Canvas* canvas, Bitmap* bmp, 
    i16 dx, i16 dy, bool flip);

void canvas_draw_text_fast(Canvas* canvas, Bitmap* bmp,
    str text, i16 x, i16 y, 
    i16 xoff, i16 yoff, TextAlign align);
void canvas_draw_text(Canvas* canvas, Bitmap* bmp,
    str text, i16 x, i16 y, 
    i16 xoff, i16 yoff, TextAlign align);   

void canvas_reset_clip_area(Canvas* canvas);
void canvas_set_clip_area(Canvas* canvas, i16 x, i16 y, i16 w, i16 h);


#endif // PROJECTNAME_CANVAS_H
