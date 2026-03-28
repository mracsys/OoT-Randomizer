/***************************************************************
                       device_wii.c

                Handles Wii USB communication.
                Adapted from UNFloader PC client.
***************************************************************/

#ifdef DEBUG_MODE
#include <stdio.h>
#include <stdlib.h>
#endif

#include "device_wii.h"
#include "lib_usb.h"
#include "lib_ipc.h"
#include "vc.h"
#include "device_usb.h"


/*********************************
              Macros
*********************************/

#define U16(x) ((uint16_t)((x)[0] << 8 | (x)[1]))
#define U32(x) ((uint32_t)((x)[0] << 24 | (x)[1] << 16 | (x)[2] << 8 | (x)[3]))


/*==============================
    device_initialize_wii
    Initializes and allocates the device data structures
==============================*/

WiiSerialDevice* device_initialize_wii()
{
    WiiSerialDevice* serial = (WiiSerialDevice*) iosAlloc(hId, sizeof(WiiSerialDevice));
    if (serial == NULL)
        return NULL;
    memset(serial, 0, sizeof(WiiSerialDevice));
    return serial;
}


/*==============================
    device_deinitialize_wii
    Deinitializes and frees the device data structures
==============================*/

void device_deinitialize_wii(WiiSerialDevice* serial)
{
    iosFree(hId, serial);
}


/*==============================
    device_test_wii
    Attempts to find USB serial devices
    @param  A pointer to the serial context
    @return DEVICEERR_OK if the device is a serial adapter (currently only supports FTDI FT232RL, FT232H, FT230X),
            DEVICEERR_NOTCART if it isn't,
            Any other device error if problems ocurred
==============================*/

DeviceError device_test_wii(WiiSerialDevice *serial)
{
    // Clear existing adapter data if it was disconnected
    memset(serial, 0, sizeof(WiiSerialDevice));

    // Get device info list
    uint8_t device_count;
    usb_device_entry device_info[MAX_USB_DEVICES];
    if (USB_GetDeviceList(device_info, MAX_USB_DEVICES, USB_SUBCLASS_NONE, &device_count) != USB_OK)
        return DEVICEERR_USBBUSY;

    // Check if the device exists
    if (device_count == 0)
        return DEVICEERR_NODEVICES;

    // Search the devices
    for (uint8_t i = 0; i < device_count; i++)
    {
        // Look for FTDI chipsets
        if (device_info[i].vid == USB_VID_FTDI)
        {
            if (device_info[i].pid == USB_PID_FT232RL || device_info[i].pid == USB_PID_FT232H || device_info[i].pid == USB_PID_FT230X)
            {
                serial->device_id = device_info[i].device_id;
                serial->vid = device_info[i].vid;
                serial->pid = device_info[i].pid;
                serial->base_clock = USB_CLOCK_FT232RL;
                serial->handle = 0;
                serial->bytes_read = 0;
                serial->bytes_written = 0;
                return DEVICEERR_OK;
            }
        }
    }

    // Could not find the adapter
    return DEVICEERR_CARTFINDFAIL;
}

/*==============================
    device_open_wii
    Opens the USB pipe
    @param  A pointer to the serial context
    @return The device error, or OK
==============================*/

DeviceError device_open_wii(WiiSerialDevice *serial)
{
    // Open the device
    if (USB_OpenDevice(serial->device_id, serial->vid, serial->pid, &serial->handle) != USB_OK || serial->handle == 0)
        return DEVICEERR_CANTOPEN;

    // Reset the device
    if (USB_WriteCtrlMsg(serial->handle, 0x40, 0x00, 0x0000, 0x00, 0, NULL) < USB_OK) 
        return DEVICEERR_RESETFAIL;

    // Purge RX and TX buffers
    if (USB_WriteCtrlMsg(serial->handle, 0x40, 0x00, 0x0001, 0x00, 0, NULL) < USB_OK) // Purge RX
        return DEVICEERR_PURGEFAIL;
    if (USB_WriteCtrlMsg(serial->handle, 0x40, 0x00, 0x0002, 0x00, 0, NULL) < USB_OK) // Purge TX
        return DEVICEERR_PURGEFAIL;

    // Set baud rate (115200)
    // Divisor is calculated as 3000000 / baud
    if (USB_WriteCtrlMsg(serial->handle, 0x40, 0x03, serial->base_clock, 0x00, 0, NULL) < USB_OK)
        return DEVICEERR_BITMODEFAIL_RESET;

    // Set line properties: 8N1
    // wValue = data(8) | parity(0=none) | stop(0=1bit) = 0x0008
    if (USB_WriteCtrlMsg(serial->handle, 0x40, 0x04, 0x0008, 0x00, 0, NULL) < USB_OK)
        return DEVICEERR_BITMODEFAIL_RESET;

    // Disable flow control
    if (USB_WriteCtrlMsg(serial->handle, 0x40, 0x02, 0x0000, 0x00, 0, NULL) < USB_OK)
        return DEVICEERR_BITMODEFAIL_RESET;

    // Assert DTR and RTS — required for UART TX to become active
    if (USB_WriteCtrlMsg(serial->handle, 0x40, 0x01, 0x0303, 0x00, 0, NULL) < USB_OK)
        return DEVICEERR_SETDTRFAIL;

    // Set latency timer to 2ms (lowest possible, default 16ms)
    if (USB_WriteCtrlMsg(serial->handle, 0x40, 0x09, 0x0002, 0x00, 0, NULL) < USB_OK)
        return DEVICEERR_TIMEOUTSETFAIL;

    // Ok
    return DEVICEERR_OK;
}

/*==============================
    device_senddata_wii
    Sends data to the PC
    @param  A pointer to the serial context
    @param  Header for the data that is being sent
    @param  A buffer containing said data
    @return The device error, or OK
==============================*/

DeviceError device_senddata_wii(WiiSerialDevice *serial, uint32_t dataheader, byte *data)
{
    uint32_t size = USBHEADER_GETSIZE(dataheader);
    // Pad to alignment on 4-byte boundary + header
    uint32_t newsize = ALIGN(size, 4) + 4;
    byte*    datacopy = NULL;
    uint32_t bytes_done = 0;
    uint32_t bytes_left = newsize;

    // Put in the DMA header along with length and type information in the buffer

    // Copy the data onto a temp variable
    // 32-byte aligned for Wii USB DMA engine
    datacopy = (byte*) iosAllocAligned(hId, ALIGN(newsize, 32), 32);
    memset(datacopy, 0, newsize);
    if (datacopy == NULL)
        return DEVICEERR_MALLOCFAIL;
    memcpy(datacopy+4, data, size);
    datacopy[0] = (dataheader >> 24) & 0xFF;
    datacopy[1] = (dataheader >> 16) & 0xFF;
    datacopy[2] = (dataheader >> 8)  & 0xFF;
    datacopy[3] = dataheader & 0xFF;

    // Send the data in chunks
    USBStatus err;
    while (bytes_left > 0)
    {
        uint32_t bytes_do = MAX_PACKET_SIZE;
        if (bytes_left < MAX_PACKET_SIZE)
            bytes_do = bytes_left;
        err = device_usb_write(serial->handle, datacopy+bytes_done, bytes_do, &serial->bytes_written);
        if (err != USB_OK)
        {
            return DEVICEERR_WRITEFAIL;
        }
        bytes_left -= serial->bytes_written;
        bytes_done += serial->bytes_written;
    }

    // Free used up resources
    iosFree(hId, datacopy);
    #ifdef DEBUG_MODE
    printf("Sent %d bytes\n", bytes_done);
    #endif
    return DEVICEERR_OK;
}

/*==============================
    device_receivedata_wii
    Receives data from the PC
    @param  A pointer to the serial context
    @param  A pointer to an 32-bit value where
            the received data header will be
            stored.
    @param  A pointer to a byte buffer pointer
            where the data will be malloc'ed into.
    @return The device error, or OK
==============================*/

#ifdef DEBUG_MODE
static uint8_t poll_fail = 0;
#endif

DeviceError device_receivedata_wii(WiiSerialDevice *serial, uint32_t *dataheader, byte **buff)
{
    uint32_t size;
    uint32_t alignment = 4;

    // First, check if we have data to read
    USBStatus err = device_usb_getqueuestatus(serial->handle, &size);
    if (err == USB_DEVICE_NOT_FOUND)
    {
        return DEVICEERR_NODEVICES;
    }
    else if (err != USB_OK)
    {
        #ifdef DEBUG_MODE
        if (!poll_fail) printf("Poll failure: %d\n", err);
        poll_fail = 1;
        //printf("Poll failure: %d\n", err);
        #endif
        return DEVICEERR_POLLFAIL;
    }

    // If we do
    if (size > 0)
    {
        #ifdef DEBUG_MODE
        printf("Received %d bytes\n", size);
        #endif
        uint32_t dataread = 0;
        uint32_t totalread = 0;
        uint32_t offset = 4; // 4-byte header
        USBPacketFlag packet_status = PACKET_CONTINUE;
        byte     temp[4];

        // Get information about the incoming data and store it in dataheader
        err = device_usb_read(serial->handle, temp, 4, &serial->bytes_read, PACKET_START);
        if (err != USB_OK)
        {
            #ifdef DEBUG_MODE
            printf("Failed to read header: %d\n", err);
            #endif
            return DEVICEERR_READFAIL;
        }
        (*dataheader) = temp[0] << 24 | temp[1] << 16 | temp[2] << 8 | temp[3];
        totalread += serial->bytes_read;

        // Read the data into the buffer, in 64 byte chunks
        size = (*dataheader) & 0x00FFFFFF;
        (*buff) = (byte*)iosAlloc(hId, size);
        if ((*buff) == NULL)
            return DEVICEERR_MALLOCFAIL;

        // Do in 64 byte chunks (Wii USB buffer size)
        // Might be able to do 16KiB?
        while (dataread < size)
        {
            uint32_t readamount = size-dataread;
            if (readamount > MAX_PACKET_SIZE - offset)
                readamount = MAX_PACKET_SIZE - offset;
            err = device_usb_read(serial->handle, (*buff)+dataread, readamount, &serial->bytes_read, packet_status);
            if (err != USB_OK)
            {
                #ifdef DEBUG_MODE
                printf("Failed to read payload: %d\n", err);
                #endif
                return DEVICEERR_READFAIL;
            }
            totalread += serial->bytes_read;
            dataread += serial->bytes_read;
            offset = 0;
            packet_status = PACKET_START;
        }

        // Ensure 4 byte alignment by reading X amount of bytes needed
        if (totalread % alignment != 0)
        {
            byte* tempbuff = (byte*)iosAlloc(hId, alignment*sizeof(byte));
            int left = alignment - (totalread % alignment);
            err = device_usb_read(serial->handle, tempbuff, left, &serial->bytes_read, PACKET_CONTINUE);
            if (err != USB_OK)
            {
                #ifdef DEBUG_MODE
                printf("Failed to read padding: %d\n", err);
                #endif
                return DEVICEERR_READFAIL;
            }
            iosFree(hId, tempbuff);
        }
        #ifdef DEBUG_MODE
        printf("Received message of size %d\n", size);
        for (uint32_t i = 0; i < size; i++) {
            printf("read[%d] = 0x%02X '%c'\n", i, (*buff)[i], (*buff)[i] >= 0x20 && (*buff)[i] < 0x7F ? (*buff)[i] : '.');
        }
        #endif
    }
    else
    {
        (*dataheader) = 0;
        (*buff) = NULL;
    }

    // All's good
    return DEVICEERR_OK;
}

/*==============================
    device_close_wii
    Closes the USB pipe
    @param  A pointer to the serial context
    @return The device error, or OK
==============================*/

DeviceError device_close_wii(WiiSerialDevice *serial)
{
    if (USB_CloseDevice(&serial->handle) < USB_OK)
        return DEVICEERR_CLOSEFAIL;
    return DEVICEERR_OK;
}


/*==============================
    swap_endian
    Swaps the endianess of the data
    @param  The data to swap the endianess of
    @return The data with endianess swapped
==============================*/

uint32_t swap_endian(uint32_t val)
{
    return ((val << 24)) | 
           ((val << 8) & 0x00ff0000) |
           ((val >> 8) & 0x0000ff00) | 
           ((val >> 24));
}
