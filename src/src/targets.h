#pragma once

#include <stdint.h>

#define CONCAT_helper(x, y) x ## y
#define CONCAT(x, y) CONCAT_helper(x, y)
#define IDENT(x) x
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define PATH(x, y) STR(IDENT(x)IDENT(y)IDENT(.h))


#if defined(EACHINE_TX801)
#include "targets/Eachine_TX801/Eachine_TX801.h"
#elif defined(EACHINE_TX526)
#include "targets/Eachine_TX526/Eachine_TX526.h"
#elif defined(EWRF_E7082VM)
#include "targets/EWRF_E7082VM/EWRF_E7082VM.h"
#endif

// These are target specific functions and need to implemented per target!

void taget_setup(void);
void taget_loop(void);
void target_set_power_dB(uint8_t power);

void target_rfPowerAmpPinSetup(void);
uint32_t vpd_value_get(void);

