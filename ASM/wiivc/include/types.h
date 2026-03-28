#ifndef __TYPES_H
#define __TYPES_H

// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: Copyright fincs, devkitPro
#pragma once
#include <stdint.h>
#include <stddef.h>

/*! @name Integer types
	@brief Traditional shorthand form of integer types defined in `stdint.h`.
	@{
*/

typedef uint8_t  u8;       //!<  8-bit unsigned integer.
typedef uint16_t u16;      //!< 16-bit unsigned integer.
typedef uint32_t u32;      //!< 32-bit unsigned integer.
typedef uint64_t u64;      //!< 64-bit unsigned integer.
typedef uintptr_t uptr;    //!< Pointer-sized unsigned integer.

typedef int8_t  s8;        //!<  8-bit signed integer.
typedef int16_t s16;       //!< 16-bit signed integer.
typedef int32_t s32;       //!< 32-bit signed integer.
typedef int64_t s64;       //!< 64-bit signed integer.
typedef intptr_t sptr;     //!< Pointer-sized signed integer.

typedef volatile u8  vu8;  //!<  8-bit volatile unsigned integer.
typedef volatile u16 vu16; //!< 16-bit volatile unsigned integer.
typedef volatile u32 vu32; //!< 32-bit volatile unsigned integer.
typedef volatile u64 vu64; //!< 64-bit volatile unsigned integer.
typedef volatile uptr vuptr; //!< Pointer-sized volatile unsigned integer.

typedef volatile s8  vs8;  //!<  8-bit volatile signed integer.
typedef volatile s16 vs16; //!< 16-bit volatile signed integer.
typedef volatile s32 vs32; //!< 32-bit volatile signed integer.
typedef volatile s64 vs64; //!< 64-bit volatile signed integer.
typedef volatile sptr vsptr; //!< Pointer-sized volatile signed integer.

typedef float f32;
typedef double f64;

// Ensure bool is 4 bytes to match decomp
typedef int bool;

#define true  1
#define false 0

#endif