#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>

#include "types.h"

//#if VC_VERSION == NACE
#define DEVICE_ADDRESS_INDEX_BITS 20
//#else
//#define DEVICE_ADDRESS_INDEX_BITS 16
//#endif

#define DEVICE_ADDRESS_OFFSET_BITS (32 - DEVICE_ADDRESS_INDEX_BITS)

typedef struct CpuBlock CpuBlock;
typedef int (*UnknownBlockCallback)(CpuBlock* pBlock, int bUnknown);

struct CpuBlock {
    /* 0x00 */ struct CpuBlock* pNext;
    /* 0x04 */ u32 nSize;
    /* 0x08 */ UnknownBlockCallback pfUnknown;
    /* 0x0C */ u32 nAddress0;
    /* 0x10 */ u32 nAddress1;
}; // size = 0x14

typedef int (*GetBlockFunc)(void* pObject, CpuBlock* pBlock);

typedef int (*Put8Func)(void* pObject, u32 nAddress, s8* pData);
typedef int (*Put16Func)(void* pObject, u32 nAddress, s16* pData);
typedef int (*Put32Func)(void* pObject, u32 nAddress, s32* pData);
typedef int (*Put64Func)(void* pObject, u32 nAddress, s64* pData);

typedef int (*Get8Func)(void* pObject, u32 nAddress, s8* pData);
typedef int (*Get16Func)(void* pObject, u32 nAddress, s16* pData);
typedef int (*Get32Func)(void* pObject, u32 nAddress, s32* pData);
typedef int (*Get64Func)(void* pObject, u32 nAddress, s64* pData);

typedef enum CpuExceptionCode {
    CEC_NONE = -1,
    CEC_INTERRUPT = 0,
    CEC_TLB_MODIFICATION = 1,
    CEC_TLB_LOAD = 2,
    CEC_TLB_STORE = 3,
    CEC_ADDRESS_LOAD = 4,
    CEC_ADDRESS_STORE = 5,
    CEC_BUS_INSTRUCTION = 6,
    CEC_BUS_DATA = 7,
    CEC_SYSCALL = 8,
    CEC_BREAK = 9,
    CEC_RESERVED = 10,
    CEC_COPROCESSOR = 11,
    CEC_OVERFLOW = 12,
    CEC_TRAP = 13,
    CEC_VCE_INSTRUCTION = 14,
    CEC_FLOAT = 15,
    CEC_RESERVED_16 = 16,
    CEC_RESERVED_17 = 17,
    CEC_RESERVED_18 = 18,
    CEC_RESERVED_19 = 19,
    CEC_RESERVED_20 = 20,
    CEC_RESERVED_21 = 21,
    CEC_RESERVED_22 = 22,
    CEC_WATCH = 23,
    CEC_RESERVED_24 = 24,
    CEC_RESERVED_25 = 25,
    CEC_RESERVED_26 = 26,
    CEC_RESERVED_27 = 27,
    CEC_RESERVED_28 = 28,
    CEC_RESERVED_29 = 29,
    CEC_RESERVED_30 = 30,
    CEC_VCE_DATA = 31,
    CEC_COUNT = 32,
} CpuExceptionCode;

typedef enum CpuMode {
    CM_NONE = -1,
    CM_USER = 0,
    CM_SUPER = 1,
    CM_KERNEL = 2,
} CpuMode;

typedef enum CpuSize {
    CS_NONE = -1,
    CS_32BIT = 0,
    CS_64BIT = 1,
} CpuSize;

typedef union CpuGpr {
    struct {
        /* 0x0 */ s8 _0s8;
        /* 0x1 */ s8 _1s8;
        /* 0x2 */ s8 _2s8;
        /* 0x3 */ s8 _3s8;
        /* 0x4 */ s8 _4s8;
        /* 0x5 */ s8 _5s8;
        /* 0x6 */ s8 _6s8;
        /* 0x7 */ s8 s8;
    };
    struct {
        /* 0x0 */ s16 _0s16;
        /* 0x2 */ s16 _1s16;
        /* 0x4 */ s16 _2s16;
        /* 0x6 */ s16 s16;
    };
    struct {
        /* 0x0 */ s32 _0s32;
        /* 0x4 */ s32 s32;
    };
    struct {
        /* 0x0 */ s64 s64;
    };
    struct {
        /* 0x0 */ u8 _0u8;
        /* 0x1 */ u8 _1u8;
        /* 0x2 */ u8 _2u8;
        /* 0x3 */ u8 _3u8;
        /* 0x4 */ u8 _4u8;
        /* 0x5 */ u8 _5u8;
        /* 0x6 */ u8 _6u8;
        /* 0x7 */ u8 u8;
    };
    struct {
        /* 0x0 */ u16 _0u16;
        /* 0x2 */ u16 _1u16;
        /* 0x4 */ u16 _2u16;
        /* 0x6 */ u16 u16;
    };
    struct {
        /* 0x0 */ u32 _0u32;
        /* 0x4 */ u32 u32;
    };
    struct {
        /* 0x0 */ u64 u64;
    };
} CpuGpr;

typedef union CpuFpr {
    struct {
        /* 0x0 */ f32 _0f32;
        /* 0x4 */ f32 f32;
    };
    struct {
        /* 0x0 */ f64 f64;
    };
    struct {
        /* 0x0 */ s32 _0s32;
        /* 0x4 */ s32 s32;
    };
    struct {
        /* 0x0 */ s64 s64;
    };
    struct {
        /* 0x0 */ u32 _0u32;
        /* 0x4 */ u32 u32;
    };
    struct {
        /* 0x0 */ u64 u64;
    };
} CpuFpr;

typedef struct CpuDevice {
    /* 0x00 */ s32 nType;
    /* 0x04 */ void* pObject;
    /* 0x08 */ s32 nOffsetAddress;
    /* 0x0C */ Get8Func pfGet8;
    /* 0x10 */ Get16Func pfGet16;
    /* 0x14 */ Get32Func pfGet32;
    /* 0x18 */ Get64Func pfGet64;
    /* 0x1C */ Put8Func pfPut8;
    /* 0x20 */ Put16Func pfPut16;
    /* 0x24 */ Put32Func pfPut32;
    /* 0x28 */ Put64Func pfPut64;
#if IS_GC
    /* 0x2C */ u32 nAddressPhysical0;
    /* 0x30 */ u32 nAddressPhysical1;
#elif IS_WII && IS_OOT
    /* 0x2C */ GetBlockFunc pfGetBlock;
    /* 0x30 */ u32 nAddressVirtual0;
    /* 0x34 */ u32 nAddressVirtual1;
    /* 0x38 */ u32 nAddressPhysical0;
    /* 0x3C */ u32 nAddressPhysical1;
#elif IS_WII && IS_MM
    /* 0x2C */ u32 nAddressPhysical;
    /* 0x30 */ u32 nSize;
#endif
} CpuDevice; // size = 0x40

typedef struct CpuJump {
    /* 0x0 */ s32 nOffsetHost;
    /* 0x4 */ s32 nAddressN64;
} CpuJump; // size = 0x8

typedef struct CpuCallerID {
    /* 0x0 */ s32 N64address;
    /* 0x4 */ s32 GCNaddress;
} CpuCallerID; // size = 0x8

typedef struct CpuFunction CpuFunction;

struct CpuFunction {
    /* 0x00 */ void* pnBase;
    /* 0x04 */ void* pfCode;
    /* 0x08 */ s32 nCountJump;
    /* 0x0C */ CpuJump* aJump;
    /* 0x10 */ s32 nAddress0;
    /* 0x14 */ s32 nAddress1;
    /* 0x18 */ CpuCallerID* block;
    /* 0x1C */ s32 callerID_total;
    /* 0x20 */ s32 callerID_flag;
    /* 0x24 */ u32 nChecksum;
    /* 0x28 */ s32 timeToLive;
    /* 0x2C */ s32 memory_size;
    /* 0x30 */ s32 heapID;
    /* 0x34 */ s32 heapWhere;
    /* 0x38 */ s32 treeheapWhere;
    /* 0x3C */ CpuFunction* prev;
    /* 0x40 */ CpuFunction* left;
    /* 0x44 */ CpuFunction* right;
}; // size = 0x48

typedef struct CpuTreeRoot {
    /* 0x00 */ u16 total;
    /* 0x04 */ s32 total_memory;
    /* 0x08 */ s32 root_address;
    /* 0x0C */ s32 start_range;
    /* 0x10 */ s32 end_range;
    /* 0x14 */ s32 cache_miss;
    /* 0x18 */ s32 cache[20];
    /* 0x68 */ CpuFunction* left;
    /* 0x6C */ CpuFunction* right;
    /* 0x70 */ s32 kill_limit;
    /* 0x74 */ s32 kill_number;
    /* 0x78 */ s32 side;
    /* 0x7C */ CpuFunction* restore;
    /* 0x80 */ s32 restore_side;
} CpuTreeRoot; // size = 0x84

typedef struct CpuAddress {
    /* 0x0 */ s32 nN64;
    /* 0x4 */ s32 nHost;
    /* 0x8 */ CpuFunction* pFunction;
} CpuAddress; // size = 0xC

typedef struct CpuCodeHack {
    /* 0x0 */ u32 nAddress;
    /* 0x4 */ u32 nOpcodeOld;
    /* 0x8 */ u32 nOpcodeNew;
} CpuCodeHack; // size = 0xC

// cpu_optimize
typedef struct CpuOptimize {
    /* 0x00 */ u32 validCheck;
    /* 0x04 */ u32 destGPR_check;
    /* 0x08 */ s32 destGPR;
    /* 0x0C */ s32 destGPR_mapping;
    /* 0x10 */ u32 destFPR_check;
    /* 0x14 */ s32 destFPR;
    /* 0x18 */ u32 addr_check;
    /* 0x1C */ s32 addr_last;
    /* 0x20 */ u32 checkType;
    /* 0x24 */ u32 checkNext;
} CpuOptimize; // size = 0x28

typedef struct Cpu Cpu;
typedef s32 (*CpuExecuteFunc)(Cpu* pCPU, s32 nCount, s32 nAddressN64, s32 nAddressGCN);

#if IS_GC

struct Cpu {
    /* 0x00000 */ s32 nMode;
    /* 0x00004 */ s32 nTick;
    /* 0x00008 */ void* pHost;
    /* 0x00010 */ s64 nLo;
    /* 0x00018 */ s64 nHi;
    /* 0x00020 */ s32 nCountAddress;
    /* 0x00024 */ s32 iDeviceDefault;
    /* 0x00028 */ u32 nPC;
    /* 0x0002C */ u32 nWaitPC;
    /* 0x00030 */ u32 nCallLast;
    /* 0x00034 */ CpuFunction* pFunctionLast;
    /* 0x00038 */ s32 nReturnAddrLast;
    /* 0x0003C */ s32 survivalTimer;
    /* 0x00040 */ CpuGpr aGPR[32];
    /* 0x00140 */ CpuFpr aFPR[32];
    /* 0x00240 */ u64 aTLB[48][5];
    /* 0x009C0 */ s32 anFCR[32];
    /* 0x00A40 */ s64 anCP0[32];
    /* 0x00B40 */ CpuExecuteFunc pfStep;
    /* 0x00B44 */ CpuExecuteFunc pfJump;
    /* 0x00B48 */ CpuExecuteFunc pfCall;
    /* 0x00B4C */ CpuExecuteFunc pfIdle;
    /* 0x00B50 */ CpuExecuteFunc pfRam;
    /* 0x00B54 */ CpuExecuteFunc pfRamF;
    /* 0x00B58 */ u32 nTickLast;
    /* 0x00B5C */ u32 nRetrace;
    /* 0x00B60 */ u32 nRetraceUsed;
    /* 0x00B64 */ CpuDevice* apDevice[256];
    /* 0x00F64 */ u8 aiDevice[65536];
    /* 0x10F64 */ void* gHeap1;
    /* 0x10F68 */ void* gHeap2;
    /* 0x10F6C */ u32 aHeap1Flag[192];
    /* 0x1126C */ u32 aHeap2Flag[13];
    /* 0x112A0 */ CpuTreeRoot* gTree;
    /* 0x112A4 */ CpuAddress aAddressCache[256];
    /* 0x11EA4 */ s32 nCountCodeHack;
    /* 0x11EA8 */ CpuCodeHack aCodeHack[32];
    /* 0x12028 */ s64 nTimeRetrace;
    /* 0x12030 */ OSAlarm alarmRetrace;
    /* 0x12058 */ u32 nFlagRAM;
    /* 0x1205C */ u32 nFlagCODE;
    /* 0x12060 */ u32 nCompileFlag;
    /* 0x12064 */ CpuOptimize nOptimize;
}; // size = 0x12090

#elif IS_WII && IS_OOT

typedef struct Cpu {
    /* 0x00000 */ s32 nMode;
    /* 0x00004 */ s32 nTick;
    /* 0x00008 */ s64 nLo;
    /* 0x00010 */ s64 nHi;
    /* 0x00018 */ s32 nCountAddress;
    /* 0x0001C */ s32 iDeviceDefault;
    /* 0x00020 */ u32 nPC;
    /* 0x00024 */ u32 nWaitPC;
    /* 0x00028 */ u32 nCallLast;
    /* 0x0002C */ CpuFunction* pFunctionLast;
    /* 0x00030 */ s32 nReturnAddrLast;
    /* 0x00034 */ s32 survivalTimer;
    /* 0x00038 */ u32 nTickLast;
    /* 0x0003C */ u32 nRetrace;
    /* 0x00040 */ u32 nRetraceUsed;
    /* 0x00048 */ CpuGpr aGPR[32];
    /* 0x00148 */ CpuFpr aFPR[32];
    /* 0x00248 */ u64 aTLB[48][5];
    /* 0x009C8 */ s32 anFCR[32];
    /* 0x00A48 */ s64 anCP0[32];
    /* 0x00B48 */ CpuExecuteFunc pfStep;
    /* 0x00B4C */ CpuExecuteFunc pfJump;
    /* 0x00B50 */ CpuExecuteFunc pfCall;
    /* 0x00B54 */ CpuExecuteFunc pfIdle;
    /* 0x00B58 */ CpuExecuteFunc pfRam;
    /* 0x00B5C */ CpuExecuteFunc pfRamF;
    /* 0x00B60 */ CpuDevice* apDevice[256];
    /* 0x00F60 */ u8 aiDevice[1 << DEVICE_ADDRESS_INDEX_BITS];
    /* 0x10F60 */ void* gHeap1;
    /* 0x10F64 */ void* gHeap2;
    /* 0x10F68 */ u32 aHeap1Flag[192];
    /* 0x11268 */ u32 aHeap2Flag[13];
    /* 0x1129C */ void* gHeapTree;
    /* 0x112A0 */ u32 aHeapTreeFlag[125];
    /* 0x11494 */ CpuTreeRoot* gTree;
    /* 0x11498 */ CpuAddress aAddressCache[256];
    /* 0x12098 */ s32 nCountCodeHack;
    /* 0x1209C */ CpuCodeHack aCodeHack[32];
    /* 0x1221C */ u32 nFlagRAM;
    /* 0x12220 */ u32 nFlagCODE;
    /* 0x12224 */ u32 nCompileFlag;
    /* 0x12228 */ s32 unk_12228[18];
    /* 0x12270 */ CpuOptimize nOptimize;
    /* 0x12298 */ s64 nTimeRetrace;
    /* 0x122A0 */ u8 pad[0x30];
} Cpu; // size = 0x122D0

#elif IS_WII && IS_MM

typedef struct Cpu {
    /* 0x00000 */ s32 nMode;
    /* 0x00004 */ char unk_0x04[0x14];
    /* 0x00018 */ void* pHost;
    /* 0x0001C */ char unk_0x1C[0x4];
    /* 0x00020 */ s64 nLo;
    /* 0x00028 */ s64 nHi;
    /* 0x00030 */ u32 nCountAddress;
    /* 0x00034 */ u8 iDeviceDefault;
    /* 0x00038 */ u32 nPC;
    /* 0x0003C */ u32 nWaitPC;
    /* 0x00040 */ char unk_0x3C[0xC];
    /* 0x0004C */ CpuFunction* pFunctionLast;
    /* 0x00050 */ s32 nReturnAddrLast;
    /* 0x00054 */ s32 survivalTimer;
    /* 0x00058 */ CpuGpr aGPR[32];
    /* 0x00158 */ CpuFpr aFPR[32];
    /* 0x00258 */ u64 aTLB[48][5];
    /* 0x009D8 */ s32 anFCR[32];
    /* 0x00A58 */ s64 anCP0[32];
    /* 0x00B58 */ CpuExecuteFunc pfStep;
    /* 0x00B5C */ CpuExecuteFunc pfJump;
    /* 0x00B60 */ CpuExecuteFunc pfCall;
    /* 0x00B64 */ CpuExecuteFunc pfIdle;
    /* 0x00B68 */ CpuExecuteFunc pfRam;
    /* 0x00B6C */ CpuExecuteFunc pfRamF;
    /* 0x00B70 */ s64 nTimeLast;
    /* 0x00B78 */ char unk_0xB78[8];
    /* 0x00B80 */ CpuDevice* apDevice[256];
    /* 0x00F80 */ u8 aiDevice[1 << DEVICE_ADDRESS_INDEX_BITS];
    /* 0x10F80 */ void* gHeap1;
    /* 0x10F84 */ void* gHeap2;
    /* 0x10F88 */ u32 aHeap1Flag[256];
    /* 0x11388 */ u32 aHeap2Flag[13];
    /* 0x113BC */ CpuTreeRoot* gHeapTree;
    /* 0x113C0 */ char unk_0x113C0[0xDC0];
    /* 0x12180 */ u32 known_regs;
    /* 0x12184 */ char unk_0x12184[8];
    /* 0x1218C */ u32 jr_is_ra;
    /* 0x12190 */ char unk_0x12190[0x18];
    /* 0x121A8 */ u32 prev_loadstore_base;
    /* 0x121AC */ char unk_0x121AC[0x14];
} Cpu; // size = 0x121C0

#endif

#endif
