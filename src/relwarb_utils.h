#ifndef RELWARB_UTILS_H
#define RELWARB_UTILS_H

#include "relwarb_defines.h"
#include "relwarb.h"

real32 Abs(real32 x);

bool32 Intersect(const RectangularShape & shape1, const RectangularShape & shape2);

// NOTE(Charly): Checks for equality between str1 and str2
//               Behaviour is undefined is one of the strings' length is less that length.
bool32 StrEqual(const char* str1, const char* str2, int length);

uint32 StrLength(const char* str);

#endif // RELWARB_UTILS_H