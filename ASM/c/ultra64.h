#ifndef ULTRA64_H
#define ULTRA64_H

#include "ultratypes.h"

#define OS_DCACHE_ROUNDUP_ADDR(x) (void *)(((((u32)(x)+0xf)/0x10)*0x10))
#define OS_DCACHE_ROUNDUP_SIZE(x) (u32)(((((u32)(x)+0xf)/0x10)*0x10))

extern u32 __osProbeTLB(void*);
extern void osWritebackDCache(void* vaddr, s32 nbytes);
extern void osInvalDCache(void* vaddr, s32 nbytes);
extern u32 osGetCount(void);

#endif