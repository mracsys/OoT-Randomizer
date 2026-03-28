#include <stdint.h>

#include "serial.h"

#include "flashcart.h"
#include "usb.h"

extern uint8_t SERIAL_ENABLE;
extern uint8_t SERIAL_RECEIVE;
extern uint8_t SERIAL_TRANSMIT;
extern uint32_t SERIAL_OUTGOING_ADDRESS;
extern uint32_t SERIAL_OUTGOING_HEADER;
extern uint8_t* SERIAL_RECEIVE_BUFFER;
extern uint8_t* SERIAL_TRANSMIT_BUFFER;

uint32_t receive_cursor = 0;
uint32_t current_receive_header = 0;

uint32_t serial_poll() {
    char is_cart = usb_getcart();
    if (SERIAL_ENABLE && is_cart == CART_NONE) {
        if (SERIAL_RECEIVE) {
            current_receive_header = (
                (SERIAL_RECEIVE_BUFFER[receive_cursor + 0] << 24) |
                (SERIAL_RECEIVE_BUFFER[receive_cursor + 1] << 16) |
                (SERIAL_RECEIVE_BUFFER[receive_cursor + 2] << 8) |
                (SERIAL_RECEIVE_BUFFER[receive_cursor + 3] << 0)
            );
            return current_receive_header;
        }
    }
    if (is_cart != CART_NONE) {
        return usb_poll();
    }
    return 0;
}

DeviceError serial_receive(uint32_t *header) {

}

void serial_skip(uint32_t nbytes) {
    char is_cart = usb_getcart();
    if (SERIAL_ENABLE && is_cart == CART_NONE) {
        uint32_t data_size = USBHEADER_GETSIZE(current_receive_header);
        if (nbytes) {}
    }
    if (is_cart != CART_NONE) {
        return usb_skip(nbytes);
    }
}

DeviceError serial_send() {

}

DeviceError serial_process() {
    char is_cart = usb_getcart();
    
    // 
    if (SERIAL_ENABLE && is_cart == CART_NONE) {
        if (SERIAL_RECEIVE) {
            uint32_t header;
            serial_receive(&header);
            SERIAL_RECEIVE = 0;
        }
    }

}
