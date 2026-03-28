#include "hooks.h"
#include "types.h"
#include "vc.h"
#include "serial_stream.h"

extern u8* cpuExecuteLoadStore_hook_addr_ha;
extern u8* cpuExecuteLoadStore_hook_addr_l;

/**
 * @brief Hook in main loop, replacing the function pointer to `cpuExecuteLoadStore` call in `cpuExecute`.
 * 
 * Stream to/from emulated RAM before load/store instructions are executed.
 * 
 * @param pCPU The emulated N64 RAM.
 * @param nSize Unused. Original call would set this to either 4MiB or 8MiB depending on game. The hack forces this to 8MiB.
 * @return bool true on success, false otherwise.
 */
void frameEnd_hook(Cpu* pCPU) {
    pCPU->gTree->kill_number = 0;

    serial_stream();
}
