#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
//#include "rtc6705.h"
#include "gpio.h"
#include "pwm.h"
#include "printf.h"
#include "helpers.h"
#include <math.h>

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

#define CAL_FREQ_SIZE 9
#define CAL_DBM_SIZE 17
uint16_t calFreqs[CAL_FREQ_SIZE] = {5600,	5650,	5700,	5750,	5800,	5850,	5900, 5950, 6000};
uint8_t calDBm[CAL_DBM_SIZE] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
uint16_t calVpd[CAL_DBM_SIZE][CAL_FREQ_SIZE] = {
{415, 375, 375, 380, 380, 380, 395, 390, 400}, 
{420, 420, 400, 400, 405, 425, 430, 440, 435}, 
{470, 455, 445, 445, 455, 465, 485, 490, 490}, 
{525, 510, 490, 490, 500, 515, 540, 550, 545}, 
{575, 565, 555, 545, 560, 575, 595, 600, 600}, 
{640, 630, 610, 610, 625, 645, 665, 675, 670}, 
{730, 710, 685, 680, 690, 725, 750, 755, 755}, 
{805, 785, 755, 755, 770, 805, 830, 840, 835}, 
{905, 875, 850, 845, 860, 895, 910, 930, 935}, 
{1020, 995, 955, 945, 960, 995, 1030, 1045, 1055}, 
{1180, 1115, 1075, 1065, 1080, 1125, 1150, 1175, 1165}, 
{1335, 1260, 1230, 1210, 1235, 1260, 1295, 1310, 1320}, 
{1415, 1395, 1355, 1355, 1355, 1390, 1395, 1400, 1390}, 
{1440, 1430, 1430, 1430, 1435, 1435, 1440, 1430, 1425}, 
{1440, 1440, 1450, 1445, 1450, 1450, 1450, 1440, 1435}, 
{1450, 1450, 1450, 1455, 1450, 1450, 1450, 1450, 1435}, 
{1450, 1450, 1450, 1455, 1465, 1460, 1460, 1450, 1445}
};

uint16_t bilinearInterpolation(uint8_t power)
{
  float powerInDbm = 10 * log10(power);

  float tempFreq = myEEPROM.currFreq;

  if (tempFreq < 5600) tempFreq = 5600;
  if (tempFreq > 6000) tempFreq = 6000;
  
  uint8_t calFreqsIndex = 0;
  for (uint8_t i = 0; i < (CAL_FREQ_SIZE-1); i++)
  {
    if (tempFreq < calFreqs[i + 1])
    {
      calFreqsIndex = i;
      break;
    }
  }

  uint8_t calDBmIndex = 0;
  for (uint8_t i = 0; i < (CAL_DBM_SIZE-1); i++)
  {
    if (powerInDbm < calDBm[i + 1])
    {
      calDBmIndex = i;
      break;
    }
  }

  float x = powerInDbm;
  float x1 = calDBm[calDBmIndex];
  float x2 = calDBm[calDBmIndex + 1];

  float y = tempFreq;
  float y1 = calFreqs[calFreqsIndex];
  float y2 = calFreqs[calFreqsIndex + 1];

  float Q11 = calVpd[calDBmIndex][calFreqsIndex];
  float Q12 = calVpd[calDBmIndex][calFreqsIndex + 1];
  float Q21 = calVpd[calDBmIndex + 1][calFreqsIndex];
  float Q22 = calVpd[calDBmIndex + 1][calFreqsIndex + 1];

  float fxy1 = Q11 * (x2 - x) / (x2 - x1) + Q21 * (x - x1) / (x2 - x1);
  float fxy2 = Q12 * (x2 - x) / (x2 - x1) + Q22 * (x - x1) / (x2 - x1);

  uint16_t fxy = fxy1 * (y2 - y) / (y2 - y1) + fxy2 * (y - y1) / (y2 - y1);

  return fxy;
}

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
  if (pwm_val > 2000)
  {
    pwm_val--;
    pwm_out_write(outputPowerTimer, pwm_val);
  }
}

void target_set_power_mW(uint16_t power)
{
  // uint8_t index = get_power_index_by_mW(power);

  // if (index == 0xff)
  //   return;

  // uint16_t tempFreq = myEEPROM.currFreq;
  // VpdSetPoint = power_mapping[index].a - power_mapping[index].b*tempFreq + power_mapping[index].c*tempFreq*tempFreq - power_mapping[index].d*tempFreq*tempFreq*tempFreq;

  // pwm_val = power_mapping[index].pwm_val;
  // amp_state = power_mapping[index].amp_state;

  if (power < 10)
  {
    VpdSetPoint = 0;
    pwm_val = 3000;
    amp_state = 0;
  } else if (power >= 400)
  {
    VpdSetPoint = 1500;
    pwm_val = 2000;
    amp_state = 1;
  } else
  {
    VpdSetPoint = bilinearInterpolation(power);
    amp_state = 1;
  }
  
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
