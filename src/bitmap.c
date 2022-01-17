#include "bitmap.h"
#include "system.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {

    u16 width;
    u16 height;
    u8* pixels;
    u8* mask;

} _Bitmap;


typedef struct {

    u16 width;
    u16 height;
    u16 count;

    _Bitmap** sprites;

} _SpriteSheet;


static bool next_char(u8* out, FILE* f) {

    i16 c = fgetc(f);
    if (c == EOF) return true;

    *out = (u8)c;

    return false;
}


static void decode_RLE(u8* data, u16 size, FILE* f) {

    u8 color;
    u8 length;
    u16 p = 0;
    u8 i;

    while (true) {

        if (next_char(&color, f)) return;
        if (next_char(&length, f)) return;

        for (i = 0; i < length; ++ i) {

            data[p ++] = color;

            if (p == size)
                return;
        }
    }
}


Bitmap* new_bitmap(u16 width, u16 height, bool createMask) {

    _Bitmap* bmp = (_Bitmap*) calloc(1, sizeof(_Bitmap));
    if (bmp == NULL) {

        m_memory_error();
        return NULL;
    }

    bmp->width = width;
    bmp->height = height;

    bmp->pixels = (u8*) calloc(width*height, sizeof(u8));
    if (bmp->pixels == NULL) {

        m_memory_error();
        dispose_bitmap((Bitmap*) bmp);
        return NULL;
    }

    bmp->mask = NULL;
    if (createMask) {

        bmp->mask = (u8*) calloc(width*height, sizeof(u8));
        if (bmp->mask == NULL) {

            m_memory_error();
            dispose_bitmap((Bitmap*) bmp);
            return NULL;
        }
    }

    return (Bitmap*) bmp;
}


Bitmap* create_bitmap_from_data(u16 width, u16 height, 
    u8* pixels, u8* mask, u32 start,  u32 offset) {

    u16 x, y;
    u32 i;

    _Bitmap* bmp = (_Bitmap*) new_bitmap(width, height, mask != NULL);
    if (bmp == NULL) {

        return NULL;
    }

    i = start;
    for (y = 0; y < height; ++ y) {

        for (x = 0; x < width; ++ x, ++ i) {

            bmp->pixels[y*width + x] = pixels[i];
        }
        i += offset - width;
    }


    if (mask != NULL) {

        i = start;
        for (y = 0; y < height; ++ y) {

            for (x = 0; x < width; ++ x, ++ i) {

                bmp->mask[y*width + x] = mask[i];
            }
            i += offset - width;
        }
    }

    return (Bitmap*) bmp; 
}


Bitmap* load_bitmap(str path) {

    _Bitmap* bmp = NULL;
    FILE* f;
    u16 width;
    u16 height;
    u8 hasMask;
    u8 RLE;

    f = fopen(path, "rb");
    if (f == NULL) {

        m_throw_error("The file \'", path, "\' does not exist.");
        return NULL;
    }

    fread(&width, sizeof(u16), 1, f);
    fread(&height, sizeof(u16), 1, f);
    fread(&hasMask, sizeof(u8), 1, f);
    fread(&RLE, sizeof(u8), 1, f);

    bmp = (_Bitmap*) new_bitmap(width, height, hasMask);
    if (bmp == NULL) {

        fclose(f);
        return NULL;
    }

    if (!RLE) {

        fread(bmp->pixels, sizeof(u8), width*height, f);
        if (hasMask) {

            fread(bmp->mask, sizeof(u8), width*height, f);
        }
    }
    else {

        decode_RLE(bmp->pixels, width*height, f);
        if (hasMask) {

            decode_RLE(bmp->mask, width*height, f);
        }
    }

    fclose(f);

    return (Bitmap*) bmp;
}


void dispose_bitmap(Bitmap* _bmp) {

    _Bitmap* bmp = (_Bitmap*) _bmp;

    if (bmp == NULL) return;

    m_free(bmp->pixels);
    m_free(bmp->mask);
    m_free(bmp);
}


SpriteSheet* create_sprite_sheet_from_bitmap(Bitmap* _bmp, u16 width, u16 height) {
    
    _SpriteSheet* sheet;
    _Bitmap* bmp = (_Bitmap*) _bmp;
    i16 x, y;
    u16 w, h;
    u32 start;

    if (bmp->width % width != 0 || bmp->height % height != 0) {

        m_throw_error("Could not create a sprite sheet from a bitmap: invalid dimensions.", NULL, NULL);
        return NULL;
    }
    
    sheet = (_SpriteSheet*) calloc(1, sizeof(_SpriteSheet));
    if (sheet == NULL) {

        m_memory_error();
        return NULL;
    }

    w = bmp->width % width;
    h = bmp->height % height;

    sheet->count = w * h;
    sheet->sprites = (_Bitmap**) calloc(sheet->count, sizeof(_Bitmap*));
    if (sheet->sprites == NULL) {

        m_memory_error();
        return NULL;
    }

    for (y = 0; y < h; ++ y) {
   
        for (x = 0; x < w; ++ x) {

            start = y * h * bmp->width + x * w;

            sheet->sprites[y*w + x] = (_Bitmap*)create_bitmap_from_data(width, height, 
                    bmp->pixels, bmp->mask,
                    start, (u32) bmp->width);
            if (sheet->sprites[y*w + x] == NULL) {

                dispose_sprite_sheet((SpriteSheet*) sheet);
                return NULL;
            }
        }
    }

    return (SpriteSheet*) sheet;
}


void dispose_sprite_sheet(SpriteSheet* _sheet) {

    _SpriteSheet* sheet = (_SpriteSheet*) _sheet;
    u16 i;
    
    if (sheet == NULL) return;

    for (i = 0; i < sheet->count; ++ i) {

        dispose_bitmap((Bitmap*) sheet->sprites[i]);
    }
    m_free(sheet);
}
