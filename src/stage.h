#ifndef PROJECTNAME_STAGE_H
#define PROJECTNAME_STAGE_H


#include "types.h"
#include "tilemap.h"
#include "canvas.h"
#include "audio.h"


EMPTY_STRUCT(Stage);


void init_stage();


Stage* new_stage(u16 maxWidth, u16 maxHeight);
void dispose_stage(Stage* stage);

void stage_init_tilemap(Stage* stage, Tilemap* tilemap, bool resetting, bool isFinal);

bool stage_update(Stage* stage, AudioSystem* audio, i16 step);
void stage_draw(Stage* stage, Canvas* canvas, 
    Bitmap* staticTiles, Bitmap* dynamicTiles);

void stage_get_size(Stage* stage, i16* width, i16* height);

bool stage_reset(Stage* stage, bool force);
void stage_force_redraw(Stage* stage);


#endif // PROJECTNAME_STAGE_H
