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
    u16 assetCount;
    u16 assetMax;

} _AssetCache;



AssetCache* new_asset_cache(u16 maxSize) {


    _AssetCache* assets = (_AssetCache*) calloc(1, sizeof(_AssetCache));
    if (assets == NULL) {

        m_memory_error();
        return NULL;
    }

    assets->assetMax = maxSize;
    assets->assetCount = 0;

    assets->bitmaps = (Bitmap**) calloc(maxSize, sizeof(Bitmap*));
    if (assets->bitmaps == NULL) {

        m_memory_error();
        dispose_asset_cache((AssetCache*) assets);
        return NULL;
    }

    return assets;
}


void dispose_asset_cache(AssetCache* _assets) {

    _AssetCache* assets = (_AssetCache*) _assets;
    u16 i;

    if (assets == NULL)
        return;

    for (i = 0; i < assets->assetCount; ++ i) {

        dispose_bitmap(assets->bitmaps[i]);
    }
    m_free(assets->bitmaps);
    m_free(assets);
}


void asset_cache_store_bitmap(AssetCache* _assets, Bitmap* bitmap, const str name) {

    _AssetCache* assets = (_AssetCache*) _assets;
}


void asset_cache_remove_bitmap(AssetCache* _assets, const str name) {

    _AssetCache* assets = (_AssetCache*) _assets;
}


Bitmap* asset_cache_get_bitmap(AssetCache* _assets, const str name) {

    _AssetCache* assets = (_AssetCache*) _assets;
}
