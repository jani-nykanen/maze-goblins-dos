#include "window.h"
#include "system.h"
#include "game.h"

#include <stdio.h>


static void print_error() {

    printf("%s", system_get_error());
}


i16 main() {

    Window* window;

    if (init_system()) {

        print_error();
        return 1;
    }

    window = new_window(320, 200, "Game", 1);
    if (window == NULL) {

        print_error();
        return 1;
    }

    if (init_game_scene() != 0) {

        dispose_window(window);
        print_error();
        return 1;
    }
    
    register_game_scene(window);
    window_make_active(window);

    dispose_game_scene();
    dispose_window(window);

    return 0;
}
