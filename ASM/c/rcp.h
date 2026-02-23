#ifndef ULTRA64_RCP_H
#define ULTRA64_RCP_H

#include "R4300.h"
#include "ultratypes.h"
#include "convert.h"

/**
 * Peripheral Interface (PI) Registers
 */
#define PI_BASE_REG         0x04600000

// PI DRAM address (R/W): [23:0] starting RDRAM address
#define PI_DRAM_ADDR_REG    (PI_BASE_REG + 0x00)

// PI pbus (cartridge) address (R/W): [31:0] starting AD16 address
#define PI_CART_ADDR_REG    (PI_BASE_REG + 0x04)

// PI status (R): [3] interrupt flag, [2] error, [1] IO busy, [0] DMA busy
//           (W): [1] clear intr, [0] reset controller (and abort current op)
#define PI_STATUS_REG       (PI_BASE_REG + 0x10)

/**
 * Common macros
 */
#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

#define IO_READ(addr)       (*(vu32*)PHYS_TO_K1(addr))
#define IO_WRITE(addr,data) (*(vu32*)PHYS_TO_K1(addr)=(u32)(data))

#endif

#endif