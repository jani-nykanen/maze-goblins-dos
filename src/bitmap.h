#ifndef PROJECTNAME_BITMAP_H
#define PROJECTNAME_BITMAP_H


#include "types.h"


EMPTY_STRUCT(Bitmap);


Bitmap* new_bitmap(u16 width, u16 height, bool createMask);
Bitmap* create_bitmap_from_data(u16 width, u16 height, 
    u8* pixels, u8* mask, u32 start, u32 offset);
Bitmap* create_monochrome_bitmap_from_data(u16 width, u16 height, u8* pixels);
Bitmap* load_bitmap(str path);
void dispose_bitmap(Bitmap* bmp);

void bitmap_get_size(Bitmap* bmp, u16* w, u16* h);


#endif // PROJECTNAME_BITMAP_H
