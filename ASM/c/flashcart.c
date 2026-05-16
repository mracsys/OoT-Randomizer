#include <stdbool.h>

#include "flashcart.h"

#include "get_items.h"
#include "item_upgrades.h"
#include "z64.h"
#include "usb.h"
#include "ultratypes.h"
#include "ultra64.h"
#include "file_select.h"
#include "dungeon_info.h"

#define GAME_STATE_MENU 0
#define GAME_STATE_PLAY 1
#define GAME_STATE_INIT 2

uint8_t FLASHCART_READ_BUF[FLASHCART_WRITE_BUFFER_SIZE];

// Intermediate write buffer to ensure we don't interfere with
// PC messages mid-frame before they can be read.
// Bypassed in specific instances where it is known the queue
// is empty, such as sending heartbeats or save file info.
typedef struct {
    uint8_t buffer[FLASHCART_WRITE_BUFFER_SIZE];
    uint8_t read_cursor;
    uint8_t write_cursor;
} write_queue;

write_queue FLASHCART_WRITE_QUEUE;

#define FLASHCART_CAN_QUEUE(x) (FLASHCART_WRITE_QUEUE.write_cursor + x + sizeof(int) * 2 < FLASHCART_WRITE_BUFFER_SIZE ? 1 : 0)

uint8_t FLASHCART_PROTOCOL_VERSION = 3;
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

uint8_t flashcart_in_game = GAME_STATE_INIT;
uint8_t flashcart_file_name[0x08] = { 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF };
uint8_t flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;

uint8_t frames_since_last_ping = 0;

// Separate write ack monitoring variables in case
// the timeout start value happens to be 0;
bool usb_write_block = false;
uint32_t usb_write_start = 0;

bool filename_queued = false;
bool savectxt_queued = false;

uint8_t* pending_gossip_hint = NULL;

void flashcart_initialize() {
    memset(FLASHCART_WRITE_QUEUE.buffer, 0, sizeof(FLASHCART_WRITE_QUEUE.buffer));
    FLASHCART_WRITE_QUEUE.read_cursor = 0;
    FLASHCART_WRITE_QUEUE.write_cursor = 0;
    usb_initialize();
}

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
        usb_sendreset();
        flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
    }
}

void flashcart_update_in_game(z64_menudata_t* menu_data) {
    if (menu_data == NULL) {
        // Save context size is 0x1428 in decomp, 0x1450 in z64.h
        if (flashcart_queue_message(DATATYPE_INGAME_STATE, &z64_file, 5200)) {
            savectxt_queued = true;
            flashcart_in_game = GAME_STATE_PLAY;
        }
        send_dungeon_reward_areas();
        send_dungeon_locations();
    } else {
        if (flashcart_queue_message(DATATYPE_SAVE_FILENAME, flashcart_file_name, 16)) {
            filename_queued = true;
            flashcart_in_game = GAME_STATE_MENU;
        }
        for (int i = 0; i < 8; i++) {
            flashcart_file_name[i] = menu_data->name[menu_data->selected_item][i];
        }
    }
}

bool flashcart_queue_message(int datatype, const void* data, int size) {
    if ((savectxt_queued && datatype == DATATYPE_INGAME_STATE) || (filename_queued && datatype == DATATYPE_SAVE_FILENAME))
        return true;
    switch (datatype) {
        case DATATYPE_INGAME_STATE:
        case DATATYPE_SAVE_FILENAME:
            // Queue pointer to save context instead of the data
            if (!FLASHCART_CAN_QUEUE(sizeof(void*))) return false;
            break;
        default:
            if (!FLASHCART_CAN_QUEUE(size)) return false;
            break;
    }
    memcpy(&FLASHCART_WRITE_QUEUE.buffer[FLASHCART_WRITE_QUEUE.write_cursor], &datatype, sizeof(int));
    FLASHCART_WRITE_QUEUE.write_cursor += sizeof(int);
    memcpy(&FLASHCART_WRITE_QUEUE.buffer[FLASHCART_WRITE_QUEUE.write_cursor], &size, sizeof(int));
    FLASHCART_WRITE_QUEUE.write_cursor += sizeof(int);
    switch (datatype) {
        case DATATYPE_INGAME_STATE:
        case DATATYPE_SAVE_FILENAME:
            // Queue pointer to save context instead of the data
            memcpy(&FLASHCART_WRITE_QUEUE.buffer[FLASHCART_WRITE_QUEUE.write_cursor], &data, sizeof(void*));
            FLASHCART_WRITE_QUEUE.write_cursor += sizeof(void*);
            break;
        default:
            memcpy(&FLASHCART_WRITE_QUEUE.buffer[FLASHCART_WRITE_QUEUE.write_cursor], data, size);
            FLASHCART_WRITE_QUEUE.write_cursor += size;
            break;
    }
    return true;
}

void flashcart_pop_message() {
    int outgoing_size, outgoing_type;
    memcpy(&outgoing_type, &FLASHCART_WRITE_QUEUE.buffer[FLASHCART_WRITE_QUEUE.read_cursor], sizeof(int));
    memcpy(&outgoing_size, &FLASHCART_WRITE_QUEUE.buffer[FLASHCART_WRITE_QUEUE.read_cursor + sizeof(int)], sizeof(int));
    switch (outgoing_type) {
        case DATATYPE_INGAME_STATE:
        case DATATYPE_SAVE_FILENAME:
            FLASHCART_WRITE_QUEUE.read_cursor += sizeof(int) * 3;
            break;
        default:
            FLASHCART_WRITE_QUEUE.read_cursor += outgoing_size + sizeof(int) * 2;
            break;
    }
    if (FLASHCART_WRITE_QUEUE.read_cursor == FLASHCART_WRITE_QUEUE.write_cursor) {
        FLASHCART_WRITE_QUEUE.read_cursor = 0;
        FLASHCART_WRITE_QUEUE.write_cursor = 0;
    }
}

void flashcart_frame(z64_menudata_t* menu_data) {
    if (usb_getcart() != CART_NONE) {
        bool signal_processing = SERIAL_PROCESSING != 0 ? true : false;
        // Handle potentially lost acknowledge packet without
        // total communications loss. Force reset the connection
        // to avoid duplicating items and desyncing the item counter.
        // 7000ms chosen to be consistent with PC ack timeout.
        // Timeout starts after USB write to ensure any lag from
        // writing does not count against the timeout.
        if (usb_write_block && usb_timeout_check(usb_write_start, 7000)) {
            flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
        }
        // Clear USB write block in case the connection was reset
        if (flashcart_protocol_state != FLASHCART_PROTOCOL_STATE_MW) {
            usb_write_block = false;
        }
        // Clear USB buffer before potentially writing back
        if (usb_poll() != 0) {
            u32 header = usb_poll();
            int incoming_type = USBHEADER_GETTYPE(header);
            int incoming_size = USBHEADER_GETSIZE(header);
            s8 read_status = 0; // assume failure
            if (incoming_size <= FLASHCART_READ_BUFFER_SIZE) {
                read_status = usb_read(FLASHCART_READ_BUF, incoming_size);
            } else {
                // Discard packets that are too large for the buffer
                usb_skip(incoming_size);
                incoming_type = DATATYPE_HEARTBEAT;
                incoming_size = 0;
            }
            // Heartbeat from external client is ignored in all
            // states as it contains no data to process.
            if (read_status && incoming_type != DATATYPE_HEARTBEAT) {
                // Data potentially requiring action
                switch (flashcart_protocol_state) {
                    case FLASHCART_PROTOCOL_STATE_INIT: {
                        if (incoming_type == DATATYPE_HANDSHAKE) {
                            flashcart_handshake();
                        } else {
                            usb_sendreset();
                        }
                        break;
                    }
                    case FLASHCART_PROTOCOL_STATE_HANDSHAKE: {
                        if (incoming_type == DATATYPE_HANDSHAKE && FLASHCART_READ_BUF[0] == 'M' && FLASHCART_READ_BUF[1] == 'W') {
                            if (FLASHCART_READ_BUF[2] != FLASHCART_PROTOCOL_VERSION) {
                                usb_sendreset();
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
                            usb_sendreset();
                            flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
                        }
                        break;
                    }
                    case FLASHCART_PROTOCOL_STATE_MW: {
                        if (incoming_type == DATATYPE_ACK_MESSAGE) {
                            // Check if we actually sent a message to acknowledge.
                            if (!usb_write_block) {
                                usb_sendreset();
                                flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
                            } else {
                                int outgoing_type;
                                memcpy(&outgoing_type, &FLASHCART_WRITE_QUEUE.buffer[FLASHCART_WRITE_QUEUE.read_cursor], sizeof(int));
                                if (outgoing_type == DATATYPE_INGAME_STATE) savectxt_queued = false;
                                if (outgoing_type == DATATYPE_SAVE_FILENAME) filename_queued = false;
                                flashcart_pop_message();
                                // Allow sending the next message in queue
                                usb_write_block = false;
                            }
                        } else if (incoming_type == DATATYPE_UNRECOVERABLE) {
                            // Check if we actually sent a message to acknowledge.
                            if (!usb_write_block) {
                                usb_sendreset();
                                flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
                            } else {
                                // Retry sending message, don't remove from queue yet.
                                usb_write_block = false;
                            }
                        } else if (incoming_type == DATATYPE_PLAYER_NAMES) {
                            // player data
                            if (incoming_size < 10) {
                                usb_sendreadfailure();
                            } else {
                                uint8_t player_id = FLASHCART_READ_BUF[0];
                                for (int i = 0; i < 8; i++) {
                                    PLAYER_NAMES[player_id][i] = FLASHCART_READ_BUF[1 + i];
                                }
                                MW_PROGRESSIVE_ITEMS_STATE[player_id] = *((mw_progressive_items_state_t*) (&FLASHCART_READ_BUF[9]));
                                usb_sendreadsuccess();
                            }
                        } else if (incoming_type == DATATYPE_SEND_ITEM) {
                            // get item
                            if (incoming_size < 2)
                                usb_sendreadfailure();
                            uint16_t incoming_item = FLASHCART_READ_BUF[0] << 8 | FLASHCART_READ_BUF[1];
                            if (incoming_item == 0)
                                usb_sendreadfailure();
                            override_t override = { 0 };
                            override.key.scene = 0xFF;
                            override.key.type = OVR_DELAYED;
                            override.key.flag = 0xFF;
                            override.value.base.player = incoming_item == 0xCA ? (PLAYER_ID == 1 ? 2 : 1) : PLAYER_ID;
                            override.value.base.item_id = incoming_item;
                            push_pending_item(override);
                            usb_sendreadsuccess();
                        } else if (incoming_type == DATATYPE_READ_MEMORY) {
                            // format XXXXXXXXYYYYYYYY
                            // X = RAM address
                            // Y = Total bytes to send
                            if (incoming_size < 8) {
                                usb_sendreadfailure();
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
                                    usb_sendreadfailure();
                                } else {
                                    // data payload in place of success packet
                                    usb_write(DATATYPE_RAWBINARY, ram_address, payload_size);
                                }
                            }
                        } else if (incoming_type == DATATYPE_WRITE_MEMORY) {
                            // format XXXXXXXXYYYYYYYYZ
                            // X = RAM address
                            // Y = Total bytes to overwrite
                            // Z = Start of data payload
                            if (incoming_size < 9) {
                                usb_sendreadfailure();
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
                                    usb_sendreadfailure();
                                } else {
                                    osWritebackDCache((void*)ram_address, payload_size);
                                    osInvalDCache((void*)ram_address, payload_size);
                                    for (uint32_t i = 8; i < 8 + payload_size; i++) {
                                        *ram_address = FLASHCART_READ_BUF[i];
                                        ram_address++;
                                    }
                                    usb_sendreadsuccess();
                                }
                            }
                        } else if (incoming_type == DATATYPE_HANDSHAKE) {
                            flashcart_handshake();
                        } else {
                            usb_sendreset();
                            flashcart_protocol_state = FLASHCART_PROTOCOL_STATE_INIT;
                        }
                        break;
                    }
                }
            } else if (!read_status) {
                usb_sendreadfailure();
            }
        }
        // Re-test for additional messages in the read queue
        if (usb_poll() == 0) {
            bool message_sent = false;
            if (FLASHCART_WRITE_QUEUE.write_cursor > 0 && !usb_write_block && flashcart_protocol_state == FLASHCART_PROTOCOL_STATE_MW) {
                int outgoing_size, outgoing_type;
                memcpy(&outgoing_type, &FLASHCART_WRITE_QUEUE.buffer[FLASHCART_WRITE_QUEUE.read_cursor], sizeof(int));
                memcpy(&outgoing_size, &FLASHCART_WRITE_QUEUE.buffer[FLASHCART_WRITE_QUEUE.read_cursor + sizeof(int)], sizeof(int));
                uint8_t temp_cursor = FLASHCART_WRITE_QUEUE.read_cursor + sizeof(int) * 2;
                switch (outgoing_type) {
                    case DATATYPE_INGAME_STATE:
                        void* data;
                        memcpy(&data, &FLASHCART_WRITE_QUEUE.buffer[temp_cursor], sizeof(void*));
                        usb_write(outgoing_type, data, outgoing_size);
                        break;
                    case DATATYPE_SAVE_FILENAME:
                        uint8_t state_packet[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
                        memcpy(state_packet, flashcart_file_name, 8);
                        usb_write(outgoing_type, (void*)state_packet, 16);
                        break;
                    default:
                        usb_write(outgoing_type, &FLASHCART_WRITE_QUEUE.buffer[temp_cursor], outgoing_size);
                        break;
                }
                usb_write_start = usb_timeout_start();
                usb_write_block = true;
                message_sent = true;
            } else if (flashcart_protocol_state == FLASHCART_PROTOCOL_STATE_MW &&
                    ((menu_data == NULL && flashcart_in_game != GAME_STATE_PLAY) ||
                     menu_data != NULL)) {
                if (menu_data != NULL) {
                    if (menu_data->selected_item < 2) {
                        if (SLOT_OCCUPIED(menu_data->sram_buffer, menu_data->selected_item)) {
                            bool filenames_match = true;
                            for (int i = 0; i < 8; i++) {
                                if (menu_data->name[menu_data->selected_item][i] != flashcart_file_name[i]) {
                                    filenames_match = false;
                                    break;
                                }
                            }
                            if (!filenames_match || flashcart_in_game != GAME_STATE_MENU) {
                                flashcart_update_in_game(menu_data);
                                message_sent = true;
                            }
                        }
                    }
                } else if (z64_logo_state != 0x802C5880
                        && z64_logo_state != 0
                        && z64_file.game_mode == 0) {
                    flashcart_update_in_game(menu_data);
                    message_sent = true;
                }
            } else if (flashcart_protocol_state == FLASHCART_PROTOCOL_STATE_MW && pending_gossip_hint != NULL) {
                flashcart_queue_message(DATATYPE_SEND_HINT, pending_gossip_hint, 25);
                pending_gossip_hint = NULL;
            }
            if (++frames_since_last_ping >= 5 * 20 && !message_sent) {
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
        // Only signal that processing is done if messages were
        // ready for processing at the beginning of the function
        if (signal_processing) SERIAL_PROCESSING = 0;
    }
}
