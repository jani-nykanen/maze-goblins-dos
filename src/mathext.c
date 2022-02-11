#include "mathext.h"

#include "sine.h"


i16 min_i16(i16 x, i16 y) {

    return x < y ? x : y;
}


i16 max_i16(i16 x, i16 y) {

    return x >= y ? x : y;
}


i16 clamp_i16(i16 x, i16 min, i16 max) {

    return min_i16(max, max_i16(x, min));
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


i16 neg_mod_i16(i16 m, i16 n) {

    return ((m % n) + n) % n;
}


i16 fixed_sin(i16 angle) {

    return SINE_TABLE[neg_mod_i16(angle, 360)];
}


i16 get_fixed_trig_precision() {

    return SINE_PRECISION;
}
