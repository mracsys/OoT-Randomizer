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

bool queue_incoming_buffer(uint32_t header, uint8_t* buffer) {
    if (incoming_queue_cursor >= BUFFER_QUEUE_SIZE - 1) return false;
    incoming_queue_cursor++;
    serial_incoming_data_queue[incoming_queue_cursor] = buffer;
    serial_incoming_header_queue[incoming_queue_cursor] = header;
    return true;
}

// Assumes the data buffer referenced in the pointer list
// has already been freed!
void remove_queued_data_at_index(int queue_index) {
    if (queue_index >= 0) {
        for (int i = queue_index; i < incoming_queue_cursor; i++) {
            serial_incoming_data_queue[i] = serial_incoming_data_queue[i + 1];
            serial_incoming_header_queue[i] = serial_incoming_header_queue[i + 1];
        }
        incoming_queue_cursor--;
    }
}

void purge_queue(void) {
    for (int i = incoming_queue_cursor; i >= 0; i--) {
        iosFree(hId, serial_incoming_data_queue[i]);
    }
    incoming_queue_cursor = -1;
    active_queue_index = -1;
}

uint32_t handle_poll() {
    if (incoming_queue_cursor >= 0 && active_queue_index < 0) {
        active_queue_index = incoming_queue_cursor;
        return serial_incoming_header_queue[active_queue_index];
    } else if (active_queue_index >= 0) {
        return serial_incoming_header_queue[active_queue_index];
    } else {
        return 0;
    }
}

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
        }
    }

    device->ready = 1;
    device->busy = 0;
}

void handle_send(SerialVirtualDevice* device) {
    device->ready = 0;
    device->busy = 1;
    device->transmitting = 1;
    device_senddata_wii(serial_usb, device->transmit_header, device->transmit_addr);
    device->transmitting = 0;
    device->ready = 1;
    device->busy = 0;
}