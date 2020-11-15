#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "smartAudio.h"
#include "tramp.h"
#include <Arduino.h>
#include "pwm.h"

uint16_t dCycle;

struct timeout outputPowerTimer;

void setup()
{
//   rfPowerAmpPinSetup();
//   setPowerdB(0);

//   readEEPROM();
//   pitMode = (myEEPROM.pitmodeInRange || myEEPROM.pitmodeOutRange) ? 1 : 0;

  spiPinSetup();
  rtc6705ResetState(); // During testing registers got messed up. So now it gets reset on boot!
  rtc6705WriteFrequency(5800);
//   rtc6705WriteFrequency(myEEPROM.currFreq);

//   Serial_begin(myEEPROM.vtxMode == TRAMP ? TRAMP_BAUD : SMARTAUDIO_BAUD);

// #ifdef ARDUINO
//   while (!Serial)
//     ;
//   UART1_HalfDuplexCmd(ENABLE);
// #ifdef SERIAL_PIN
//   pinMode(SERIAL_PIN, INPUT_PULLUP);
// #endif
// #endif

//   // clear any uart garbage
//   clearSerialBuffer();


// pinMode(POWER_AMP_6, OUTPUT);
// uint16_t AutoReloadRegister = 255;//1591; // 10khz
// TIM1->PSCRL = 0x00; // 62kHz
// TIM1->ARRH = (uint8_t)(AutoReloadRegister >> 8);
// TIM1->ARRL = (uint8_t)(AutoReloadRegister);
// unsigned char tmp = TIM1->CCER2 & (uint8_t)(~(TIM1_CCER2_CC4E | TIM1_CCER2_CC4P));
// TIM1->CCER2 = tmp | TIM1_CCER2_CC4E;
// TIM1->CCMR4 = TIM1_OCMODE_PWM1 | TIM1_CCMR_OCxPE;
// TIM1->CCR4H = 0x00; // init with 0 duty cycle e.g. 0 volts
// TIM1->CCR4L = 0x00; // init with 0 duty cycle e.g. 0 volts


// dCycle = 1590; // 20dBm 100mW
// dCycle = 1321; // 17dBm 50mW
// dCycle = 1150; // 14dBm 25mW
// dCycle = 1024; // 2.9V
// dCycle = 1; // 0V
// dCycle = AutoReloadRegister / 2;
// dCycle = 155; // 1.463V
// dCycle = 160; // 1.510V 27mW
// dCycle = 170; // 1.600V 33mW
// dCycle = 175; // 1.643V 34mW
// dCycle = 180; // 1.690V 35mW
// dCycle = 200; // 1.867V 36mW
// dCycle = 210; // 1.962V
// dCycle = 220; // 2.047V
// dCycle = 230; // 2.137V
// dCycle = 255; // 2.344V 44mW

// TIM1->CCR4H = (uint8_t)(dCycle >> 8);
// TIM1->CCR4L = (uint8_t)(dCycle);

  Serial_begin(9600);
  // pinMode(UART_TX, INPUT_PULLUP);

  pinMode(LED, OUTPUT);

  pinMode(VREF, OUTPUT);
  // digitalWrite(VREF, HIGH);
  digitalWrite(VREF, LOW);

  // pinMode(RTC_BIAS, OUTPUT);
  // digitalWrite(RTC_BIAS, HIGH);
  
  outputPowerTimer = pwm_init(RTC_BIAS);

  for (int i = 0; i < 5; i++)
  {
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
    delay(50);
  }
}

void loop()
{
  Serial_write(0);
  Serial_write(1);
  Serial_write(2);
  Serial_write(4);
  Serial_write(8);
  Serial_write(16);
  Serial_write(32);

  pwm_out_write(outputPowerTimer, 2999);

  digitalWrite(LED, HIGH);
  delay(100);

  digitalWrite(LED, LOW);
  delay(100);

  // if (myEEPROM.vtxMode == TRAMP)
  // {
  //   trampProcessSerial();
  // }
  // else
  // {
  //   smartaudioProcessSerial();
  // }

  // writeEEPROM();
}
