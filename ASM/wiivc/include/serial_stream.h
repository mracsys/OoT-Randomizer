#ifndef __SERIAL_STREAM_H
#define __SERIAL_STREAM_H

#include "types.h"
#include "device_wii.h"

bool serial_stream(void);
extern WiiSerialDevice* serial_usb;

#endif