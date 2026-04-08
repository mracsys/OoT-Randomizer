#include <stdbool.h>

#include "flashcart.h"

#include "get_items.h"
#include "item_upgrades.h"
#include "z64.h"
#include "usb.h"
#include "ultratypes.h"
#include "ultra64.h"

#define GAME_STATE_MENU 0
#define GAME_STATE_PLAY 1
#define GAME_STATE_INIT 2

uint8_t FLASHCART_READ_BUF[FLASHCART_BUFFER_SIZE];

// Intermediate write buffer to ensure we don't interfere with
// PC messages mid-frame before they can be read.
// Bypassed in specific instances where it is known the queue
// is empty, such as sending heartbeats or save file info.
uint8_t FLASHCART_WRITE_QUEUE_BUF[FLASHCART_BUFFER_SIZE];
uint8_t FLASHCART_WRITE_QUEUE_CURSOR = 0;
#define FLASHCART_CAN_QUEUE(x) (FLASHCART_WRITE_QUEUE_CURSOR + x + sizeof(int) * 2 < FLASHCART_BUFFER_SIZE ? 1 : 0)

uint8_t FLASHCART_PROTOCOL_VERSION = 2;
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

// Dummy buffers
// All logic flow for these end states is handled by header data type
uint8_t FLASHCART_MESSAGE_ERROR[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t FLASHCART_MESSAGE_RESET[16] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t FLASHCART_MESSAGE_SUCCESS[16] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

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
        usb_write(DATATYPE_HANDSHAKE, reply, 16);
        flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_HANDSHAKE;
    } else {
        usb_write(DATATYPE_RESET, FLASHCART_MESSAGE_RESET, 16);
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
            flashcart_in_game = GAME_STATE_PLAY;
    } else {
        uint8_t state_packet[16] = {
            z64_file.file_name[0],
            z64_file.file_name[1],
            z64_file.file_name[2],
            z64_file.file_name[3],
            z64_file.file_name[4],
            z64_file.file_name[5],
            z64_file.file_name[6],
            z64_file.file_name[7],
            0, 0, 0, 0, 0, 0, 0, 0,
        };
        usb_write(DATATYPE_SAVE_FILENAME, state_packet, 16);
        flashcart_in_game = GAME_STATE_MENU;
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
                        if (incoming_type == DATATYPE_HANDSHAKE) {
                            flashcart_handshake();
                        } else {
                            usb_write(DATATYPE_RESET, FLASHCART_MESSAGE_RESET, 16);
                        }
                        break;
                    }
                    case FLASHCART_PROTOCOL_STATE_HANDSHAKE: {
                        if (incoming_type == DATATYPE_HANDSHAKE && FLASHCART_READ_BUF[0] == 'M' && FLASHCART_READ_BUF[1] == 'W') {
                            if (FLASHCART_READ_BUF[2] != FLASHCART_PROTOCOL_VERSION) {
                                usb_write(DATATYPE_RESET, FLASHCART_MESSAGE_RESET, 16);
                                flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
                            } else {
                                MW_SEND_OWN_ITEMS = FLASHCART_READ_BUF[3];
                                MW_PROGRESSIVE_ITEMS_ENABLE = FLASHCART_READ_BUF[4];
                                flashcart_in_game = GAME_STATE_INIT; // uninitialized; ensure state packet is sent
                                flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_MW;
                            }
                        } else if (incoming_type == DATATYPE_HANDSHAKE && FLASHCART_READ_BUF[0] == 'c') {
                            flashcart_handshake();
                        } else {
                            usb_write(DATATYPE_RESET, FLASHCART_MESSAGE_RESET, 16);
                            flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
                        }
                        break;
                    }
                    case FLASHCART_PROTOCOL_STATE_MW: {
                        if (incoming_type == DATATYPE_HEARTBEAT) {
                            // ping
                        } else if (incoming_type == DATATYPE_PLAYER_NAMES) {
                            // player data
                            if (incoming_size < 10) {
                                usb_write(DATATYPE_UNRECOVERABLE, FLASHCART_MESSAGE_ERROR, 16);
                            } else {
                                uint8_t player_id = FLASHCART_READ_BUF[0];
                                for (int i = 0; i < 8; i++) {
                                    PLAYER_NAMES[player_id][i] = FLASHCART_READ_BUF[1 + i];
                                }
                                MW_PROGRESSIVE_ITEMS_STATE[player_id] = *((mw_progressive_items_state_t*) (&FLASHCART_READ_BUF[9]));
                            }
                        } else if (incoming_type == DATATYPE_SEND_ITEM) {
                            // get item
                            uint16_t incoming_item = FLASHCART_READ_BUF[0] << 8 | FLASHCART_READ_BUF[1];
                            override_t override = { 0 };
                            override.key.scene = 0xFF;
                            override.key.type = OVR_DELAYED;
                            override.key.flag = 0xFF;
                            override.value.base.player = incoming_item == 0xca ? (PLAYER_ID == 1 ? 2 : 1) : PLAYER_ID;
                            override.value.base.item_id = incoming_item;
                            push_pending_item(override);
                        } else if (incoming_type == DATATYPE_READ_MEMORY) {
                            // format XXXXXXXXYYYYYYYY
                            // X = RAM address
                            // Y = Total bytes to send
                            if (incoming_size < 8) {
                                usb_write(DATATYPE_UNRECOVERABLE, FLASHCART_MESSAGE_ERROR, 16);
                            } else {
                                void* ram_address = (void*)((FLASHCART_READ_BUF[0] << 24) |
                                                    (FLASHCART_READ_BUF[1] << 16) |
                                                    (FLASHCART_READ_BUF[2] << 8) |
                                                    (FLASHCART_READ_BUF[3] << 0));
                                uint32_t payload_size = (FLASHCART_READ_BUF[4] << 24) |
                                                        (FLASHCART_READ_BUF[5] << 16) |
                                                        (FLASHCART_READ_BUF[6] << 8) |
                                                        (FLASHCART_READ_BUF[7] << 0);
                                // Bounds check for RAM and USB buffer size
                                if ((uint32_t)ram_address < 0x80000000 || (uint32_t)ram_address > 0x80800000 ||
                                    (uint32_t)ram_address + payload_size > 0x80800000 ||
                                    payload_size > DEBUG_ADDRESS_SIZE) {
                                    usb_write(DATATYPE_UNRECOVERABLE, FLASHCART_MESSAGE_ERROR, 16);
                                } else {
                                    usb_write(DATATYPE_RAWBINARY, ram_address, payload_size);
                                }
                            }
                        } else if (incoming_type == DATATYPE_WRITE_MEMORY) {
                            // format XXXXXXXXYYYYYYYYZ
                            // X = RAM address
                            // Y = Total bytes to overwrite
                            // Z = Start of data payload
                            if (incoming_size < 9) {
                                usb_write(DATATYPE_UNRECOVERABLE, FLASHCART_MESSAGE_ERROR, 16);
                            } else {
                                volatile uint8_t* ram_address = (void*)((FLASHCART_READ_BUF[0] << 24) |
                                                                (FLASHCART_READ_BUF[1] << 16) |
                                                                (FLASHCART_READ_BUF[2] << 8) |
                                                                (FLASHCART_READ_BUF[3] << 0));
                                uint32_t payload_size = (FLASHCART_READ_BUF[4] << 24) |
                                                        (FLASHCART_READ_BUF[5] << 16) |
                                                        (FLASHCART_READ_BUF[6] << 8) |
                                                        (FLASHCART_READ_BUF[7] << 0);
                                // Bounds check for RAM
                                if ((uint32_t)ram_address < 0x80000000 || (uint32_t)ram_address > 0x80800000 ||
                                    (uint32_t)ram_address + payload_size > 0x80800000) {
                                    usb_write(DATATYPE_UNRECOVERABLE, FLASHCART_MESSAGE_ERROR, 16);
                                } else {
                                    osWritebackDCache((void*)ram_address, payload_size);
                                    osInvalDCache((void*)ram_address, payload_size);
                                    for (uint32_t i = 8; i < 8 + payload_size; i++) {
                                        *ram_address = FLASHCART_READ_BUF[i];
                                        ram_address++;
                                    }
                                    usb_write(DATATYPE_WRITE_ACK, FLASHCART_MESSAGE_SUCCESS, 16);
                                }
                            }
                        } else if (incoming_type == DATATYPE_HANDSHAKE) {
                            flashcart_handshake();
                        } else {
                            usb_write(DATATYPE_RESET, FLASHCART_MESSAGE_RESET, 16);
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
                    ((in_game && flashcart_in_game != GAME_STATE_PLAY) ||
                     !in_game)) {
                if (!in_game) {
                    bool filenames_match = true;
                    for (int i = 0; i < 8; i++) {
                        if (z64_file.file_name[i] != flashcart_file_name[i]) {
                            filenames_match = false;
                            break;
                        }
                    }
                    if (!filenames_match || flashcart_in_game != GAME_STATE_MENU) {
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
