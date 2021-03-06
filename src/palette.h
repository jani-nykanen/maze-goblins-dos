#ifndef PROJECTNAME_PALETTE_H
#define PROJECTNAME_PALETTE_H


#include "types.h"


#define HUE_COUNT 8


void init_palette();

u8 darken_color(u8 color, i16 amount);
u8 lighten_color(u8 color, i16 amount);

void copy_hued_data_to_location(u8* data, u8* target, u16 len, u16 offset, i16 hue);

void palette_swap_dark(i16 hue);
void palette_swap_light(i16 hue);
void reset_palette();


#endif // PROJECTNAME_PALETTE_H
