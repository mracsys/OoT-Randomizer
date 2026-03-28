#include "hooks.h"
#include "types.h"
#include "vc.h"
#include "serial_stream.h"

/**
 * @brief Hook in main loop, replacing the function pointer to `cpuExecuteLoadStore` call in `cpuExecute`.
 * 
 * Stream to/from emulated RAM before load/store instructions are executed.
 * 
 * @param pCPU The emulated N64 RAM.
 * @param nSize Unused. Original call would set this to either 4MiB or 8MiB depending on game. The hack forces this to 8MiB.
 * @return bool true on success, false otherwise.
 */
bool frameEnd_hook() {
    //pCPU->gTree->kill_number = 0;

    serial_stream();
    return true;
}
