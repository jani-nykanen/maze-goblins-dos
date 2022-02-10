#include "window.h"
#include "system.h"
#include "game.h"
#include "assets.h"
#include "title.h"

#include <stdio.h>


#include "loading.h"


static void print_error() {

    printf("%s", system_get_error());
}


static Bitmap* create_loading_bitmap() {

    return create_monochrome_bitmap_from_data(
        IMAGE_WIDTH, IMAGE_HEIGHT, 
        (char*) IMAGE_DATA);
}


static i16 load_global_assets(AssetCache* assets) {

    Bitmap* bmpFont1;
    Bitmap* bmpFont2;
    
    bmpFont1 = load_bitmap("FONT.BIN");
    if (bmpFont1 == NULL) {

        return 1;
    }

    bmpFont2 = load_bitmap("FONT2.BIN");
    if (bmpFont2 == NULL) {

        return 1;
    }

    asset_cache_store_bitmap(assets, bmpFont1, "font_white");
    asset_cache_store_bitmap(assets, bmpFont2, "font_yellow");

    return 0;
}


i16 main() {

    Window* window;
    Bitmap* loadingBitmap;
    AssetCache* assets;

    if (init_system()) {

        print_error();
        return 1;
    }

    loadingBitmap = create_loading_bitmap();
    if (loadingBitmap == NULL) {

        print_error();
        return 1;
    }

    assets = new_asset_cache(16);
    if (assets == NULL) {

        dispose_bitmap(loadingBitmap);

        print_error();
        return 1;
    }

    window = new_window(320, 200, "Game", 1);
    if (window == NULL) {

        dispose_window(NULL);
        dispose_bitmap(loadingBitmap);

        print_error();
        return 1;
    }
    window_bind_loading_bitmap(window, loadingBitmap);

    window_draw_loading_screen(window);
    if (load_global_assets(assets) == 1) {

        dispose_window(window);
        dispose_bitmap(loadingBitmap);

        print_error();
        return 1;
    }

    if (init_title_screen_scene(window, assets) != 0) {

        dispose_window(window);
        dispose_bitmap(loadingBitmap);
        dispose_asset_cache(assets);

        print_error();
        return 1;
    }
    
    register_title_screen_scene(window);
    window_make_active(window);

    dispose_bitmap(loadingBitmap);
    dispose_asset_cache(assets);

    // We can dispose them all here, since if they
    // are NULL already, nothing happens
    dispose_title_screen_scene();
    dispose_game_scene();

    dispose_window(window);

    return 0;
}
