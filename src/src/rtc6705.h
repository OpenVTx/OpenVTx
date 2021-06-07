#pragma once

#include <stdint.h>

#define SynthesizerRegisterA 0x00
#define SynthesizerRegisterB 0x01
#define SynthesizerRegisterC 0x02
#define RFVCODFCControlRegister 0x03
#define VCOControlRegister 0x04
#define VCOControlRegisterCont 0x05
#define AudioModulatorControlRegister 0x06
#define PredriverandPAControlRegister 0x07
#define StateRegister 0x0F

#define MIN_FREQ 5000
#define MAX_FREQ 5999

#define PLL_SETTLE_TIME 500

void spiPinSetup(void);
void sendBits(uint32_t data);
void rtc6705ResetState(void);
void rtc6705PowerAmpOn(void);
void rtc6705PowerAmpOff(void);
void rtc6705WriteFrequency(uint32_t newFreq);
void rtc6705PowerUpAfterPLLSettleTime();
