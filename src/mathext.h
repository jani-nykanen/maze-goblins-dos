#ifndef PROJECTNAME_MATH_EXT_H
#define PROJECTNAME_MATH_EXT_H


#include "types.h"


i16 min_i16(i16 x, i16 y);
i16 max_i16(i16 x, i16 y);

i16 clamp_i16(i16 x, i16 min, i16 max);

u16 min_u16(u16 x, u16 y);
u16 max_u16(u16 x, u16 y);

i16 round_i16(i16 x, i16 d);

i16 neg_mod_i16(i16 n, i16 m);


#endif // PROJECTNAME_MATH_EXT_H
