#ifndef FLASHCART_H
#define FLASHCART_H

#include <stdbool.h>
#include "z64.h"

#define FLASHCART_PROTOCOL_STATE_INIT      0x00
#define FLASHCART_PROTOCOL_STATE_HANDSHAKE 0x01
#define FLASHCART_PROTOCOL_STATE_MW        0x02

#define FLASHCART_READ_BUFFER_SIZE 1024
#define FLASHCART_WRITE_BUFFER_SIZE 1024

void flashcart_frame(z64_menudata_t* menu_data);
bool flashcart_queue_message(int datatype, const void* data, int size);
void flashcart_initialize();

extern uint8_t flashcart_protocol_state;
extern uint8_t FLASHCART_READ_BUF[FLASHCART_READ_BUFFER_SIZE];
extern uint8_t frames_since_last_ping;
extern uint8_t flashcart_in_game;
extern uint8_t SERIAL_PROCESSING;
extern uint8_t* pending_gossip_hint;

#endif
