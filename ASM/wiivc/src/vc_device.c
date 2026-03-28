// Most of the logic here is courtesy of homeboy
// https://github.com/PracticeROM/homeboy

#include <stdint.h>

#include "vc_device.h"

#include "vc.h"
#include "handlers.h"

// Global pointer to the start of N64 RAM
void* n64_dram = NULL;

// Virtual storage device for the serial buffers.
// See `serial_device_init` at the bottom of this file
// for details.
SerialVirtualDevice* serial_device_object = NULL;

bool serial_event(void* regs, int event, void* arg);

static _XL_OBJECTTYPE serial_class = {
    "USBSERIAL",
    sizeof(SerialVirtualDevice),
    0,
    serial_event,
};

// Virtual storage device get/put handlers

#define ADDR_OFFSET 0x08060000

bool get8(SerialVirtualDevice* device, uint32_t addr, uint8_t* dest) {
    addr -= ADDR_OFFSET;
    *dest = (uint8_t)device->regs[addr >> 2];

    return true;
}

bool get16(SerialVirtualDevice* device, uint32_t addr, uint16_t* dest) {
    addr -= ADDR_OFFSET;
    *dest = (uint16_t)device->regs[addr >> 2];

    return true;
}

bool get32(SerialVirtualDevice* device, uint32_t addr, uint32_t* dest) {
    addr -= ADDR_OFFSET;

    // request for receive_header
    if (addr == 0x10) {
        *dest = handle_poll();
        return true;
    }
    *dest = (uint32_t)device->regs[addr >> 2];

    return true;
}

bool get64(SerialVirtualDevice* device, uint32_t addr, uint64_t* dest) {
    addr -= ADDR_OFFSET;
    *dest = (uint64_t)device->regs[addr >> 2];

    return true;
}

bool put8(SerialVirtualDevice* device, uint32_t addr, uint8_t* src) {
    addr -= ADDR_OFFSET;
    device->regs[addr >> 2] = *src;

    return true;
}

bool put16(SerialVirtualDevice* device, uint32_t addr, uint16_t* src) {
    addr -= ADDR_OFFSET;
    device->regs[addr >> 2] = *src;

    return true;
}

bool put32(SerialVirtualDevice* device, uint32_t addr, uint32_t* src) {
    addr -= ADDR_OFFSET;

    device->regs[addr >> 2] = *src;

    // device->transmit_addr changed, translate from N64 address to Wii address
    if (addr == 0x04) {
        device->transmit_addr = (void*)((char*)n64_dram + (*src - 0x80000000));
    // device->transmit_header changed
    } else if (addr == 0x08) {
        handle_send(device);
    // device->receive_addr changed, translate from N64 address to Wii address
    } else if (addr == 0x0C) {
        device->receive_addr = (void*)((char*)n64_dram + (*src - 0x80000000));
    // device->status changed
    } else if (addr == 0x14) {
        if (device->receiving) {
            device->receiving = 0;
            handle_read(device);
        }
    }

    return true;
}

bool put64(SerialVirtualDevice* device, uint32_t addr, uint64_t* src) {
    addr -= ADDR_OFFSET;
    uint32_t* src32 = (uint32_t*)src;
    device->regs[addr >> 2] = src32[0];
    device->regs[(addr >> 2) + 1] = src32[1];

    return true;
}

/**
 * @brief Event handler for the serial virtual storage device
 *
 * Associate the get/put event handlers with the device.
 * 0x1002 is the event ID during emulator initialization
 * to set the get/put handlers for any storage device. Once
 * the handlers are set, no other events are important.
 * See the `xlObjectEvent` call in `cpuMakeDevice` in emulator/cpu.c
 * 
 * @return none.
 */
bool serial_event(void* regs, int event, void* arg) {
    (void)(regs); // Ignore unused parameter
    if (event == 0x1002) {
        cpuSetDevicePut(SYSTEM_CPU(gpSystem), arg, put8, put16, put32, put64);
        cpuSetDeviceGet(SYSTEM_CPU(gpSystem), arg, get8, get16, get32, get64);
    }
    return true;
}

/**
 * @brief Creates virtual storage device for interfacing between the N64 and serial buffers
 *
 * Create a virtual storage device monitoring memory
 * at 0xA0806XXXX. The same process is used by VC for core
 * N64 hardware as well as external devices like save paks.
 * VC definitions are in emulator/system.c, global `gaSystemDevice`,
 * and built in `systemCreateStorageDevice` in the same file. Save
 * targets are defined in `systemSetStorageDevice`.
 * Virtual devices have custom get/put event handlers (defined above
 * for this serial device) that can be used for communication between
 * the emulator and the emulated game.
 * 
 * @return none.
 */
void serial_device_init(void) {
    xlObjectMake((void**)&serial_device_object, NULL, &serial_class);
    cpuMapObject(SYSTEM_CPU(gpSystem), serial_device_object, 0x08060000, 0x08067FFF, 0);
}
