/**
 * serial_stream.c
 * State machine to manage connected USB serial adapters
 */

#include "serial_stream.h"
#include "vc.h"
#include "device_wii.h"
#include "lib_usb.h"
#include "lib_ipc.h"
#include "handlers.h"
#include "types.h"
#include "vc_device.h"

typedef enum {
    SERIAL_INIT,
    SERIAL_SETUP,
    SERIAL_OPENING,
    SERIAL_IO,
    SERIAL_CLOSING,
    SERIAL_CLEANUP,
    SERIAL_SKIP,
} serial_state;

WiiSerialDevice* serial_usb;
serial_state state = SERIAL_INIT;
u8 attempts = 0;

/**
 * @brief Initializes global variables for the USB stack
 *
 * @return SERIAL_SETUP on success, SERIAL_INIT otherwise.
 */
serial_state serial_init(void) {
    if (USB_Initialize() != IPC_OK)
        return SERIAL_INIT;

    serial_usb = device_initialize_wii();
    if (serial_usb == NULL)
        return SERIAL_INIT;

    attempts = 0;
    return SERIAL_SETUP;
}

/**
 * @brief Looks for a compatible connected serial adapter
 *
 * @return SERIAL_OPENING on success, SERIAL_SETUP otherwise.
 */
serial_state serial_setup(void) {
    if (device_test_wii(serial_usb) != DEVICEERR_OK)
        return SERIAL_SETUP;

    attempts = 0;
    return SERIAL_OPENING;
}

/**
 * @brief Initializes the connected serial adapter
 *
 * @return SERIAL_IO on success, SERIAL_OPENING for 200 failed attempts, SERIAL_SETUP otherwise.
 */
serial_state serial_open(void) {
    if (device_open_wii(serial_usb) != DEVICEERR_OK) {
        attempts += 1;
        if (attempts > 200) {
            attempts = 0;
            return SERIAL_SETUP;
        }
        return SERIAL_OPENING;
    }

    attempts = 0;
    serial_device_object->ready = 1;
    return SERIAL_IO;
}

/**
 * @brief Polls the serial adapter for incoming data and saves to the buffer
 *
 * @return SERIAL_IO on success, SERIAL_SETUP otherwise.
 */
serial_state serial_poll(void) {
    uint32_t header = 0;
    uint8_t* buffer = NULL;
    if (device_receivedata_wii(serial_usb, &header, &buffer) == DEVICEERR_NODEVICES) {
        // device disconnected, free buffers, mark as not ready,
        // and go back to searching for a device
        serial_device_object->ready = 0;
        serial_device_object->reset = 1;
        purge_queue();
        return SERIAL_SETUP;
    }

    if (header != 0) {
        bool queued = queue_incoming_buffer(header, buffer);
        // queue is full, discard incoming data
        if (!queued)
            iosFree(hId, buffer);
    }

    return SERIAL_IO;
}

/**
 * @brief Closes the serial adapter
 *
 * @return SERIAL_CLEANUP always.
 */
serial_state serial_close(void) {
    device_close_wii(serial_usb);
    return SERIAL_CLEANUP;
}

/**
 * @brief Frees memory used by the USB stack
 *
 * @return SERIAL_SKIP always.
 */
serial_state serial_cleanup(void) {
    device_deinitialize_wii(serial_usb);
    USB_Deinitialize();
    return SERIAL_SKIP;
}

/**
 * @brief Main loop controlling serial adapter access
 *
 * @return bool true always.
 */
bool serial_stream(void) {
    switch(state) {
        case SERIAL_INIT:
            state = serial_init();
            break;
        case SERIAL_SETUP:
            state = serial_setup();
            break;
        case SERIAL_OPENING:
            state = serial_open();
            break;
        case SERIAL_IO:
            state = serial_poll();
            break;
        case SERIAL_CLOSING:
            state = serial_close();
            break;
        case SERIAL_CLEANUP:
            state = serial_cleanup();
            break;
        case SERIAL_SKIP:
        default:
            break;
    }

    return true;
}