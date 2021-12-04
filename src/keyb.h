#ifndef PROJECTNAME_KEYBOARD_H
#define PROJECTNAME_KEYBOARD_H


#include "keycode.h"
#include "types.h"


typedef enum {

    STATE_UP = 0,
    STATE_RELEASED = 2,

    STATE_DOWN = 1,
    STATE_PRESSED = 3,
    
    // Used with '&' operator
    STATE_DOWN_OR_PRESSED = 1,

} State;


void init_keyboard_listener();
void reset_keyboard_listener();

void keyboard_update();
bool keyboard_any_pressed();

State keyboard_get_normal_key(u8 key);
State keyboard_get_extended_key(u8 key);


#endif // PROJECTNAME_KEYBOARD_H
