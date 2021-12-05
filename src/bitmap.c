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


Bitmap* new_bitmap(u16 width, u16 height, bool createMash) {

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
    if (createMash) {

        bmp->mask = (u8*) calloc(width*height, sizeof(u8));
        if (bmp->mask == NULL) {

            m_memory_error();
            dispose_bitmap((Bitmap*) bmp);
            return NULL;
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

