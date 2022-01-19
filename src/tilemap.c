#include "tilemap.h"
#include "system.h"

#include <stdlib.h>
#include <stdio.h>


typedef struct {

    u8* data;
    u16 width;
    u16 height;

} _Tilemap;


typedef struct {

    _Tilemap** maps;
    u8 count;

} _TilemapPack;


TilemapPack* load_tilemap_pack(const str path) {

    _TilemapPack* out;
    u16 i;
    u16 width, height;
    FILE* f;
    
    out = (_TilemapPack*) calloc(1, sizeof(_TilemapPack));
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

    out->maps = (_Tilemap**) calloc(out->count, sizeof(_Tilemap*));
    if (out->maps == NULL) {

        m_memory_error();

        free(out);
        return NULL;
    }

    for (i = 0; i < (i16) out->count; ++ i) {

        fread(&width, sizeof(u16), 1, f);
        fread(&height, sizeof(u16), 1, f);

        out->maps[i] = (_Tilemap*) calloc(1, sizeof(_Tilemap));
        if (out->maps[i] == NULL) {

            m_memory_error();

            dispose_tilemap_pack((TilemapPack*) out);
            return NULL;
        }

        out->maps[i]->data = (u8*) malloc(width*height);
        if (out->maps[i]->data == NULL) {

            m_memory_error();

            dispose_tilemap_pack((TilemapPack*) out);
            return NULL;
        }

        fread(out->maps[i]->data, 1, width*height, f);

        out->maps[i]->width = width;
        out->maps[i]->height = height;
    }

    fclose(f);

    return (TilemapPack*) out;
}


void dispose_tilemap_pack(TilemapPack* _pack) {

    u16 i;
    _TilemapPack* pack = (_TilemapPack*) _pack;

    if (pack == NULL) return;

    for (i = 0; i < pack->count; ++ i) {

        dispose_tilemap((Tilemap*) pack->maps[i]);
    }
    m_free(pack);
}


void dispose_tilemap(Tilemap* _tilemap) {

    _Tilemap* tilemap = (_Tilemap*) _tilemap;

    if (tilemap == NULL) return;

    m_free(tilemap->data);
    m_free(tilemap);
}


Tilemap* tilemap_pack_get_tilemap(TilemapPack* _pack, u16 index) {

    _TilemapPack* pack = (_TilemapPack*) _pack;

    if (index >= pack->count)
        return NULL;

    return (Tilemap*) pack->maps[index];
}   


u16 tilemap_pack_get_tilemap_count(TilemapPack* _pack) {

    _TilemapPack* pack = (_TilemapPack*) _pack;

    return pack->count;
}


u8 tilemap_get_tile(Tilemap* _tilemap, i16 x, i16 y, u8 def) {

    _Tilemap* tilemap = (_Tilemap*) _tilemap;

    if (x < 0 || y < 0 || x >= tilemap->width || y >= tilemap->height)
        return def;

    return tilemap->data[y * tilemap->width + x];
}


void tilemap_get_size(Tilemap* _tilemap, u16* width, u16* height) {

    _Tilemap* tilemap = (_Tilemap*) _tilemap;

    *width = tilemap->width;
    *height = tilemap->height;
}


void tilemap_copy(Tilemap* _tilemap, u8* buffer) {

    _Tilemap* tilemap = (_Tilemap*) _tilemap;

    i16 x, y;

    for (y = 0; y < tilemap->height; ++ y) {

        for (x = 0; x < tilemap->width; ++ x) {

            buffer[y * tilemap->width + x] = tilemap->data[y * tilemap->width + x];
        }
    }
}
