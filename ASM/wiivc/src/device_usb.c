/***************************************************************
                       device_usb.c

                Handles raw USB communication.
                Adapted from UNFloader PC client.
***************************************************************/

#ifdef DEBUG_MODE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#endif

#include "device_usb.h"
#include "lib_usb.h"
#include "vc.h"

/*********************************
              Macros
*********************************/

#define BULK_EP_OUT 0x02
#define BULK_EP_IN  0x81


/*********************************
         Global Variables
*********************************/

static uint8_t readbuffer[BUFFER_SIZE] ATTRIBUTE_ALIGN(32);
static uint8_t usbbuffer[PACKET_SIZE+2] ATTRIBUTE_ALIGN(32);
static uint32_t readbuffer_left = 0;
static uint32_t readbuffer_readoffset = 0;
static uint32_t readbuffer_copyoffset = 0;
static uint32_t readbuffer_statusoffsets[64];
static uint8_t copy_statusoffset = -1;

/*==============================
    device_usb_write
    Writes data to a USB device
    @param  The USB handle to use
    @param  The buffer to use
    @param  The size of the data
    @param  A pointer to store the number of bytes written
    @return The USB status
==============================*/

USBStatus device_usb_write(int32_t handle, void* buffer, uint16_t size, uint32_t* written)
{
    // Flush cache to RAM for USB DMA engine
    DCFlushRange(buffer, ALIGN(size, 32));

    uint32_t totalwritten = 0;
    s32 ret;

    // Keep writing until we've finished
    while (totalwritten < size)
    {
        uint16_t packet_request = size-totalwritten < PACKET_SIZE ? size-totalwritten : PACKET_SIZE;
        int retries = 3;
        while (retries > 0)
        {
            ret = USB_WriteBlkMsg(handle, BULK_EP_OUT, packet_request, (void *)buffer+totalwritten);
            if (ret > 0) break;

            // Clear possible stalls
            USB_ClearHalt(handle, BULK_EP_OUT);
            retries--;
        }
        if (ret == -666)
        {
            (*written) = totalwritten;
            return USB_DEVICE_NOT_FOUND;
        }
        else if (ret < 0)
        {
            (*written) = totalwritten;
            return USB_IO_ERROR;
        }
        totalwritten += ret;
        // Consider timeout handling here
    }
    (*written) = totalwritten;
    return USB_OK;
}


/*==============================
    device_usb_read
    Reads data from a USB device, blocking until finished
    @param  The USB handle to use
    @param  The buffer to read into
    @param  The size of the data to read
    @param  A pointer to store the number of bytes read
    @return The USB status
==============================*/

USBStatus device_usb_read(int32_t handle, void* buffer, uint16_t size, uint32_t* read)
{
    uint32_t readcount = size;
    s32 ret;

    // Check if the read buffer is full
    uint32_t new_buffer_cursor = readbuffer_copyoffset + readcount - readbuffer_left;
    if (new_buffer_cursor > BUFFER_SIZE)
    {
        return USB_INSUFFICIENT_RESOURCES;
    }

    // If we're being asked to read more data than we have in our buffer, wait for the USB to give us more
    s64 start_time = OSGetTime();
    while (readcount > readbuffer_left)
    {
        ret = device_usb_getqueuestatus(handle, NULL);
        if (ret < USB_OK)
        {
            return USB_IO_ERROR;
        }
        if (OSGetTime() - start_time > OSMillisecondsToTicks(100))
        {
            return USB_IO_TIMEOUT;
        }
    }

    // Copy the data
    memcpy(buffer, readbuffer+readbuffer_readoffset, readcount);
    // Detect if we had FTDI status bytes in the original packet
    // that would affect alignment padding.
    uint32_t status_offset = readbuffer_statusoffsets[0];
    if (status_offset >= readbuffer_readoffset && status_offset < readbuffer_readoffset + readcount) {
        for (int i = 0; i < copy_statusoffset; i++) {
            readbuffer_statusoffsets[i] = readbuffer_statusoffsets[i + 1];
        }
        copy_statusoffset--;
    }
    readbuffer_left -= readcount;
    // only apply to bytes read as the status bytes are already
    // excluded from the read buffer total
    (*read) = readcount;

    // If we have no data left to read, we can safely reset the buffer position
    if (readbuffer_left == 0)
    {
        readbuffer_readoffset = 0;
        readbuffer_copyoffset = 0;
    }
    else
        readbuffer_readoffset += readcount;
    return USB_OK;
}


/*==============================
    device_usb_getqueuestatus
    Checks how many bytes are in the rx buffer
    @param  The USB handle to use
    @param  A pointer to store the number of bytes in the queue
    @return The USB status
==============================*/

USBStatus device_usb_getqueuestatus(int32_t handle, uint32_t* bytesleft)
{
    int retries = 3;
    s32 ret;

    // Perform a USB read to see how much data is in the actual USB buffer
    uint16_t packet_request = BUFFER_SIZE-readbuffer_copyoffset < PACKET_SIZE ? BUFFER_SIZE-readbuffer_copyoffset : PACKET_SIZE;
    while (retries > 0)
    {
        ret = USB_ReadBlkMsg(handle, BULK_EP_IN, packet_request, usbbuffer);
        if (ret >= 0) break;

        // Clear possible stalls
        USB_ClearHalt(handle, BULK_EP_IN);
        retries--;
    }
    if (ret == -666)
        return USB_DEVICE_NOT_FOUND;
    else if (ret < 0)
    {
        return ret;
    }
    else if (ret < 4) // filter FTDI 2-byte status packets
    {
        if (bytesleft != NULL)
            (*bytesleft) = readbuffer_left;
        return USB_OK;
    }

    // Ensure the CPU sees fresh data from RAM
    DCInvalidateRange(usbbuffer, ret);

    // Copy from USB buffer to read buffer. Separate buffer used
    // as the read buffer copy offset may become misaligned from
    // 32 bytes, which will cause errors for the USB system.
    // FTDI includes a 2-byte status header on every USB packet
    // which should be stripped.
    copy_statusoffset++;
    readbuffer_statusoffsets[copy_statusoffset] = readbuffer_copyoffset;
    memcpy(readbuffer+readbuffer_copyoffset, usbbuffer + 2, ret - 2);

    // Add how much we have left to read
    readbuffer_left += ret - 2;
    readbuffer_copyoffset += ret - 2;

    // Done
    if (bytesleft != NULL)
        (*bytesleft) = readbuffer_left;
    return USB_OK;
}

void device_usb_purgequeue()
{
    readbuffer_left = 0;
    readbuffer_readoffset = 0;
    readbuffer_copyoffset = 0;
    copy_statusoffset = -1;
    memset(readbuffer_statusoffsets, 0, sizeof(readbuffer_statusoffsets));
}
