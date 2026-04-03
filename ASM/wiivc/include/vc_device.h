// Copied from homeboy with some minor changes
// https://github.com/PracticeROM/homeboy
#ifndef __VC_DEVICE_H
#define __VC_DEVICE_H

typedef enum {
    SERIALERR_SUCCESS,
    SERIALERR_FAIL
} SerialDeviceError;

typedef union {
    struct {
        uint32_t key;
        void* transmit_addr;
        uint32_t transmit_header;
        void* receive_addr;
        uint32_t receive_header;
        union {
            struct {
                uint32_t              : 22;
                uint32_t reset        : 1;
                uint32_t error        : 4;
                uint32_t initialize   : 1;
                uint32_t receiving    : 1;
                uint32_t transmitting : 1;
                uint32_t busy         : 1;
                uint32_t ready        : 1;
            };
            uint32_t status;
        };
        int incoming_queue_cursor;
        int active_queue_index;
    };
    uint32_t regs[8];
} SerialVirtualDevice;

#define SERIAL_STATUS_RESET       (0b1    << 9)
#define SERIAL_STATUS_ERROR       (0b1111 << 5)
#define SERIAL_STATUS_INIT        (0b1    << 4)
#define SERIAL_STATUS_RX          (0b1    << 3)
#define SERIAL_STATUS_TX          (0b1    << 2)
#define SERIAL_STATUS_BUSY        (0b1    << 1)
#define SERIAL_STATUS_READY       (0b1    << 0)

extern void* n64_dram;
extern SerialVirtualDevice* serial_device_object;

void serial_device_init(void);

#endif
