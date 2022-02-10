#ifndef PROJECTNAME_TITLESCREEN_H
#define PROJECTNAME_TITLESCREEN_H


#include "window.h"
#include "assets.h"


i16 init_title_screen_scene(Window* window, AssetCache* assets);
void dispose_title_screen_scene();

void register_title_screen_scene(Window* window);


#endif // PROJECTNAME_TITLESCREEN_H
