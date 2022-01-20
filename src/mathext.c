#include "mathext.h"


i16 min_i16(i16 x, i16 y) {

    return x < y ? x : y;
}


i16 max_i16(i16 x, i16 y) {

    return x >= y ? x : y;
}


u16 min_u16(u16 x, u16 y) {

    return x < y ? x : y;
}


u16 max_u16(u16 x, u16 y) {

    return x >= y ? x : y;
}


i16 round_i16(i16 x, i16 d) {

    return (x + (d - 1)) / d;
}
