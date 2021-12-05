#ifndef PROJECTNAME_WINDOW_H
#define PROJECTNAME_WINDOW_H


#include "canvas.h"


// A virtual window, to make SDL2 etc. ports
// possible
EMPTY_STRUCT(Window);


typedef i16 (*UpdateCallback) (i16);
typedef void (*RedrawCallback) (Canvas*);


// All the parameters are ignored when compiled to DOS
Window* new_window(u16 width, u16 height, str caption, i16 frameSkip);
void dispose_window(Window* window);

void window_register_callback_functions(Window* window,
    UpdateCallback update,
    RedrawCallback redraw);
void window_make_active(Window* window);


#endif // PROJECTNAME_WINDOW_H
