#ifndef UNFL_USB_H
#define UNFL_USB_H

    #include "ultratypes.h"

    /*********************************
             DataType macros
    *********************************/

    // UNCOMMENT THE #DEFINE IF USING LIBDRAGON
    //#define LIBDRAGON

    // Settings
    #define USE_OSRAW          0           // Use if you're doing USB operations without the PI Manager (libultra only)
    #define DEBUG_ADDRESS_SIZE 8*1024*1024 // Max size of USB I/O. The bigger this value, the more ROM you lose!

    // CHECK_EMULATOR checks currently trip on a real console with Everdrive V3, don't use them.
    #define CHECK_EMULATOR     0           // Stops the USB library from working if it detects an emulator to prevent problems

    // Cart definitions
    #define CART_NONE      0
    #define CART_64DRIVE   1
    #define CART_EVERDRIVE 2
    #define CART_SC64      3
    #define CART_WII       4
    #define CART_EMULATOR  5

    // Data types defintions
    #define DATATYPE_EMPTY           0x00
    #define DATATYPE_TEXT            0x01
    #define DATATYPE_RAWBINARY       0x02
    #define DATATYPE_HEADER          0x03
    #define DATATYPE_SCREENSHOT      0x04
    #define DATATYPE_HEARTBEAT       0x05
    #define DATATYPE_RDBPACKET       0x06
    #define DATATYPE_TCPTEST         0x07
    #define DATATYPE_ROMUPLOAD       0x08
    #define DATATYPE_HANDSHAKE       0x09
    #define DATATYPE_INGAME_STATE    0x0A
    #define DATATYPE_SAVE_FILENAME   0x0B
    #define DATATYPE_RESET           0x0C
    #define DATATYPE_SEND_ITEM       0x0D
    #define DATATYPE_ACK_MESSAGE     0x0E
    #define DATATYPE_DUNGEON_REWARDS 0x0F
    #define DATATYPE_PLAYER_NAMES    0x10
    #define DATATYPE_READ_MEMORY     0x11
    #define DATATYPE_WRITE_MEMORY    0x12
    #define DATATYPE_UNRECOVERABLE   0x13
    #define DATATYPE_ITEM_GIVEN      0x14
    #define DATATYPE_PROG_ITEM_STATE 0x15
    #define DATATYPE_SEND_HINT       0x16
    #define DATATYPE_SEND_ENTRANCE   0x17

    // Hint types definitions
    // Start at 1 so that empty hint data can be detected
    #define HINTTYPE_WOTH       0x01
    #define HINTTYPE_GOAL       0x02
    #define HINTTYPE_FOOLISH    0x03
    #define HINTTYPE_ITEM       0x04
    #define HINTTYPE_LOCATION   0x05
    #define HINTTYPE_ENTRANCE   0x06
    #define HINTTYPE_MAJOR_ITEM 0x07

    // Wii receive states
    #define SERIAL_READ_DONE       0x00
    #define SERIAL_READ_AVAILABLE  0x01
    #define SERIAL_READ_CONTINUE   0x02

    // Wii transmit states
    #define SERIAL_WRITE_DONE      0x00
    #define SERIAL_WRITE_AVAILABLE 0x01

    /*********************************
            Convenience macros
    *********************************/

    // Use these to conveniently read the header from usb_poll()
    #define USBHEADER_GETTYPE(header) (((header) & 0xFF000000) >> 24)
    #define USBHEADER_GETSIZE(header) (((header) & 0x00FFFFFF))


    /*********************************
            Wii macros and types
    *********************************/

    typedef enum {
        SERIALERR_SUCCESS,
        SERIALERR_FAIL,
        SERIALERR_TIMEOUT,
    } SerialDeviceError;

    typedef union {
        struct {
            u32 key;
            u32 transmit_addr;
            u32 transmit_header;
            u32 receive_addr;
            u32 receive_header;
            union {
                struct {
                    u32              : 22;
                    u32 reset        : 1;
                    u32 error        : 4;
                    u32 initialize   : 1;
                    u32 receiving    : 1;
                    u32 transmitting : 1;
                    u32 busy         : 1;
                    u32 ready        : 1;
                };
                u32 status;
            };
            int incoming_queue_cursor;
            int active_queue_index;
        };
        u32 regs[8];
    } SerialVirtualDevice;

    #define wii_serial_device (*(volatile SerialVirtualDevice *)0xA8060000)
    extern volatile SerialVirtualDevice EMULATOR_DEVICE;
    extern volatile u8 SERIAL_ENABLE;


    /*********************************
              USB Functions
    *********************************/

    /*==============================
        usb_initialize
        Initializes the USB buffers and pointers
        @return 1 if the USB initialization was successful, 0 if not
    ==============================*/

    extern char usb_initialize(void);


    /*==============================
        usb_getcart
        Returns which flashcart is currently connected
        @return The CART macro that corresponds to the identified flashcart
    ==============================*/

    extern char usb_getcart(void);


    /*==============================
        usb_write
        Writes data to the USB.
        Will not write if there is data to read from USB
        @param The DATATYPE that is being sent
        @param A buffer with the data to send
        @param The size of the data being sent
        @return 1 on success, 0 on fail, -1 on timeout
    ==============================*/

    extern s8 usb_write(int datatype, const void* data, u32 size);


    /*==============================
        usb_poll
        Returns the header of data being received via USB
        The first byte contains the data type, the next 3 the number of bytes left to read
        @return The data header, or 0
    ==============================*/

    extern unsigned long usb_poll(void);


    /*==============================
        usb_read
        Reads bytes from USB into the provided buffer
        @param The buffer to put the read data in
        @param The number of bytes to read
        @return 1 on success, 0 on failure, -1 on timeout
    ==============================*/

    extern s8 usb_read(void* buffer, u32 size);


    /*==============================
        usb_skip
        Skips a USB read by the specified amount of bytes
        @param The number of bytes to skip
    ==============================*/

    extern void usb_skip(u32 nbytes);


    /*==============================
        usb_rewind
        Rewinds a USB read by the specified amount of bytes
        @param The number of bytes to rewind
    ==============================*/

    extern void usb_rewind(u32 nbytes);


    /*==============================
        usb_purge
        Purges the incoming USB data
    ==============================*/

    extern void usb_purge(void);


    /*==============================
        usb_timedout
        Checks if the USB timed out recently
        @return 1 if the USB timed out, 0 if not
    ==============================*/

    extern char usb_timedout(void);


    /*==============================
        usb_timeout_start
        Returns current value of COUNT coprocessor 0 register
        @return C0_COUNT value
    ==============================*/

    extern u32 usb_timeout_start(void);


    /*==============================
        usb_timeout_check
        Checks if timeout occurred
        @param Starting value obtained from usb_timeout_start
        @param Timeout duration specified in milliseconds
        @return true if timeout occurred, otherwise false
    ==============================*/

    extern char usb_timeout_check(u32 start_ticks, u32 duration);


    /*==============================
        usb_sendheartbeat
        Sends a heartbeat packet to the PC
    ==============================*/

    extern s8 usb_sendheartbeat(void);


    /*==============================
        usb_sendhandshake
        Sends a handshake request packet to the PC
        This is done once automatically at initialization,
        but can be called manually to ensure that the
        host side tool is aware of the current USB protocol
        version.
    ==============================*/

    extern s8 usb_sendhandshake(void);


    /*==============================
        usb_sendreadfailure
        Sends a message indicating an unrecoverable error
        while reading from USB.
    ==============================*/

    extern s8 usb_sendreadfailure(void);


    /*==============================
        usb_sendreadsuccess
        Sends a message indicating the received message
        was received and successfully processed.
    ==============================*/

    extern s8 usb_sendreadsuccess(void);


    /*==============================
        usb_sendreset
        Sends a message to restart the handshake process.
    ==============================*/

    extern s8 usb_sendreset(void);

#endif
