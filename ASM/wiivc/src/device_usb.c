/***************************************************************
                       device_usb.c

                Handles raw USB communication.
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
static uint32_t readbuffer_left = 0;
static uint32_t readbuffer_readoffset = 0;
static uint32_t readbuffer_copyoffset = 0;


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
    // usleep(100000);

    uint32_t totalwritten = 0;
    s32 ret;

    // uint8_t *buf = (uint8_t *)buffer;
    // for (uint32_t i = 0; i < size; i++) {
    //     printf("raw[%d] = 0x%02X '%c'\n", i, buf[i], buf[i] >= 0x20 && buf[i] < 0x7F ? buf[i] : '.');
    // }

    // Keep writing until we've finished
    while (totalwritten < size)
    {
        uint16_t packet_request = size-totalwritten < PACKET_SIZE ? size-totalwritten : PACKET_SIZE;
        int retries = 3;
        while (retries > 0)
        {
            // printf("Sending %d of %d bytes\n", packet_request, size);
            // printf("handle %d\n", handle);
            // printf("req %d\n", packet_request);
            // printf("buff %d\n", (uintptr_t)buffer+totalwritten);
            ret = USB_WriteBlkMsg(handle, BULK_EP_OUT, packet_request, (void *)buffer+totalwritten);
            if (ret > 0) break;
            // printf("Failed to send %d bytes: err %d\n", packet_request, ret);

            // Clear possible stalls
            USB_ClearHalt(handle, BULK_EP_OUT);
            // usleep(100000);
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
    @param  Flag to signal if the read call is part of reading a previously processed packet (ignore FTDI status bytes or not)
    @return The USB status
==============================*/

USBStatus device_usb_read(int32_t handle, void* buffer, uint16_t size, uint32_t* read, USBPacketFlag continue_packet)
{
    uint32_t readcount = size;
    s32 ret;
    uint32_t ftdi_bytes = 2;

    if (continue_packet == PACKET_CONTINUE)
        ftdi_bytes = 0;

    // Check if the read buffer is full
    uint32_t new_buffer_cursor = readbuffer_copyoffset + readcount - readbuffer_left;
    if (new_buffer_cursor > BUFFER_SIZE)
    {
        #ifdef DEBUG_MODE
        printf("%d > buffer size %d\n", new_buffer_cursor, BUFFER_SIZE);
        printf("readbuffer_copy %d\n", readbuffer_copyoffset);
        printf("readcount %d\n", readcount);
        printf("readbuffer_left %d\n", readbuffer_left);
        #endif
        return USB_INSUFFICIENT_RESOURCES;
    }

    // If we're being asked to read more data than we have in our buffer, wait for the USB to give us more
    // Account for the 2-byte FTDI status bytes
    while (readcount + ftdi_bytes > readbuffer_left)
    {
        ret = device_usb_getqueuestatus(handle, NULL);
        if (ret < USB_OK)
        {
            #ifdef DEBUG_MODE
            printf("Failed to add data to buffer: %d\n", ret);
            printf("Need %d, have %d\n", readcount + ftdi_bytes, readbuffer_left);
            #endif
            return USB_IO_ERROR;
        }
        if (ret > 0)
            break;
    }

    // Copy the data
    // Ignore the 2-byte FTDI status bytes
    if (readcount > readbuffer_left - ftdi_bytes)
        readcount = readbuffer_left - ftdi_bytes;
    memcpy(buffer, readbuffer+readbuffer_readoffset+ftdi_bytes, readcount);
    // for (uint32_t i = readbuffer_readoffset; i < readbuffer_readoffset+readbuffer_left; i++) {
    //     printf("raw[%d] = 0x%02X '%c'\n", i, readbuffer[i], readbuffer[i] >= 0x20 && readbuffer[i] < 0x7F ? readbuffer[i] : '.');
    // }
    //printf("readbuffer_left before: %d\n", readbuffer_left);
    readbuffer_left -= (readcount + ftdi_bytes);
    //printf("readbuffer_left after: %d\n", readbuffer_left);
    (*read) = readcount + ftdi_bytes;

    // If we have no data left to read, we can safetly reset the buffer position
    if (readbuffer_left == 0)
    {
        readbuffer_readoffset = 0;
        readbuffer_copyoffset = 0;
    }
    else
        readbuffer_readoffset += (readcount + ftdi_bytes);
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
    uint16_t packet_request = BUFFER_SIZE-readbuffer_copyoffset < PACKET_SIZE+2 ? BUFFER_SIZE-readbuffer_copyoffset : PACKET_SIZE+2;
    while (retries > 0)
    {
        ret = USB_ReadBlkMsg(handle, BULK_EP_IN, packet_request, readbuffer+readbuffer_copyoffset);
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
            (*bytesleft) = 0;
        // usleep(100000);
        return USB_OK;
    }

    // Ensure the CPU sees fresh data from RAM
    DCInvalidateRange(readbuffer+readbuffer_copyoffset, packet_request);
    // usleep(100000);

    // Add how much we have left to read
    //printf("readbuffer_queue before: %d\n", readbuffer_left);
    readbuffer_left += ret;
    //printf("readbuffer_queue after: %d\n", readbuffer_left);
    readbuffer_copyoffset += ret;

    // Done
    if (bytesleft != NULL)
        (*bytesleft) = readbuffer_left;
    return USB_OK;
}
