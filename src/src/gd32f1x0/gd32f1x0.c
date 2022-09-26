#include <gd32f1x0.h>
#include "targets.h"

static volatile uint32_t _ms_cntr;

void mcu_reboot(void)
{
    NVIC_SystemReset();
}

void SysTick_Handler(void)
{
    ++_ms_cntr;
}

void systick_config(void) {
    /* setup systick timer for 1ms interrupts  */
    if (SysTick_Config(SystemCoreClock / 1000)) {
        /* capture error */
        while (1)
            ;
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00);
}

uint32_t millis(void)
{
    return _ms_cntr;
}

void delay(uint32_t const ms)
{
    uint32_t const start = millis();
    while((millis() - start) < ms);
}

void delayMicroseconds(uint32_t us)
{
  __IO uint32_t currentTicks = SysTick->VAL;
  /* Number of ticks per millisecond */
  const uint32_t tickPerMs = SysTick->LOAD + 1;
  /* Number of ticks to count */
  const uint32_t nbTicks = ((us - ((us > 0) ? 1 : 0)) * tickPerMs) / 1000;
  /* Number of elapsed ticks */
  uint32_t elapsedTicks = 0;
  __IO uint32_t oldTicks = currentTicks;
  do {
    currentTicks = SysTick->VAL;
    elapsedTicks += (oldTicks < currentTicks)
                        ? tickPerMs + oldTicks - currentTicks
                        : oldTicks - currentTicks;
    oldTicks = currentTicks;
  } while (nbTicks > elapsedTicks);
}


int main(void)
{
    /* Reset vector location which is set wrongly by SystemInit */
    extern uint32_t isr_vector_table_base;
    SCB->VTOR = (__IO uint32_t) &isr_vector_table_base;
    (void)SCB->VTOR;
    __ISB();

    fwdgt_config(0x0FFF, FWDGT_PSC_DIV4);
    fwdgt_window_value_config(0x0FFF);
    fwdgt_enable(); // Enable watchdog
    fwdgt_counter_reload();

    SystemCoreClockUpdate();
    systick_config();

    __enable_irq();

    extern void setup(void);
    extern void loop(void);
    setup();
    while(1) loop();

    return 0;
}
