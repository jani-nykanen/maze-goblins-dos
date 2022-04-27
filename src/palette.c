#include "palette.h"

#include <dos.h>
#include <conio.h>
#include <i86.h>
#include <graph.h>


typedef u8 Table [HUE_COUNT] [256*3];

static u8 HUE_DARK [HUE_COUNT] [256*3];
static u8 HUE_LIGHT [HUE_COUNT] [256*3];

static const u32 PALETTE_INDEX = 0x03c8;
static const u32 PALETTE_DATA = 0x03c9;


static void set_base_palette() {

    u8 i = 0;
    u8 r, g, b;

    outp(PALETTE_INDEX, 0);
    while (1) {

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

        outp(PALETTE_DATA, r >> 2);
        outp(PALETTE_DATA, g >> 2);
        outp(PALETTE_DATA, b >> 2);

        if (i == 255) break;

        ++ i;
    }
}


static void compute_hue_palettes() {

    // A bit ugly, but, well, it works, and
    // since this is called in the beginning,
    // performance isn't that important.

    i16 i = 0;
    i16 j, k;
    u8 r, g, b;
    u8 mr, mg, mb;
    u8 c;

    for (i = 0; i < 256; ++ i) {
        
        k = i * 3;
        c = (u8) i;

        r = c >> 5;
        g = c << 3;
        g >>= 5;
        b = c << 6;
        b >>= 6;

        r *= 36;
        g *= 36;
        b *= 85;

        if (r >= 252) r = 255;
        if (g >= 252) g = 255;

        mr = r;
        mg = g;
        mb = b;

        // Dark
        for (j = 0; j < HUE_COUNT; ++ j) {

            HUE_DARK[j][k] = mr;
            HUE_DARK[j][k + 1] = mg;
            HUE_DARK[j][k + 2] = mb;

            if (j == HUE_COUNT-1) break;;

            if (mr >= 36) mr -= 36;
            if (mg >= 36) mg -= 36;
            if (j % 2 != 0 && mb >= 85) mb -= 85;

        }

        mr = r;
        mg = g;
        mb = b;

        // Light
        for (j = 0; j < HUE_COUNT; ++ j) {

            HUE_LIGHT[j][k] = mr;
            HUE_LIGHT[j][k + 1] = mg;
            HUE_LIGHT[j][k + 2] = mb;

            if (j == HUE_COUNT-1) break;;

            if (mr <= 255 - 36) mr += 36;
            if (mg <= 255 - 36) mg += 36;
            if (j % 2 != 0 && mb <= 255 - 85) mb += 85;

            if (mr >= 252) mr = 255;
            if (mg >= 252) mg = 255;
        }
    }

}


static void palette_swap(i16 hue, Table* table) {

    i16 i, j;
    u8 r, g, b;

    if (hue >= HUE_COUNT) {

        hue = HUE_COUNT-1;
    }

    outp(PALETTE_INDEX, 0);

    for (i = 0; i < 256; ++ i) {

        j = i * 3;

        r = (*table)[hue][j];
        g = (*table)[hue][j + 1];
        b = (*table)[hue][j + 2];

        outp(PALETTE_DATA, r >> 2);
        outp(PALETTE_DATA, g >> 2);
        outp(PALETTE_DATA, b >> 2);
    }
}


void init_palette() {

    set_base_palette();
    compute_hue_palettes();
}


void palette_swap_dark(i16 hue) {

    palette_swap(hue, &HUE_DARK);
}


void palette_swap_light(i16 hue) {

    palette_swap(hue, &HUE_LIGHT);
}


void reset_palette() {

    palette_swap_dark(0);
}
