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
  // {mW, dBm, pwm_val, VpdSetPoint, amp_state, a, b, c, d}
  {0, 0, 3000, 0, 0, 0, 0, 0, 0},
  {25, 14, 2381, 590, 1, 922700, 473.3921, 0.08096176, 0.000004612795},
  {50, 17, 2360, 830, 1, 1863792, 957.7339, 0.1640354, 0.000009360269},
  {100, 20, 2344, 1200, 1, 2530390, 1295.161, 0.2209582, 0.00001255892},
  {400, 26, 0, 9999, 1, 9999, 0, 0, 0}, // This is max power and about 500mW
};

// https://mycurvefit.com/
// y = 922700 - 473.3921*x + 0.08096176*x^2 - 0.000004612795*x^3 25mW
// y = 1863792 - 957.7339*x + 0.1640354*x^2 - 0.000009360269*x^3 50mw
// y = 2530390 - 1295.161*x + 0.2209582*x^2 - 0.00001255892*x^3  100mW

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

  if (index == 0xff)
    return;

  uint16_t tempFreq = myEEPROM.currFreq;
  VpdSetPoint = power_mapping[index].a - power_mapping[index].b*tempFreq + power_mapping[index].c*tempFreq*tempFreq - power_mapping[index].d*tempFreq*tempFreq*tempFreq;

  pwm_val = power_mapping[index].pwm_val;
  amp_state = power_mapping[index].amp_state;

  pwm_out_write(outputPowerTimer, pwm_val);
  gpio_out_write(vref_pin, amp_state);

  #if OUTPUT_POWER_TESTING
  VpdSetPoint = 0;
  pwm_val = 3000;
  pwm_out_write(outputPowerTimer, pwm_val);
  amp_state = 1;
  gpio_out_write(vref_pin, amp_state);
  #endif /* OUTPUT_POWER_TESTING */
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
  if (200 <= (now - temp)) {
    temp = now;
    
    len = snprintf(buff, sizeof(buff), "%lu,%lu,%lu,%lu\n", myEEPROM.currFreq, pwm_val, VpdSetPoint, currentVpd);
    Serial_write_len((uint8_t*)buff, len);  

    #if OUTPUT_POWER_TESTING
    VpdSetPoint = VpdSetPoint + 5;
    if (VpdSetPoint > 2000)
    {
      VpdSetPoint = 0;
      pwm_val = 3000;
      pwm_out_write(outputPowerTimer, pwm_val);
      myEEPROM.currFreq = myEEPROM.currFreq + 50;
      if (myEEPROM.currFreq > 6000)
      {
        myEEPROM.currFreq = 5600;
      }
      rtc6705WriteFrequency(myEEPROM.currFreq);
    }
    #endif /* OUTPUT_POWER_TESTING */
  }
#endif /* DEBUG */

  /* Reset WD */
  fwdgt_counter_reload();
}
