#include "types.h"


Point_i16 point_i16(i16 x, i16 y) {

    Point_i16 p;

    p.x = x;
    p.y = y;

    return p;
}


Point_i32 point_i32(i32 x, i32 y) {

    Point_i32 p;

    p.x = x;
    p.y = y;

    return p;
}


Rect_i16 rect_i16(i16 x, i16 y, i16 w, i16 h) {

    Rect_i16 r;

    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;

    return r;
}
