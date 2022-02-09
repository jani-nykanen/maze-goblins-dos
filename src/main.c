#include "window.h"
#include "system.h"
#include "game.h"

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


i16 main() {

    Window* window;
    Bitmap* loadingBitmap;

    if (init_system()) {

        print_error();
        return 1;
    }

    loadingBitmap = create_loading_bitmap();
    if (loadingBitmap == NULL) {

        print_error();
        return 1;
    }

    window = new_window(320, 200, "Game", 1);
    if (window == NULL) {

        dispose_window(NULL);
        print_error();
        return 1;
    }
    window_bind_loading_bitmap(window, loadingBitmap);

    if (init_game_scene(window) != 0) {

        dispose_window(window);
        print_error();
        return 1;
    }
    
    register_game_scene(window);
    window_make_active(window);

    dispose_bitmap(loadingBitmap);
    dispose_game_scene();
    dispose_window(window);

    return 0;
}
