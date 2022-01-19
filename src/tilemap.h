#ifndef PROJECTNAME_TILEMAP_H
#define PROJECTNAME_TILEMAP_H


#include "types.h"


EMPTY_STRUCT(Tilemap);
EMPTY_STRUCT(TilemapPack);


TilemapPack* load_tilemap_pack(const str path);
void dispose_tilemap_pack(TilemapPack* pack);
void dispose_tilemap(Tilemap* tmap);

Tilemap* tilemap_pack_get_tilemap(TilemapPack* pack, u16 index);
u16 tilemap_pack_get_tilemap_count(TilemapPack* pack);

u8 tilemap_get_tile(Tilemap* tilemap, i16 x, i16 y, u8 def);
void tilemap_get_size(Tilemap* tilemap, u16* width, u16* height);

// Note: buffer here must be already allocated
void tilemap_copy(Tilemap* tilemap, u8* buffer);


#endif // PROJECTNAME_TILEMAP_H
