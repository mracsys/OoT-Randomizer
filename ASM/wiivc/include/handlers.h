#ifndef __HANDLERS_H
#define __HANDLERS_H

#include "types.h"
#include "vc_device.h"

bool queue_incoming_buffer(uint32_t header, uint8_t* buffer);
void purge_queue(void);
uint32_t handle_poll();
void handle_read(SerialVirtualDevice* device);
void handle_send(SerialVirtualDevice* device);

#endif