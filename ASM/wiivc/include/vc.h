// Copied from homeboy with some minor changes
// https://github.com/PracticeROM/homeboy

#ifndef __VC_H
#define __VC_H

#include <stddef.h>
#include <stdint.h>

#include "cpu.h"

#define INIT __attribute__((section(".init")))

// system.h
typedef enum SystemMode {
    SM_NONE = -1,
    SM_RUNNING = 0,
    SM_STOPPED = 1,
} SystemMode;

typedef enum SystemObjectType {
    SOT_NONE = -1,
    SOT_CPU = 0,
    SOT_PIF = 1,
    SOT_RAM = 2,
    SOT_ROM = 3,
    SOT_RSP = 4,
    SOT_RDP = 5,
    SOT_MI = 6,
    SOT_DISK = 7,
    SOT_AI = 8,
    SOT_VI = 9,
    SOT_SI = 10,
    SOT_PI = 11,
    SOT_RDB = 12,
    SOT_PAK = 13,
    SOT_SRAM = 14,
    SOT_FLASH = 15,
    SOT_CODE = 16,
    SOT_HELP = 17,
    SOT_LIBRARY = 18,
    SOT_FRAME = 19,
    SOT_AUDIO = 20,
    SOT_VIDEO = 21,
    SOT_CONTROLLER = 22,
    SOT_COUNT = 23,
} SystemObjectType;

typedef s32 SystemRomType; // big enum, no need to have it here for now

typedef struct System {
    /* 0x00 */ bool bException;
    /* 0x04 */ SystemMode eMode;
    /* 0x08 */ SystemObjectType storageDevice;
    /* 0x0C */ SystemRomType eTypeROM;
    /* 0x10 */ void* apObject[SOT_COUNT];
    /* 0x6C */ s32 unk_6C;
    /* 0x70 */ u64 nAddressBreak;
    /* 0x78 */ s32 unk_78[19];
    /* 0xC4 */ void* pSound;
    /* 0xC8 */ u8 anException[16];
} System; // size = 0xD8

typedef struct Ram {
    /* 0x00 */ void* pHost;
    /* 0x04 */ u8* pBuffer;
    /* 0x08 */ u32 nSize;
    /* 0x0C */ u32 RDRAM_CONFIG_REG;
    /* 0x10 */ u32 RDRAM_DEVICE_ID_REG;
    /* 0x14 */ u32 RDRAM_DELAY_REG;
    /* 0x18 */ u32 RDRAM_MODE_REG;
    /* 0x1C */ u32 RDRAM_REF_INTERVAL_REG;
    /* 0x20 */ u32 RDRAM_REF_ROW_REG;
    /* 0x24 */ u32 RDRAM_RAS_INTERVAL_REG;
    /* 0x28 */ u32 RDRAM_MIN_INTERVAL_REG;
    /* 0x2C */ u32 RDRAM_ADDR_SELECT_REG;
    /* 0x30 */ u32 RDRAM_DEVICE_MANUF_REG;
    /* 0x34 */ u32 RI_MODE_REG;
    /* 0x38 */ u32 RI_CONFIG_REG;
    /* 0x3C */ u32 RI_SELECT_REG;
    /* 0x40 */ u32 RI_REFRESH_REG;
    /* 0x44 */ u32 RI_LATENCY_REG;
} Ram; // size = 0x48

#define SYSTEM_CPU(pSystem) ((void*)(((System*)(pSystem))->apObject[SOT_CPU]))
#define SYSTEM_ROM(pSystem) ((Rom*)(((System*)(pSystem))->apObject[SOT_ROM]))

// xlObject.h
typedef struct _XL_OBJECTTYPE _XL_OBJECTTYPE;

typedef int (*EventFunc)(void* pObject, int nEvent, void* pArgument);

struct _XL_OBJECTTYPE {
    /* 0x0 */ char* szName;
    /* 0x4 */ s32 nSizeObject;
    /* 0x8 */ _XL_OBJECTTYPE* pClassBase;
    /* 0xC */ EventFunc pfEvent;
}; // size = 0x10


typedef enum SoundPlayMode {
    SPM_NONE = -1,
    SPM_PLAY = 0,
    SPM_RAMPQUEUED = 1,
    SPM_RAMPPLAYED = 2,
} SoundPlayMode;

typedef struct Sound {
    /* 0x000 */ s32 unk_00;
    /* 0x004 */ void* pSrcData;
    /* 0x008 */ s32 nFrequency;
    /* 0x00C */ s32 nDacrate;
    /* 0x010 */ s32 nSndLen;
    /* 0x014 */ void* apBuffer[16];
    /* 0x01C */ s32 anSizeBuffer[16];
    /* 0x094 */ s32 unk_94;
    /* 0x098 */ s32 nVolumeCurve[257];
    /* 0x49C */ s32 iBufferPlay;
    /* 0x4A0 */ s32 iBufferMake;
    /* 0x4A4 */ volatile SoundPlayMode eMode;
    /* 0x4A8 */ void* pBufferZero;
    /* 0x4AC */ void* pBufferHold;
    /* 0x4B0 */ void* pBufferRampUp;
    /* 0x4B4 */ void* pBufferRampDown;
    /* 0x4B8 */ s32 nSizePlay;
    /* 0x4BC */ s32 nSizeZero;
    /* 0x4C0 */ s32 nSizeHold;
    /* 0x4C4 */ s32 nSizeRamp;
} Sound; // size = 0x4C8

bool cpuExecuteUpdate(Cpu* pCPU, s32* pnAddressGCN, u32 nCount);
bool cpuMapObject(Cpu* pCPU, void* pObject, u32 nAddress0, u32 nAddress1, s32 nType);
bool cpuSetDeviceGet(Cpu* pCPU, CpuDevice* pDevice, void* pfGet8, void* pfGet16, void* pfGet32, void* pfGet64);
bool cpuSetDevicePut(Cpu* pCPU, CpuDevice* pDevice, void* pfPut8, void* pfPut16, void* pfPut32, void* pfPut64);
bool cpuFindFunction(Cpu* pCPU, s32 theAddress, CpuFunction** tree_node);
bool ramWipe(Ram* pRAM);
bool ramSetSize(Ram* pRAM, s32 nSize);
bool simulatorRumbleStop(s32 channel);
bool soundSetBufferSize(Sound* pSound, s32 nSize);
s32 cpuExecuteLoadStore(Cpu* pCPU, s32 nCount, s32 nAddressN64, s32 nAddressGCN);
bool cpuExecute(Cpu* pCPU, s32 nCount, u64 nAddressBreak);

bool xlHeapTake(void** ppHeap, s32 nByteCount);
bool xlHeapFree(void** ppHeap);
bool xlObjectMake(void** ppObject, void* pArgument, _XL_OBJECTTYPE* pType);

void DCStoreRange(const void* buf, u32 len);
void ICInvalidateRange(const void* buf, u32 len);
void DCFlushRange(void *startaddress,u32 len);
void DCInvalidateRange(void *startaddress,u32 len);

s64 OSGetTime(void);
u32 OSGetTick(void);

extern s32 ganMapGPR[32];
extern System* gpSystem;
extern u32 gnFlagZelda;

// TODO: use decomp types and names
#define cur_thread  (*(volatile OSThread**)0x800000C0)
#define ex_handlers ((volatile OSExceptionHandler*)0x80003000)

int IOS_OpenAsync(const char* file, int mode, void* callback, void* callback_data);
int IOS_Open(const char* file, int mode);
int IOS_CloseAsync(int fd, void* callback, void* callback_data);
int IOS_Close(int fd);
int IOS_ReadAsync(int fd, void* data, size_t len, void* callback, void* callback_data);
int IOS_Read(int fd, void* data, size_t len);
int IOS_WriteAsync(int fd, void* data, size_t len, void* callback, void* callback_data);
int IOS_Write(int fd, void* data, size_t len);
int IOS_SeekAsync(int fd, int where, int whence, void* callback, void* callback_data);
int IOS_Seek(int fd, int where, int whence);
int IOS_IoctlAsync(int fd, int ioctl, void* buffer_in, size_t size_in, void* buffer_io, size_t size_out, void* callback,
                   void* callback_data);
int IOS_Ioctl(int fd, int ioctl, void* buffer_in, size_t size_in, void* buffer_io, size_t size_out);
int IOS_IoctlvAsync(int fd, int ioctl, int cnt_in, int cnt_io, void* argv, void* callback, void* callback_data);
int IOS_Ioctlv(int fd, int ioctl, int cnt_in, int cnt_io, void* argv);

int iosCreateHeap(void* heap, size_t size);
void* iosAllocAligned(int hid, size_t size, size_t page_size);
bool iosFree(int hid, void* ptr);

void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* dst, int val, size_t n);

#define _ATTRIBUTE(attrs) __attribute__ (attrs)
int	snprintf (char *__restrict, size_t, const char *__restrict, ...);
int	printf (const char *__restrict, ...);
size_t strnlen (const char *, size_t);

#define title_id             (*(volatile u32*)0x80003180)

#define ios_heap_addr        0x933E8000
#define allocMEM2(ptr, size) xlHeapTake((void**)(ptr), (0x70000000 | (size)))

#define OS_BUS_CLOCK_SPEED (*(vs32*)0x800000F8)

// Time base frequency = 1/4 bus clock
#define OS_TIME_SPEED (OS_BUS_CLOCK_SPEED / 4)

#define OSTicksToSeconds(ticks) ((ticks) / OS_TIME_SPEED)
#define OSTicksToMilliseconds(ticks) ((ticks) / (OS_TIME_SPEED / 1000))
#define OSTicksToMicroseconds(ticks) (((ticks) * 8) / (OS_TIME_SPEED / 125000))
#define OSTicksToNanoseconds(ticks) (((ticks) * 8000) / (OS_TIME_SPEED / 125000))
#define OSSecondsToTicks(sec) ((sec) * OS_TIME_SPEED)
#define OSMillisecondsToTicks(msec) ((msec) * (OS_TIME_SPEED / 1000))
#define OSMicrosecondsToTicks(usec) (((usec) * (OS_TIME_SPEED / 125000)) / 8)
#define OSNanosecondsToTicks(nsec) (((nsec) * (OS_TIME_SPEED / 125000)) / 8000)

#define OSDiffTick(tick1, tick0) ((s32)(tick1) - (s32)(tick0))

#endif

