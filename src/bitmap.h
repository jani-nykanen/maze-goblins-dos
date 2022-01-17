#ifndef PROJECTNAME_BITMAP_H
#define PROJECTNAME_BITMAP_H


#include "types.h"


EMPTY_STRUCT(Bitmap);
EMPTY_STRUCT(SpriteSheet);


Bitmap* new_bitmap(u16 width, u16 height, bool createMask);
Bitmap* create_bitmap_from_data(u16 width, u16 height, 
    u8* pixels, u8* mask, u32 start, u32 offset);
Bitmap* load_bitmap(str path);
void dispose_bitmap(Bitmap* bmp);


SpriteSheet* create_sprite_sheet_from_bitmap(Bitmap* bmp, u16 width, u16 height);
void dispose_sprite_sheet(SpriteSheet* sheet);


#endif // PROJECTNAME_BITMAP_H
