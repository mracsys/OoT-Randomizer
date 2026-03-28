#include "hooks.h"
#include "types.h"
#include "vc.h"
#include "serial_stream.h"

/**
 * @brief Hook in main loop, replacing the load for the return value of frameEnd (true).
 * 
 * Stream to/from emulated RAM.
 * 
 * @return bool always true.
 */
bool frameEnd_hook() {
    serial_stream();
    return true;
}
