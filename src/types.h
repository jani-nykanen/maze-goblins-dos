#ifndef PROJECTNAME_TYPES_H
#define PROJECTNAME_TYPES_H


#include <stdbool.h>


#define EMPTY_STRUCT(name) typedef struct name name;


typedef unsigned char u8;
typedef signed char i8;

typedef unsigned short u16;
typedef signed short i16;

// Should hold true with Watcom
typedef unsigned long u32;
typedef signed long i32;
// But just to be sure maybe use this?
typedef unsigned long ulong;


typedef const char* str;


typedef struct {

    i16 x;
    i16 y;

} Point_i16;


typedef struct {

    i32 x;
    i32 y;

} Point_i32;


typedef struct {

    i16 x;
    i16 y;
    i16 w;
    i16 h;

} Rect_i16;


Point_i16 point_i16(i16 x, i16 y);
Point_i32 point_i32(i32 x, i32 y);

Rect_i16 rect_i16(i16 x, i16 y, i16 w, i16 h);


#endif // PROJECTNAME_TYPES_H
