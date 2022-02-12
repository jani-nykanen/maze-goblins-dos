#ifndef PROJECTNAME_STORY_H
#define PROJECTNAME_STORY_H


#include "window.h"
#include "assets.h"


i16 init_story_scene(Window* window, AssetCache* assets, bool isEnding);
void dispose_story_scene();

void register_story_scene(Window* window);


#endif // PROJECTNAME_STORY_H
