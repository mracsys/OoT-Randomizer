#ifndef __DEVICE_WII_H
#define __DEVICE_WII_H

#include <stdint.h>

/*********************************
                 Macros
*********************************/

#define USB_VID_FTDI 0x0403

#define USB_PID_FT232RL 0x6001
#define USB_PID_FT232H  0x6014
#define USB_PID_FT230X  0x6015

// Assumes baud rate of 115200
// FT232H uses the same divisor calc up to 3MBaud despite the higher clock rate
// See https://www.ftdichip.com/Documents/AppNotes/AN_120_Aliasing_VCP_Baud_Rates.pdf
#define USB_CLOCK_FT232RL 0x001A

#define MAX_PACKET_SIZE 64

#define USBHEADER_GETTYPE(header) (((header) & 0xFF000000) >> 24)
#define USBHEADER_GETSIZE(header) (((header) & 0x00FFFFFF))
#define USBHEADER_CREATE(type, left) ((((type)<<24) | ((left) & 0x00FFFFFF)))


/*********************************
             Typedefs
*********************************/

typedef uint8_t byte;

typedef struct
{
    int32_t  device_id;
    uint16_t vid;
    uint16_t pid;
    int32_t  handle;
    uint32_t base_clock;
    uint32_t bytes_written;
    uint32_t bytes_read;
} WiiSerialDevice;


/*********************************
             Enumerations
*********************************/

typedef enum {
    DATATYPE_UNKNOWN       = 0x00,
    DATATYPE_TEXT          = 0x01,
    DATATYPE_RAWBINARY     = 0x02,
    DATATYPE_HEADER        = 0x03,
    DATATYPE_SCREENSHOT    = 0x04,
    DATATYPE_HEARTBEAT     = 0x05,
    DATATYPE_RDBPACKET     = 0x06,
    DATATYPE_HANDSHAKE     = 0x07,
    DATATYPE_INGAME_STATE  = 0x08,
    DATATYPE_SAVE_FILENAME = 0x09
} USBDataType;

typedef enum {
    DEVICEERR_OK = 0,
    DEVICEERR_NOTCART,
    DEVICEERR_USBBUSY,
    DEVICEERR_NODEVICES,
    DEVICEERR_CARTFINDFAIL,
    DEVICEERR_CANTOPEN,
    DEVICEERR_FILEREADFAIL,
    DEVICEERR_RESETFAIL,
    DEVICEERR_RESETPORTFAIL,
    DEVICEERR_TIMEOUTSETFAIL,
    DEVICEERR_PURGEFAIL,
    DEVICEERR_READFAIL,
    DEVICEERR_WRITEFAIL,
    DEVICEERR_WRITEZERO,
    DEVICEERR_CLOSEFAIL,
    DEVICEERR_BITMODEFAIL_RESET,
    DEVICEERR_BITMODEFAIL_SYNCFIFO,
    DEVICEERR_SETDTRFAIL,
    DEVICEERR_CLEARDTRFAIL,
    DEVICEERR_GETMODEMSTATUSFAIL,
    DEVICEERR_TXREPLYMISMATCH,
    DEVICEERR_READCOMPSIGFAIL,
    DEVICEERR_NOCOMPSIG,
    DEVICEERR_READPACKSIZEFAIL,
    DEVICEERR_BADPACKSIZE,
    DEVICEERR_MALLOCFAIL,
    DEVICEERR_UPLOADCANCELLED,
    DEVICEERR_TIMEOUT,
    DEVICEERR_POLLFAIL,
    DEVICEERR_64D_BADCMP,
    DEVICEERR_64D_8303USB,
    DEVICEERR_64D_CANTDEBUG,
    DEVICEERR_64D_BADDMA,
    DEVICEERR_64D_DATATOOBIG,
    DEVICEERR_SC64_CMDFAIL,
    DEVICEERR_SC64_COMMFAIL,
    DEVICEERR_SC64_CTRLRELEASEFAIL,
    DEVICEERR_SC64_CTRLRESETFAIL,
    DEVICEERR_SC64_FIRMWARECHECKFAIL,
    DEVICEERR_SC64_FIRMWAREUNSUPPORTED,
} DeviceError;

/*********************************
        Function Prototypes
*********************************/

WiiSerialDevice* device_initialize_wii();
DeviceError device_test_wii(WiiSerialDevice* serial);
DeviceError device_open_wii(WiiSerialDevice* serial);
DeviceError device_senddata_wii(WiiSerialDevice *serial, uint32_t dataheader, byte *data);
DeviceError device_receivedata_wii(WiiSerialDevice* serial, uint32_t* dataheader, byte** buff);
DeviceError device_close_wii(WiiSerialDevice* serial);
void device_deinitialize_wii(WiiSerialDevice* serial);

#define  SWAP(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) // From https://graphics.stanford.edu/~seander/bithacks.html#SwappingValuesXOR
uint32_t swap_endian(uint32_t val);


#endif
