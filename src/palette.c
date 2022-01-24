#include "palette.h"

#include <dos.h>
#include <conio.h>
#include <i86.h>
#include <graph.h>


#define HUE_COUNT 8


static u8 HUE_DARK [HUE_COUNT] [256];
static u8 HUE_LIGHT [HUE_COUNT] [256];


static void set_base_palette() {

    const u32 PALETTE_INDEX = 0x03c8;
    const u32 PALETTE_DATA = 0x03c9;

    u8 i = 0;
    u8 r, g, b;

    outp(PALETTE_INDEX,0);
    do {

        r = i >> 5;
        g = i << 3;
        g >>= 5;
        b = i << 6;
        b >>= 6;

        r *= 36;
        g *= 36;
        b *= 85;

        if (r >= 252) r = 255;
        if (g >= 252) g = 255;

        outp(PALETTE_DATA, r / 4);
        outp(PALETTE_DATA, g / 4);
        outp(PALETTE_DATA, b / 4);
    }
    while ((++ i) != 0);
}


static void compute_hue_palettes() {

    i16 i = 0;
    i16 j;
    u8 r, g, b;
    u8 mr, mg, mb;
    u8 c;

    for (i = 0; i < 256; ++ i) {

        c = (u8) i;

        r = c >> 5;
        g = c << 3;
        g >>= 5;
        b = c << 6;
        b >>= 6;

        for (j = 0; j < HUE_COUNT; ++ j) {

            mr = r << 5;
            mg = g << 2;
            mb = b;

            HUE_DARK[j][i] = mr | mg | mb;

            if (j == HUE_COUNT-1) break;

            if (r > 0) -- r;
            if (g > 0) -- g;
            if (b > 0 && j % 2 != 0)
                -- b;
        }
    }

}


void init_palette() {

    set_base_palette();
    compute_hue_palettes();
}


u8 darken_color(u8 color, i16 amount) {

    if (amount <= 0)
        return color;
    else if (amount >= HUE_COUNT)
        return 0;

    return HUE_DARK[amount][(u16) color];
}


u8 lighten_color(u8 color, i16 amount) {

}
