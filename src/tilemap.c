#include "tilemap.h"
#include "system.h"

#include <stdlib.h>
#include <stdio.h>


TilemapPack* load_tilemap_pack(const str path) {

    TilemapPack* out;
    u16 i;
    u16 width, height;
    FILE* f;
    
    out = (TilemapPack*) calloc(1, sizeof(TilemapPack));
    if (out == NULL) {

        m_memory_error();
        return NULL;
    }
    
    f = fopen(path, "rb");
    if (f == NULL) {

        m_throw_error("Failed to open a file in ", path, NULL);
        return NULL;
    }

    fread(&out->count, 1, 1, f);

    out->maps = (Tilemap**) calloc(out->count, sizeof(Tilemap*));
    if (out->maps == NULL) {

        m_memory_error();

        free(out);
        return NULL;
    }

    for (i = 0; i < (i16) out->count; ++ i) {

        fread(&width, sizeof(u16), 1, f);
        fread(&height, sizeof(u16), 1, f);

        out->maps[i] = (Tilemap*) calloc(1, sizeof(Tilemap));
        if (out->maps[i] == NULL) {

            m_memory_error();

            dispose_tilemap_pack(out);
            return NULL;
        }

        out->maps[i]->data = (u8*) malloc(width*height);
        if (out->maps[i]->data == NULL) {

            m_memory_error();

            dispose_tilemap_pack(out);
            return NULL;
        }

        fread(out->maps[i]->data, 1, width*height, f);

        out->maps[i]->width = width;
        out->maps[i]->height = height;
    }

    fclose(f);

    return out;
}


void dispose_tilemap_pack(TilemapPack* pack) {

    u16 i;

    if (pack == NULL) return;

    for (i = 0; i < pack->count; ++ i) {

        dispose_tilemap(pack->maps[i]);
    }

    free(pack);
}


void dispose_tilemap(Tilemap* tmap) {

    if (tmap == NULL) return;

    m_free(tmap->data);
    m_free(tmap);
}
