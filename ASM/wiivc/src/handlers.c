#include "handlers.h"
#include "vc.h"
#include "vc_device.h"
#include "device_wii.h"
#include "lib_usb.h"
#include "serial_stream.h"

#define BUFFER_QUEUE_SIZE 16

uint8_t* serial_incoming_data_queue[BUFFER_QUEUE_SIZE];
uint32_t serial_incoming_header_queue[BUFFER_QUEUE_SIZE];
int incoming_queue_cursor = -1;
int active_queue_index = -1;
uint32_t incoming_data_position = 0;

/**
 * @brief Adds buffer pointer and associated data header to incoming data queue.
 *
 * Handles multiple messages potentially being received by the Wii before
 * the N64 has a chance to process a message.
 *
 * @param header Header with data type and buffer length.
 * @param buffer Pointer to the data.
 * @return bool true on success, false otherwise.
 */
bool queue_incoming_buffer(uint32_t header, uint8_t* buffer) {
    if (incoming_queue_cursor >= BUFFER_QUEUE_SIZE - 1) return false;
    incoming_queue_cursor++;
    serial_device_object->incoming_queue_cursor = incoming_queue_cursor;
    serial_incoming_data_queue[incoming_queue_cursor] = buffer;
    serial_incoming_header_queue[incoming_queue_cursor] = header;
    return true;
}

/**
 * @brief Removes an item from the incoming data queues.
 *
 * Assumes the data buffer referenced in the pointer list
 * has already been freed, and the active index is no longer in use!
 * This function is used instead of the current queue cursor
 * because a new message may come in while the N64 is still
 * processing another message.
 *
 * @param queue_index Index in the queue to remove.
 * @return none.
 */
void remove_queued_data_at_index(int queue_index) {
    if (queue_index >= 0) {
        // Shift data down and adjust cursor
        for (int i = queue_index; i < incoming_queue_cursor; i++) {
            serial_incoming_data_queue[i] = serial_incoming_data_queue[i + 1];
            serial_incoming_header_queue[i] = serial_incoming_header_queue[i + 1];
        }
        incoming_queue_cursor--;
        serial_device_object->incoming_queue_cursor = incoming_queue_cursor;
    }
}

/**
 * @brief Clears out the incoming data queue.
 *
 * If there's an error in the USB subsystem (e.g. device disconnected),
 * clear out the incoming data and free associated memory for the
 * data buffers
 *
 * @return none
 */
void purge_queue(void) {
    for (int i = incoming_queue_cursor; i >= 0; i--) {
        iosFree(hId, serial_incoming_data_queue[i]);
    }
    incoming_queue_cursor = -1;
    active_queue_index = -1;
    serial_device_object->incoming_queue_cursor = incoming_queue_cursor;
    serial_device_object->active_queue_index = active_queue_index;
}

/**
 * @brief Polls the incoming queue for data to process.
 *
 * @return u32 Data header if data is available, otherwise 0.
 */
uint32_t handle_poll() {
    // N64 is not currently reading anything
    if (incoming_queue_cursor >= 0 && active_queue_index < 0) {
        active_queue_index = incoming_queue_cursor;
        serial_device_object->active_queue_index = active_queue_index;
        return serial_incoming_header_queue[active_queue_index];
    // N64 is reading a buffer
    } else if (active_queue_index >= 0) {
        return serial_incoming_header_queue[active_queue_index];
    // No data available
    } else {
        return 0;
    }
}

/**
 * @brief Copies from an incoming data buffer to N64 memory.
 *
 * The N64 will set a RAM address to copy memory to prior
 * to triggering this handler. device->receiving is used
 * to trigger, so it is not set in the handler.
 *
 * @param device Emulator storage device connected to the N64.
 * @return none.
 */
void handle_read(SerialVirtualDevice* device) {
    device->ready = 0;
    device->busy = 1;

    if (active_queue_index >= 0) {
        uint32_t header = serial_incoming_header_queue[active_queue_index];
        uint8_t* buffer = serial_incoming_data_queue[active_queue_index];
        uint32_t incoming_size = USBHEADER_GETSIZE(header);
        uint32_t sent_size = incoming_size;
        if (incoming_size > 512)
            sent_size = 512;

        memcpy(device->receive_addr, buffer + incoming_data_position, sent_size);

        if (incoming_size > sent_size + incoming_data_position) {
            incoming_data_position += sent_size;
        } else {
            incoming_data_position = 0;
            iosFree(hId, buffer);
            remove_queued_data_at_index(active_queue_index);
            active_queue_index = -1;
            serial_device_object->active_queue_index = active_queue_index;
        }
    }

    device->ready = 1;
    device->busy = 0;
}

/**
 * @brief Sends data directly from N64 memory through the connected serial adapter.
 *
 * The N64 will set a RAM address to copy memory from as
 * well as the data header prior, which triggers this handler.
 * No need for intermediary buffers as the N64 freezes execution
 * until the message is sent.
 *
 * @param device Emulator storage device connected to the N64.
 * @return none.
 */
void handle_send(SerialVirtualDevice* device) {
    device->ready = 0;
    device->busy = 1;
    device->transmitting = 1;
    device_senddata_wii(serial_usb, device->transmit_header, device->transmit_addr);
    device->transmitting = 0;
    device->ready = 1;
    device->busy = 0;
}