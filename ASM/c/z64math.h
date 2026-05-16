#ifndef Z64MATH_H
#define Z64MATH_H

#include <stdint.h>

typedef struct {
    float x, y, z;
} Vec3f; // size = 0x0C

typedef struct Vec3s {
    int16_t x, y, z;
} Vec3s; // size = 0x06

#endif