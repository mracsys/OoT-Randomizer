#include <stdbool.h>

#include "flashcart.h"

#include "get_items.h"
#include "item_upgrades.h"
#include "z64.h"
#include "usb.h"
#include "ultratypes.h"

uint8_t FLASHCART_READ_BUF[FLASHCART_BUFFER_SIZE];

// Intermediate write buffer to ensure we don't interfere with
// PC messages mid-frame before they can be read.
// Bypassed in specific instances where it is known the queue
// is empty, such as sending heartbeats or save file info.
uint8_t FLASHCART_WRITE_QUEUE_BUF[FLASHCART_BUFFER_SIZE];
uint8_t FLASHCART_WRITE_QUEUE_CURSOR = 0;
#define FLASHCART_CAN_QUEUE(x) (FLASHCART_WRITE_QUEUE_CURSOR + x + sizeof(int) * 2 < FLASHCART_BUFFER_SIZE ? 1 : 0)

uint8_t FLASHCART_PROTOCOL_VERSION = 1;
extern uint8_t CFG_RANDO_VERSION_MAJOR;
extern uint8_t CFG_RANDO_VERSION_MINOR;
extern uint8_t CFG_RANDO_VERSION_PATCH;
extern uint8_t CFG_RANDO_VERSION_BRANCH;
extern uint8_t CFG_RANDO_VERSION_SUPPLEMENTARY;
extern uint8_t PLAYER_ID;
extern uint8_t CFG_FILE_SELECT_HASH[5];
extern uint8_t MW_SEND_OWN_ITEMS;
extern uint8_t MW_PROGRESSIVE_ITEMS_ENABLE;
extern uint8_t PLAYER_NAMES[256][8];
extern mw_progressive_items_state_t MW_PROGRESSIVE_ITEMS_STATE[256];

uint8_t flashcart_in_game = 2;
char flashcart_file_name[0x08] = { 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF };
uint8_t flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;

uint8_t FLASHCART_MESSAGE_RESET[16] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t frames_since_last_ping = 0;

void flashcart_handshake() {
    if (FLASHCART_READ_BUF[0] == 'c' && FLASHCART_READ_BUF[1] == 'm' && FLASHCART_READ_BUF[2] == 'd' && FLASHCART_READ_BUF[3] == 't') {
        uint8_t reply[16] = {
            'O', 'o', 'T', 'R',
            FLASHCART_PROTOCOL_VERSION,
            CFG_RANDO_VERSION_MAJOR,
            CFG_RANDO_VERSION_MINOR,
            CFG_RANDO_VERSION_PATCH,
            CFG_RANDO_VERSION_BRANCH,
            CFG_RANDO_VERSION_SUPPLEMENTARY,
            PLAYER_ID,
            CFG_FILE_SELECT_HASH[0],
            CFG_FILE_SELECT_HASH[1],
            CFG_FILE_SELECT_HASH[2],
            CFG_FILE_SELECT_HASH[3],
            CFG_FILE_SELECT_HASH[4],
        };
        usb_write(DATATYPE_RAWBINARY, reply, 16);
        flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_HANDSHAKE;
    } else {
        usb_write(DATATYPE_RAWBINARY, FLASHCART_MESSAGE_RESET, 16);
        flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
    }
}

void flashcart_update_in_game(bool in_game) {
    if (in_game) {
        // Save context size is 0x1428 in decomp, 0x1450 in z64.h
        s8 result = usb_write(DATATYPE_INGAME_STATE, &z64_file, 5200); // State: In Game
        // uint8_t state_packet[16] = {
        //     0x01, // State: File Select
        //     z64_file.file_name[0],
        //     z64_file.file_name[1],
        //     z64_file.file_name[2],
        //     z64_file.file_name[3],
        //     z64_file.file_name[4],
        //     z64_file.file_name[5],
        //     z64_file.file_name[6],
        //     z64_file.file_name[7],
        //     0, 0, 0, 0, 0, 0, 0,
        // };
        // usb_write(DATATYPE_INGAME_STATE, state_packet, 16);
        if (result == 1)
            flashcart_in_game = 1;
    } else {
        uint8_t state_packet[16] = {
            0x01, // State: File Select
            z64_file.file_name[0],
            z64_file.file_name[1],
            z64_file.file_name[2],
            z64_file.file_name[3],
            z64_file.file_name[4],
            z64_file.file_name[5],
            z64_file.file_name[6],
            z64_file.file_name[7],
            0, 0, 0, 0, 0, 0, 0,
        };
        usb_write(DATATYPE_SAVE_FILENAME, state_packet, 16);
        flashcart_in_game = 0;
        for (int i = 0; i < 8; i++) {
            flashcart_file_name[i] = z64_file.file_name[i];
        }
    }
}

bool flashcart_queue_message(int datatype, const void* data, int size) {
    if (!FLASHCART_CAN_QUEUE(size)) return false;
    memcpy(&FLASHCART_WRITE_QUEUE_BUF[FLASHCART_WRITE_QUEUE_CURSOR], data, size);
    FLASHCART_WRITE_QUEUE_CURSOR += size;
    FLASHCART_WRITE_QUEUE_BUF[FLASHCART_WRITE_QUEUE_CURSOR] = datatype;
    FLASHCART_WRITE_QUEUE_BUF[FLASHCART_WRITE_QUEUE_CURSOR + sizeof(int)] = size;
    FLASHCART_WRITE_QUEUE_CURSOR += sizeof(int) * 2;
    return true;
}

void flashcart_frame(bool in_game) {
    if (usb_getcart() != CART_NONE) {
        // Clear USB buffer before potentially writing back
        if (usb_poll() != 0) {
            u32 header = usb_poll();
            int incoming_type = USBHEADER_GETTYPE(header);
            int incoming_size = USBHEADER_GETSIZE(header);
            if (incoming_size <= FLASHCART_BUFFER_SIZE) {
                usb_read(FLASHCART_READ_BUF, incoming_size);
            } else {
                // Discard packets that are too large for the buffer
                usb_skip(incoming_size);
                incoming_type = DATATYPE_HEARTBEAT;
                incoming_size = 0;
            }
            // Heartbeat from external client is ignored in all
            // states as it contains no data to process.
            if (incoming_type != DATATYPE_HEARTBEAT) {
                // Data potentially requiring action
                switch (flashcart_protocol_state) {
                    case FLASHCART_PROTOCOL_STATE_INIT: {
                        flashcart_handshake();
                        break;
                    }
                    case FLASHCART_PROTOCOL_STATE_HANDSHAKE: {
                        if (FLASHCART_READ_BUF[0] == 'M' && FLASHCART_READ_BUF[1] == 'W') {
                            if (FLASHCART_READ_BUF[2] != FLASHCART_PROTOCOL_VERSION) {
                                usb_write(DATATYPE_RAWBINARY, FLASHCART_MESSAGE_RESET, 16);
                                flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
                            } else {
                                MW_SEND_OWN_ITEMS = FLASHCART_READ_BUF[3];
                                MW_PROGRESSIVE_ITEMS_ENABLE = FLASHCART_READ_BUF[4];
                                flashcart_in_game = 2; // uninitialized; ensure state packet is sent
                                flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_MW;
                            }
                        } else if (FLASHCART_READ_BUF[0] == 'c') {
                            flashcart_handshake();
                        } else {
                            usb_write(DATATYPE_RAWBINARY, FLASHCART_MESSAGE_RESET, 16);
                            flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
                        }
                        break;
                    }
                    case FLASHCART_PROTOCOL_STATE_MW: {
                        if (FLASHCART_READ_BUF[0] == 0x00) {
                            // ping
                        } else if (FLASHCART_READ_BUF[0] == 0x01) {
                            // player data
                            uint8_t player_id = FLASHCART_READ_BUF[1];
                            for (int i = 0; i < 8; i++) {
                                PLAYER_NAMES[player_id][i] = FLASHCART_READ_BUF[2 + i];
                            }
                            MW_PROGRESSIVE_ITEMS_STATE[player_id] = *((mw_progressive_items_state_t*) (&FLASHCART_READ_BUF[10]));
                        } else if (FLASHCART_READ_BUF[0] == 0x02) {
                            // get item
                            uint16_t incoming_item = FLASHCART_READ_BUF[1] << 8 | FLASHCART_READ_BUF[2];
                            override_t override = { 0 };
                            override.key.scene = 0xFF;
                            override.key.type = OVR_DELAYED;
                            override.key.flag = 0xFF;
                            override.value.base.player = incoming_item == 0xca ? (PLAYER_ID == 1 ? 2 : 1) : PLAYER_ID;
                            override.value.base.item_id = incoming_item;
                            push_pending_item(override);
                        } else if (FLASHCART_READ_BUF[0] == 'c') {
                            flashcart_handshake();
                        } else {
                            usb_write(DATATYPE_RAWBINARY, FLASHCART_MESSAGE_RESET, 16);
                            flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
                        }
                        break;
                    }
                }
            }
        }
        // Re-test for additional messages in the read queue
        if (usb_poll() == 0) {
            if (FLASHCART_WRITE_QUEUE_CURSOR > 0) {
                // Flashcart write buffer is emptied all at once.
                // Clients are expected to handle multiple messages
                // through the protocol headers.
                while (FLASHCART_WRITE_QUEUE_CURSOR > 0) {
                    int outgoing_size = FLASHCART_WRITE_QUEUE_BUF[FLASHCART_WRITE_QUEUE_CURSOR - sizeof(int)];
                    int outgoing_type = FLASHCART_WRITE_QUEUE_BUF[FLASHCART_WRITE_QUEUE_CURSOR - sizeof(int) * 2];
                    FLASHCART_WRITE_QUEUE_CURSOR -= outgoing_size + sizeof(int) * 2;
                    usb_write(outgoing_type, &FLASHCART_WRITE_QUEUE_BUF[FLASHCART_WRITE_QUEUE_CURSOR], outgoing_size);
                }
            } else if (flashcart_protocol_state == FLASHCART_PROTOCOL_STATE_MW &&
                    ((in_game && flashcart_in_game != 1) ||
                     !in_game)) {
                if (!in_game) {
                    bool filenames_match = true;
                    for (int i = 0; i < 8; i++) {
                        if (z64_file.file_name[i] != flashcart_file_name[i]) {
                            filenames_match = false;
                            break;
                        }
                    }
                    if (!filenames_match || flashcart_in_game != 0) {
                        flashcart_update_in_game(in_game);
                    }
                } else if (z64_logo_state != 0x802C5880
                        && z64_logo_state != 0
                        && z64_file.game_mode == 0) {
                    flashcart_update_in_game(in_game);
                }
            } else if (++frames_since_last_ping >= 5 * 20) {
                // No incoming data to process. Send heartbeat to
                // maintain connection or signal to a new client we
                // are ready to handshake.
                if (flashcart_protocol_state == FLASHCART_PROTOCOL_STATE_MW) {
                    usb_sendheartbeat();
                } else {
                    usb_sendhandshake();
                }
                frames_since_last_ping = 0;
            }
        }
    }
}
