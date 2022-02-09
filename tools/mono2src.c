

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>


#define INTEGER(type, bits) typedef type i##bits; \
typedef unsigned type u##bits;

INTEGER(char, 8)
INTEGER(short, 16)
INTEGER(int, 32)
INTEGER(long, 64)

typedef float f32;
typedef double f64;

typedef const char* str;


static i32 convert_bitmap(str in, str out) {

  
    i32 w, h;
    i32 channels;
    u8* pdata;

    i32 i;
    u8 byte;
    u8 p, q;
    i32 count;
    
    bool anyAlpha = false;

    FILE* f;

    pdata = stbi_load(in, &w, &h, &channels, 0);
    if (pdata == NULL) {
        
        printf("png2bin: Failed to load a bitmap in %s!\n", in);
        return 1;
    }

    f = fopen(out, "w");
    if (f == NULL) {

        printf("png2bin: Failed to create a file to %s!\n", out);
        exit(1);
    }

    // Header
    fprintf(f, "#define IMAGE_WIDTH %d\n#define IMAGE_HEIGHT %d\n", w, h);
    fprintf(f, "static const char IMAGE_DATA[]={");

    byte = 0;
    count = 0;
    for (i = 0; i < w*h*channels; i += channels) {

        p = pdata[i] == 255 ? 1 : 0;
        q = p << (7 - count);

        byte = byte | q;

        if (count == 7) {

            fprintf(f, "%u,", (u32)byte);
            byte = 0;
        }
        count = (count + 1) % 8;
    }
    fprintf(f, "};\n");

    fclose(f);

    return 0;
}


i32 main(i32 argc, str* argv) {

    if (argc < 3) {

        printf("mono2src: Must pass at least 2 arguments. Help: mono2src input output\n");
        return 1;
    }

    if (convert_bitmap(argv[1], argv[2])) {

        printf("mono2src: Image conversion failed. Terminating.\n");
        return 1;
    }
    printf("mono2src: Converted %s to %s successfully.\n", argv[1], argv[2]);

    return 0;
}
