#ifndef PI_READ_WRITE
#define PI_READ_WRITE

#include "z64.h"

typedef struct OSDevMgr {
    /* 0x00 */ u32 active;
    /* 0x04 */ OSThread* thread;
    /* 0x08 */ OSMesgQueue* cmdQueue;
    /* 0x0C */ OSMesgQueue* evtQueue;
    /* 0x10 */ OSMesgQueue* acsQueue;
    /* 0x14 */ s32 (*dma)(s32, u32, void*, size_t);
    /* 0x18 */ s32 (*edma)(OSPiHandle*, s32, u32, void*, size_t);
} OSDevMgr; // size = 0x1C

extern OSDevMgr __osPiDevMgr;
extern OSMesgQueue* osPiGetCmdQueue(void);
extern s32 osSendMesg(OSMesgQueue* mq, OSMesg msg, s32 flag);
extern s32 osRecvMesg(OSMesgQueue* mq, OSMesg* msg, s32 flag);
extern s32 osJamMesg(OSMesgQueue* mq, OSMesg msg, s32 flag);
extern void osCreateMesgQueue(OSMesgQueue* mq, OSMesg* msg, s32 count);

s32 __osPiRawReadIo(u32 devAddr, u32* data);
s32 __osPiRawWriteIo(u32 devAddr, u32 data);
s32 osPiReadIo(u32 devAddr, u32* data);
s32 osPiWriteIo(u32 devAddr, u32 data);
s32 osPiStartDma(OSIoMesg* mb, s32 priority, s32 direction, u32 devAddr, void* dramAddr, u32 size, OSMesgQueue* mq);


#endif