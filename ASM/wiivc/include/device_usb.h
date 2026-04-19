#ifndef __DEVICE_USB_H
#define __DEVICE_USB_H

#include <stdint.h>


/*********************************
                 Macros
*********************************/

//#define DEBUG_MODE

#define BUFFER_SIZE 8*1024
#define PACKET_SIZE 64 //32*1024

#define USB_PURGE_RX  1
#define USB_PURGE_TX  2

#define USB_BITMODE_RESET          0x00
#define USB_BITMODE_ASYNC_BITBANG  0x01
#define USB_BITMODE_MPSSE          0x02
#define USB_BITMODE_SYNC_BITBANG   0x04
#define USB_BITMODE_MCU_HOST       0x08
#define USB_BITMODE_FAST_SERIAL    0x10
#define USB_BITMODE_CBUS_BITBANG   0x20
#define USB_BITMODE_SYNC_FIFO      0x40

// Only two USB ports, likely at most a hard drive
// or network adapter attached, no hubs.
// Increase size if there is someone out there daisy
// chaining USB devices on a Wii of all things.
// libogc supports a max of 32.
#define MAX_USB_DEVICES 2

#define ALIGN(s, align) (((uint32_t)(s) + ((align)-1)) & ~((align)-1))


/*********************************
                 Types
*********************************/

typedef enum {
    USB_OK_SPACER,  // USB_OK Provided by libogc
    USB_INVALID_HANDLE,
    USB_DEVICE_NOT_FOUND,
    USB_DEVICE_NOT_OPENED,
    USB_IO_ERROR,
    USB_INSUFFICIENT_RESOURCES,
    USB_INVALID_PARAMETER,
    USB_INVALID_BAUD_RATE,
    USB_DEVICE_NOT_OPENED_FOR_ERASE,
    USB_DEVICE_NOT_OPENED_FOR_WRITE,
    USB_FAILED_TO_WRITE_DEVICE,
    USB_EEPROM_READ_FAILED,
    USB_EEPROM_WRITE_FAILED,
    USB_EEPROM_ERASE_FAILED,
    USB_EEPROM_NOT_PRESENT,
    USB_EEPROM_NOT_PROGRAMMED,
    USB_INVALID_ARGS,
    USB_NOT_SUPPORTED,
    USB_OTHER_ERROR,
    USB_DEVICE_LIST_NOT_READY,
    USB_IO_TIMEOUT,
} USBStatus;


/*********************************
        Function Prototypes
*********************************/

USBStatus device_usb_write(int32_t handle, void* buffer, uint16_t size, uint32_t* written);
USBStatus device_usb_read(int32_t handle, void* buffer, uint16_t size, uint32_t* read);
USBStatus device_usb_getqueuestatus(int32_t handle, uint32_t* bytesleft);
void device_usb_purgequeue();

#endif