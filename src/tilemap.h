#ifndef PROJECTNAME_TILEMAP_H
#define PROJECTNAME_TILEMAP_H


#include "types.h"


// TODO: Hide information

typedef struct {

    u8* data;
    u16 width;
    u16 height;

} Tilemap;


typedef struct {

    Tilemap** maps;
    u8 count;

} TilemapPack;


TilemapPack* load_tilemap_pack(const str path);
void dispose_tilemap_pack(TilemapPack* pack);

void dispose_tilemap(Tilemap* tmap);


#endif // PROJECTNAME_TILEMAP_H
