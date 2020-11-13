#pragma once

#include <stdint.h>

// These are target specific functions and need to implemented per target!
void setPowermW(uint16_t power);
void setPowerdB(uint16_t currPowerdB);

#ifdef EACHINE_TX801
#include "targets/Eachine_TX801/Eachine_TX801.h"
#endif
#ifdef EACHINE_TX526
#include "targets/Eachine_TX526/Eachine_TX526.h"
#endif
