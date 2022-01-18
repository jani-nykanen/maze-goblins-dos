#ifndef PROJECTNAME_STAGE_H
#define PROJECTNAME_STAGE_H


#include "types.h"
#include "tilemap.h"
#include "canvas.h"


EMPTY_STRUCT(Stage);


Stage* new_stage(u16 maxWidth, u16 maxHeight);
void dispose_stage(Stage* stage);

void stage_init_tilemap(Stage* stage, Tilemap* tilemap);

void stage_update(Stage* stage, i16 step);
void stage_draw(Stage* stage, Canvas* canvas, 
    Bitmap* staticTiles, Bitmap* dynamicTiles);


#endif // PROJECTNAME_STAGE_H
