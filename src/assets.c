#include "assets.h"
#include "system.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// Easier than dynamically allocate
// memory for each name entry separately
#define MAX_NAME_LENGTH 64


typedef struct {

    Bitmap** bitmaps;
    u8* names [MAX_NAME_LENGTH];
    u16 assetMax;

} _AssetCache;


static i16 find_index(_AssetCache* assets, const str name) {

    i16 i;

    for (i = 0; i < assets->assetMax; ++ i) {

        if (strcmp(assets->names[i], name) == 0) {

            return i;
        }
    }
    return -1;
}


AssetCache* new_asset_cache(u16 maxSize) {


    _AssetCache* assets = (_AssetCache*) calloc(1, sizeof(_AssetCache));
    if (assets == NULL) {

        m_memory_error();
        return NULL;
    }

    assets->assetMax = maxSize;

    assets->bitmaps = (Bitmap**) calloc(maxSize, sizeof(Bitmap*));
    if (assets->bitmaps == NULL) {

        m_memory_error();
        dispose_asset_cache((AssetCache*) assets);
        return NULL;
    }

    return (AssetCache*) assets;
}


void dispose_asset_cache(AssetCache* _assets) {

    _AssetCache* assets = (_AssetCache*) _assets;
    u16 i;

    if (assets == NULL)
        return;

    for (i = 0; i < assets->assetMax; ++ i) {
        
        dispose_bitmap(assets->bitmaps[i]);
    }
    m_free(assets->bitmaps);
    m_free(assets);
}


void asset_cache_store_bitmap(AssetCache* _assets, Bitmap* bitmap, const str name) {

    _AssetCache* assets = (_AssetCache*) _assets;

    i16 i;
    i16 index = -1;

    for (i = 0; i < assets->assetMax; ++ i) {

        if (assets->bitmaps[i] == NULL) {

            index = i;
            break;
        }
    }

    // No room
    if (index == -1) return;

    strcpy(assets->names[index], name);
    assets->bitmaps[index] = bitmap;
}


void asset_cache_remove_bitmap(AssetCache* _assets, const str name) {

    _AssetCache* assets = (_AssetCache*) _assets;

    i16 i = find_index(assets, name);
    if (i == -1) return;

    dispose_bitmap(assets->bitmaps[i]);
    assets->bitmaps[i] = NULL;
}


Bitmap* asset_cache_get_bitmap(AssetCache* _assets, const str name) {

    _AssetCache* assets = (_AssetCache*) _assets;

    i16 i = find_index(assets, name);
    if (i == -1) return NULL;

    // Note: this can be null if the bitmap with the same
    // name was destroyed
    return assets->bitmaps[i];
}
