#ifdef DEBUG_MODE
#include <stdio.h>
#include <gccore.h>
#endif

#include "lib_usb.h"
#include "vc.h"
#include "hooks.h"
#include "vc_device.h"

/**
 * @brief Entry point for the hack, replacing the call to `ramWipe` call in `systemSetupGameRAM`.
 *
 * @param pRAM Pointer to the N64 RAM virtual object.
 * @return bool true on success, false otherwise.
 */
INIT bool _start(Ram* pRAM) {
    if (!ramWipe(pRAM)) {
        return false;
    }

	if(hId==-1) hId = iosCreateHeap((void*)ios_heap_addr, USB_HEAPSIZE);

    serial_device_init();
    // "O","O","T","R" = 0x4F4F5452
    serial_device_object->key = 0x4F4F5452;
    n64_dram = pRAM->pBuffer;

    return true;
}
