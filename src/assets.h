#ifndef PROJECTNAME_ASSETS_H
#define PROJECTNAME_ASSETS_H


#include <bitmap.h>


EMPTY_STRUCT(AssetCache);


AssetCache* new_asset_cache(u16 maxSize);
void dispose_asset_cache(AssetCache* assets);

void asset_cache_store_bitmap(AssetCache* assets, Bitmap* bitmap, const str name);
void asset_cache_remove_bitmap(AssetCache* assets, const str name);
Bitmap* asset_cache_get_bitmap(AssetCache* assets, const str name);


#endif // PROJECTNAME_ASSETS_H
