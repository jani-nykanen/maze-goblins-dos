//
// Notes: this code is from three different years, from several
// (possibly unfinished) projects. I'm too lazy to rewrite it since
// it works, so I'll just let it be. However, note the following:
// the coding style might be a bit different than in the main code.
//


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



static bool str_exists(str word, str* argv, u32 len, u32 start) {
    
	u32 i;
	
	for (i = start; i < len; ++ i) {
		
		if (strcmp(word, argv[i]) == 0)
			return true;
	}
	return false;
}


static i32 store_RLE_data(u8* data, size_t size, FILE* f) {

    u8 current, color, out;
    i32 p = 0;
    i32 length = 0;

    current = data[p ++];
    fwrite(&current, 1, 1, f);

    for (; p < size; ++ p) {

        color = data[p];
        ++ length;

        if (color != current) {

            out = (u8)length;
            fwrite(&out, 1, 1, f);

            current = color;
            fwrite(&current, 1, 1, f);

            length = 0;
        }
        else if (length == 255) {

            out = 255;
            fwrite(&out, 1, 1, f);

            fwrite(&current, 1, 1, f);
            length = 0;
        }
    }
    out = (u8)(length +1);
    fwrite(&out, 1, 1, f);

    return 0;
}



static i32 convert_bitmap(str in, str out, bool useRLE, bool bw) {

    const u8 ALPHA = 170;
    const f32 DIVISOR1 = 36.428f;
    const i32 DIVISOR2 = 85;

    i32 w, h;
    i32 channels;
    u8* pdata;

    i32 i = 0;
    i32 row = 0;
    i32 column = 0;
    i32 pixelCount;

    u8 pixel;
    u8 r,g,b,a;
    u8 er,eg,eb;
    u8 byte;

    u8* data;
    u8* mask;
    
    bool anyAlpha = false;

    FILE* f;

    pdata = stbi_load(in, &w, &h, &channels, 0);
    if (pdata == NULL) {
        
        printf("png2bin: Failed to load a bitmap in %s!\n", in);
        return 1;
    }

    pixelCount = w * h;

    data = (u8*) calloc(pixelCount, sizeof(u8));
    if (data == NULL) {

        printf("png2bin: Memory allocation error!\n");
        return 1;
    }

    mask = (u8*) calloc(pixelCount, sizeof(u8));
    if (mask == NULL) {
        
        printf("png2bin: Memory allocation error!\n");
        return 1;
    }

    for(i = 0; i < pixelCount; ++ i) {

        row = i / w;
        column = i % w;

        if (channels == 4) {

            a = pdata[i*4 +3];
            mask[i] = a < 255 ? 0 : 255;

            if (a < 255) anyAlpha = true;
        }

        b = pdata[i*channels +2];
        g = pdata[i*channels +1];
        r = pdata[i*channels ];
		
		if (bw) {
			
			pixel = (b + g + r) / 3;
		}
		else {

			er = (u8) roundf((f32)r / DIVISOR1);
			if (er > 7) r = 7;
			er = er << 5;

			eg = (u8) roundf((f32)g / DIVISOR1);
			if (eg > 7) eg = 7;
			eg = eg << 2;
            
			eb = (b / DIVISOR2);

			pixel = er | eg | eb;
		}
        data[i] = pixel;
    }

    f = fopen(out, "wb");
    if (f == NULL) {

        printf("png2bin: Failed to create a file to %s!\n", out);
        exit(1);
    }

    fwrite(&w, sizeof(u16), 1, f);
    fwrite(&h, sizeof(u16), 1, f);

    byte = anyAlpha && channels == 4;
    fwrite(&byte, 1, 1, f);
    
    byte = useRLE;
    fwrite(&byte, 1, 1, f);

    if (useRLE)
        store_RLE_data(data, w*h, f);
    else
        fwrite(data, 1, w*h, f);

    if (channels == 4 && anyAlpha) {

        if (useRLE)
            store_RLE_data(mask, w*h, f);
        else
            fwrite(mask, 1, w*h, f);
    }
    fclose(f);

    return 0;
}


i32 main(i32 argc, str* argv) {

    bool RLE = false;
	bool bw = false;

    if (argc < 3) {

        printf("png2bin: Must pass at least 2 arguments. Help: png2bin input output (-RLE) (-bw)\n");
        return 1;
    }

    if (argc > 3) {

        RLE = str_exists("-RLE", argv, argc, 1);
		bw = str_exists("-bw", argv, argc, 1);
    }

    if (convert_bitmap(argv[1], argv[2], RLE, bw)) {

        printf("png2bin: Image conversion failed. Terminating.\n");
        return 1;
    }
    printf("png2bin: Converted %s to %s successfully. Flags: RLE=%d, bw=%d.\n", argv[1], argv[2], RLE, bw);

    return 0;
}
