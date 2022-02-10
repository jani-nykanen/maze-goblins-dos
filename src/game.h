#ifndef PROJECTNAME_GAME_H
#define PROJECTNAME_GAME_H


#include "window.h"
#include "assets.h"


i16 init_game_scene(Window* window, AssetCache* assets, u16 startIndex);
void dispose_game_scene();

void register_game_scene(Window* window);


#endif // PROJECTNAME_GAME_H
