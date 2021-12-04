#ifndef PROJECTNAME_CANVAS_H
#define PROJECTNAME_CANVAS_H


#include "types.h"


EMPTY_STRUCT(Canvas);


Canvas* new_canvas(u16 width, u16 height);
void dispose_canvas(Canvas* canvas);

void canvas_clear(Canvas* canvas, u8 color);

void canvas_copy_to_memory_location(Canvas* canvas, u32 loc);


#endif // PROJECTNAME_CANVAS_H
