#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "gpio.h"
// #include "pwm.h"
// #include "printf.h"
#include "helpers.h"
#include <math.h>
#include "serial.h"

#define OUTPUT_POWER_INTERVAL 5 // ms

gpio_pwm_t outputPowerTimer;
gpio_out_t vref_pin;
gpio_adc_t vpd_pin;
uint32_t currentVpd = 0;

uint16_t pwm_val = 2500;
uint16_t VpdSetPoint = 0;
uint8_t amp_state = 0;

#define CAL_FREQ_SIZE 9
#define CAL_DBM_SIZE 17
uint16_t calFreqs[CAL_FREQ_SIZE] = {5600,	5650,	5700,	5750,	5800,	5850,	5900, 5950, 6000};
uint8_t calDBm[CAL_DBM_SIZE] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
uint16_t calVpd[CAL_DBM_SIZE][CAL_FREQ_SIZE] = {
{415, 375, 375, 380, 380, 380, 395, 390, 400},  // 10
{420, 420, 400, 400, 405, 425, 430, 440, 435},  // 11
{470, 455, 445, 445, 455, 465, 485, 490, 490},  // 12
{525, 510, 490, 490, 500, 515, 540, 550, 545},  // 13
{575, 565, 555, 545, 560, 575, 595, 600, 600},  // 14
{640, 630, 610, 610, 625, 645, 665, 675, 670},  // 15
{730, 710, 685, 680, 690, 725, 750, 755, 755},  // 16
{805, 785, 755, 755, 770, 805, 830, 840, 835},  // 17
{905, 875, 850, 845, 860, 895, 910, 930, 935},  // 18
{1020, 995, 955, 945, 960, 995, 1030, 1045, 1055},       // 19
{1180, 1115, 1075, 1065, 1080, 1125, 1150, 1175, 1165},  // 20
{1335, 1260, 1230, 1210, 1235, 1260, 1295, 1310, 1320},  // 21
{1415, 1395, 1355, 1355, 1355, 1390, 1395, 1400, 1390},  // 22
{1440, 1430, 1430, 1430, 1435, 1435, 1440, 1430, 1425},  // 23
{1440, 1440, 1450, 1445, 1450, 1450, 1450, 1440, 1435},  // 24
{1450, 1450, 1450, 1455, 1450, 1450, 1450, 1450, 1435},  // 25
{1450, 1450, 1450, 1455, 1465, 1460, 1460, 1450, 1445}   // 26
};

uint16_t bilinearInterpolation(float dB)
{

  dB = dB + OFFSET;

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
  for (uint8_t j = 0; j < (CAL_DBM_SIZE-1); j++)
  {
    if (dB < calDBm[j + 1])
    {
      calDBmIndex = j;
      break;
    }
  }

  float x = dB;
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
  pwm_out_write(RTC_BIAS, 255); // 3 uW
  
  // vref_pin = gpio_out_setup(VREF, 1); // Power amp ON
  // pwm_out_write(RTC_BIAS, 0); // 33 mW

  vpd_pin = adc_config(VPD);
}

uint32_t vpd_value_get(void)
{
  return adc_read(vpd_pin);
}

void increasePWMVal()
{
  if (pwm_val < 2500)
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

void target_set_power_dB(float dB)
{

  
  // pwm_out_write(outputPowerTimer, 0);
  // gpio_out_write(vref_pin, 0);


  // if (dB < 10)
  // {
  //   VpdSetPoint = 0;
  //   pwm_val = 2500;
  //   amp_state = 0;
  // } else if (dB >= 26)
  // {
  //   VpdSetPoint = 1500;
  //   pwm_val = 2000;
  //   amp_state = 1;
  // } else
  // {
  //   VpdSetPoint = bilinearInterpolation(dB);
  //   amp_state = 1;
  // }
  
  // pwm_out_write(outputPowerTimer, pwm_val);
  // gpio_out_write(vref_pin, amp_state);

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
    
    // len = snprintf(buff, sizeof(buff), "%lu,%lu,%lu,%lu\n", myEEPROM.currFreq, pwm_val, VpdSetPoint, currentVpd);
    // len = snprintf(buff, sizeof(buff), "%lu\n", currentVpd);
    // Serial_write_len((uint8_t*)buff, len);  


    // currentVpd = 85;

    Serial_write(48 + (currentVpd / 1000));  
    Serial_write(48 + (currentVpd % 1000) / 100);  
    Serial_write(48 + (currentVpd % 100) / 10);  
    Serial_write(48 + (currentVpd % 10));  
    // Serial_write_len((uint8_t*)currentVpd, 4);
    // Serial_write_len("\r\n", 3);

    // Serial_write_len("AA", 3);
    Serial_write_len("\r\n", 3);

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
}
