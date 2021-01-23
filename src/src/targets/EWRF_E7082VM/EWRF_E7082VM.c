#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
//#include "rtc6705.h"
#include "gpio.h"
#include "pwm.h"
#include "printf.h"
#include "helpers.h"

#define OUTPUT_POWER_INTERVAL 5 // ms

gpio_pwm_t outputPowerTimer;
gpio_out_t vref_pin;
gpio_adc_t vpd_pin;
uint32_t currentVpd = 0;

uint16_t pwm_val = 3000;
uint16_t VpdSetPoint = 0;
uint8_t amp_state = 0;

struct PowerMapping power_mapping[] = {
  // {mW, dBm, pwm_val, VpdSetPoint, amp_state}
  {0, 0, 3000, 0, 0},
  {25, 14, 2374, 530, 1},
  {50, 17, 2359, 740, 1},
  {100, 20, 2350, 1045, 1},
  {400, 26, 0, 9999, 1}, // This is max power and about 500mW
};

uint8_t power_mapping_size = ARRAY_SIZE(power_mapping);


void target_rfPowerAmpPinSetup(void)
{
  vref_pin = gpio_out_setup(VREF, 0); // Power amp OFF

  outputPowerTimer = pwm_init(RTC_BIAS);
  pwm_out_write(outputPowerTimer, 3000);

  vpd_pin = adc_config(VPD);
}

uint32_t vpd_value_get(void)
{
  return adc_read(vpd_pin);
}

void increasePWMVal()
{
  if (pwm_val < 3000)
  {
    pwm_val++;
    pwm_out_write(outputPowerTimer, pwm_val);
  }
}

void decreasePWMVal()
{
  if (pwm_val > 0)
  {
    pwm_val--;
    pwm_out_write(outputPowerTimer, pwm_val);
  }
}

void target_set_power_mW(uint16_t power)
{
  uint8_t index = get_power_index_by_mW(power);
  if (index < 0xff)
  {
    pwm_val = power_mapping[index].pwm_val;
    VpdSetPoint = power_mapping[index].VpdSetPoint;
    amp_state = power_mapping[index].amp_state;

    pwm_out_write(outputPowerTimer, pwm_val);
    gpio_out_write(vref_pin, amp_state);
  }
}

void checkPowerOutput(void)
{
  static uint32_t temp;
  uint32_t now = millis();
  if (OUTPUT_POWER_INTERVAL <= (now - temp))
  {
    temp = now;
    currentVpd = vpd_value_get();

    if (currentVpd > VpdSetPoint)
    {
      increasePWMVal();
    }
    else if (currentVpd < VpdSetPoint)
    {
      decreasePWMVal();
    }
  }
}

void taget_setup(void)
{
  /* TODO: Configure WDG, fwdgt_config() */
}

void taget_loop(void)
{
#if DEBUG
  static uint32_t temp;
  static char buff[32];
  int len;
  uint32_t now = millis();
  currentVpd = vpd_value_get();
  if (1000 <= (now - temp)) {
    temp = now;
    len = snprintf(buff, sizeof(buff), "a:%lu\n", currentVpd);
    Serial_write_len((uint8_t*)buff, len);
    len = snprintf(buff, sizeof(buff), "p:%lu\n", pwm_val);
    Serial_write_len((uint8_t*)buff, len);  
  }
#endif /* DEBUG */

  /* Reset WD */
  fwdgt_counter_reload();
}
