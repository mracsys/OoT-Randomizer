#include "pi_read_write.h"
#include "variables.h"
#include "convert.h"
#include "ultra64.h"
#include "rcp.h"
#include "R4300.h"
#include "ultratypes.h"

u32 osVirtualToPhysical(void* vaddr) {
    if (IS_KSEG0(vaddr)) {
        return K0_TO_PHYS(vaddr);
    }

    if (IS_KSEG1(vaddr)) {
        return K1_TO_PHYS(vaddr);
    }

    return __osProbeTLB(vaddr);
}


s32 __osPiRawReadIo(u32 devAddr, u32* data) {
    register u32 status;


    status = IO_READ(PI_STATUS_REG);
    while (status & (PI_STATUS_DMA_BUSY | PI_STATUS_IO_BUSY)) {
        status = IO_READ(PI_STATUS_REG);
    }

    *data = IO_READ((u32)osRomBase | devAddr);

    return 0;
}


s32 __osPiRawWriteIo(u32 devAddr, u32 data) {
    register u32 status;

    status = IO_READ(PI_STATUS_REG);
    while (status & (PI_STATUS_DMA_BUSY | PI_STATUS_IO_BUSY)) {
        status = IO_READ(PI_STATUS_REG);
    }

    IO_WRITE((u32)osRomBase | devAddr, data);

    return 0;
}


s32 osPiStartDma(OSIoMesg* mb, s32 priority, s32 direction, u32 devAddr, void* dramAddr, u32 size, OSMesgQueue* mq) {
    register s32 ret;
    if (!__osPiDevMgr.active) {
        return -1;
    }

    if (direction == OS_READ) {
        mb->hdr.type = OS_MESG_TYPE_DMAREAD;
    } else {
        mb->hdr.type = OS_MESG_TYPE_DMAWRITE;
    }

    mb->hdr.pri = priority;
    mb->hdr.retQueue = mq;
    mb->dramAddr = dramAddr;
    mb->devAddr = devAddr;
    mb->size = size;
    mb->piHandle = NULL;

    if (priority == OS_MESG_PRI_HIGH) {
        ret = osJamMesg(osPiGetCmdQueue(), (OSMesg)mb, OS_MESG_NOBLOCK);
    } else {
        ret = osSendMesg(osPiGetCmdQueue(), (OSMesg)mb, OS_MESG_NOBLOCK);
    }

    return ret;
}


s32 osPiReadIo(u32 devAddr, u32* data) {
    register s32 ret;

    z64_osPiGetAccess();
    ret = __osPiRawReadIo(devAddr, data);
    z64_osPiRelAccess();

    return ret;
}


s32 osPiWriteIo(u32 devAddr, u32 data) {
    register s32 ret;

    z64_osPiGetAccess();
    ret = __osPiRawWriteIo(devAddr, data);
    z64_osPiRelAccess();

    return ret;
}
