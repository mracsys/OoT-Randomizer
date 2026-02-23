#ifndef ULTRA64_R4300_H
#define ULTRA64_R4300_H

#ifdef _LANGUAGE_C
#include "ultratypes.h"
#define U32(x) ((u32)x)
#define C_REG(x) (x)
#else
#define U32(x) (x)
#define C_REG(x) $x
#endif

// Segment base addresses and sizes
#define K0BASE      0x80000000
#define K1BASE      0xA0000000
#define K2BASE      0xC0000000

// Address conversion macros
#define K0_TO_PHYS(x)       (U32(x) & 0x1FFFFFFF)  // kseg0 to physical
#define K1_TO_PHYS(x)       (U32(x) & 0x1FFFFFFF)  // kseg1 to physical
#define PHYS_TO_K1(x)       (U32(x) | 0xA0000000)  // physical to kseg1

// Address predicates
#define IS_KSEG0(x)         (U32(x) >= K0BASE && U32(x) < K1BASE)
#define IS_KSEG1(x)         (U32(x) >= K1BASE && U32(x) < K2BASE)

#endif