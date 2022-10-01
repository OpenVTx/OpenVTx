#pragma once

#include <stdint.h>

#define CONCAT_helper(x, y) x ## y
#define CONCAT(x, y) CONCAT_helper(x, y)
#define IDENT(x) x
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define PATH(x, y) STR(IDENT(x)IDENT(y)IDENT(.h))


#if defined(GENERIC_GD32F130) || defined(HAPPYMODEL_PANCAKE) || defined(BETAFPV_A03)
#include "targets/Generic_GD32F130/Generic_GD32F130.h"
#elif defined(EACHINE_TX801)
#include "targets/Eachine_TX801/Eachine_TX801.h"
#elif defined(EACHINE_TX526)
#include "targets/Eachine_TX526/Eachine_TX526.h"
#endif

// These are target specific functions and need to implemented per target!

void target_setup(void);
void target_loop(void);
void target_set_power_dB(float power);
void checkPowerOutput(void);

void target_rfPowerAmpPinSetup(void);
uint32_t vpd_value_get(void);

void mcu_reboot(void);
