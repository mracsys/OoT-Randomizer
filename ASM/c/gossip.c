#include "gossip.h"
#include "usb.h"
#include "flashcart.h"

void send_hint_message(EnGs* this) {
    if (this->actor.text_id != 0x2053) {
        if (usb_getcart() != CART_NONE && flashcart_protocol_state == FLASHCART_PROTOCOL_STATE_MW) {
            uint8_t* hint_buffer = NULL;
            for (int i = 0; i < 40; i++) {
                uint16_t table_row_id = GOSSIP_HINT_DATA[i * 25] << 8 | GOSSIP_HINT_DATA[i * 25 + 1];
                if (table_row_id == this->actor.text_id) {
                    pending_gossip_hint = &GOSSIP_HINT_DATA[i * 25];
                    break;
                }
            }
        }
    }
}