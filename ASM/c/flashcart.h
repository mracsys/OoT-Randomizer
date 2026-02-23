#ifndef FLASHCART_H
#define FLASHCART_H

#include <stdbool.h>
#include "z64.h"

#define FLASHCART_PROTOCOL_STATE_INIT      0x00
#define FLASHCART_PROTOCOL_STATE_HANDSHAKE 0x01
#define FLASHCART_PROTOCOL_STATE_MW        0x02

#define FLASHCART_BUFFER_SIZE 128

void flashcart_frame(bool in_game);
bool flashcart_queue_message(int datatype, const void* data, int size);

extern uint8_t flashcart_protocol_state;
extern uint8_t FLASHCART_READ_BUF[FLASHCART_BUFFER_SIZE];
extern uint8_t FLASHCART_WRITE_QUEUE_CURSOR;
extern uint8_t FLASHCART_WRITE_QUEUE_BUF[FLASHCART_BUFFER_SIZE];
extern uint8_t frames_since_last_ping;
extern uint8_t flashcart_in_game;

#endif
