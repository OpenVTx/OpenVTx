#include "pwm.h"
#include "Arduino.h"
#include "gd32f1x0_timer.h"
#include "gd32f1x0_rcu.h"
#define SKIP_CODE
#include "targets.h"
#undef SKIP_CODE

#if PWM_BIAS_FREQ > 1000000
#error "Too high PWM freq!"
#endif

struct pwms {
    uint32_t periph;
    uint32_t pin;
    uint8_t ch, af;
};
struct pwms pwm_config[] = {
    {TIMER1,  PA0,  0, 2},
    {TIMER1,  PA1,  1, 2},
    {TIMER1,  PA2,  2, 2},
    {TIMER14, PA2,  0, 0},
    {TIMER1,  PA3,  3, 2},
    {TIMER14, PA3,  1, 0},
    {TIMER13, PA4,  0, 4},
    {TIMER1,  PA5,  0, 2},
    {TIMER2,  PA6,  0, 1},
    {TIMER15, PA6,  0, 5},
    {TIMER2,  PA7,  1, 1},
    {TIMER13, PA7,  0, 4},
    {TIMER16, PA7,  0, 5},
    {TIMER0,  PA8,  0, 2},
    {TIMER0,  PA9,  1, 2},
    {TIMER0,  PA10, 2, 2},
    {TIMER0,  PA11, 3, 2},
    {TIMER1,  PA15, 0, 2},
    {TIMER2,  PB0,  2, 1},
    {TIMER2,  PB1,  3, 1},
    {TIMER1,  PB3,  1, 2},
    {TIMER2,  PB4,  0, 1},
    {TIMER2,  PB5,  1, 1}, // {TIMER16, PB5,  1, 2},
    {TIMER15, PB8,  0, 2},
    {TIMER16, PB9,  0, 2},
    {TIMER1,  PB10, 2, 2},
    {TIMER1,  PB11, 3, 2},
};

void timer_power_on(uint32_t timer_periph)
{
    switch(timer_periph){
    case TIMER0:
        rcu_periph_clock_enable(RCU_TIMER0);
        break;
    case TIMER1:
        rcu_periph_clock_enable(RCU_TIMER1);
        break;
    case TIMER2:
        rcu_periph_clock_enable(RCU_TIMER2);
        break;
    case TIMER5:
        rcu_periph_clock_enable(RCU_TIMER5);
        break;
    case TIMER13:
        rcu_periph_clock_enable(RCU_TIMER13);
        break;
    case TIMER14:
        rcu_periph_clock_enable(RCU_TIMER14);
        break;
    case TIMER15:
        rcu_periph_clock_enable(RCU_TIMER15);
        break;
    case TIMER16:
        rcu_periph_clock_enable(RCU_TIMER16);
        break;
    default:
        break;
    }
}

struct timeout pwm_init(uint32_t pin)
{
    // uint8_t pwmConfigIndex = 0;
    // for (pwmConfigIndex = 0; pwmConfigIndex < ARRAY_SIZE(pwm_config); pwmConfigIndex++)
    // {
    //     if (pin == pwm_config[pwmConfigIndex].pin)
    //         break;
    // }

    uint32_t timer = TIMER2;
    // uint32_t timer = pwm_config[pwmConfigIndex].periph;
    uint8_t channel = 1;
    // uint8_t channel = pwm_config[pwmConfigIndex].ch;

    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_5);

    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    // if (channel < ARRAY_SIZE(pwm_config))
    // {
        /* Init timer if not done yet */
        // if (!(TIMER_CTL0(timer) & TIMER_COUNTER_ENABLE))
        // {
            timer_power_on(timer);
            timer_deinit(timer);
            /* TIMERx configuration */
            timer_initpara.prescaler = 1;  // High clk required to provide the PWM resolution required.
            timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
            timer_initpara.counterdirection = TIMER_COUNTER_UP;
            timer_initpara.period = 2999; // Results in ~12kHz PWM
            timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
            timer_initpara.repetitioncounter = 0;
            timer_init(timer, &timer_initpara);
        // }

        /* Init timer channel PWM0 mode */
        timer_ocintpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
        timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
        
        timer_channel_output_config(timer, channel, &timer_ocintpara);

        timer_channel_output_pulse_value_config(timer, channel, timer_initpara.period);
        timer_channel_output_mode_config(timer, channel, TIMER_OC_MODE_PWM0);
        timer_channel_output_shadow_config(timer, channel, TIMER_OC_SHADOW_DISABLE);

        /* auto-reload preload enable */
        timer_auto_reload_shadow_enable(timer);
        /* auto-reload preload enable */
        timer_enable(timer);

    // }
    return (struct timeout){.tim = timer, .ch = channel};
}

void pwm_out_write(struct timeout pwm, int val)
{
    // if (pwm.tim) {
    //     val = (val <= 0) ? 0 : ((val <= 100) ? (val - 1) : 100);
        timer_channel_output_pulse_value_config(pwm.tim, pwm.ch, val);
    // }
}
