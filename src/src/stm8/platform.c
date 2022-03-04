#include "platform.h"
#include <stm8s.h>
#include <stdint.h>

uint32_t _bootloader_data;


void mcu_reboot(void)
{
    WWDG->CR = WWDG_CR_WDGA;
}

float log10f(int val)
{
    #warning "log10 is not implemented!!"
    (void)val;
    return 0;
}
