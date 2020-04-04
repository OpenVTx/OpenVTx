#include <Arduino.h>
#include <EEPROM.h>

char rxPacket[16] = {0};
char txPacket[16] = {0};

uint16_t currFreq = 5800;
uint16_t currPower = 25;
uint16_t temperature = 69;
uint8_t pitMode = 0;

/*
 * New targets can be added in the targets folder and added to targets.h
 * Uncomment your VTx.
 */
#define EACHINE_TX801
//#define EACHINE_XXXXX
//#define EACHINE_YYYYY

#include "targets.h"
#include "rtc6705.h"

void emptyUartBuffers()
{
  while (Serial_available())
  {
    Serial_read();
  }
  Serial_flush();
}

void zeroRxPacket()
{
  memcpy(rxPacket, 0, 16);
}

void zeroTxPacket()
{
  memcpy(txPacket, 0, 16);
}

uint8_t calcCrc(uint8_t *packet)
{
    uint8_t crc = 0;
    
    for (int i = 1 ; i < 14 ; i++) {
        crc += packet[i];
    }

    return crc;
}

void trampSendPacket()
{
  for (int i=0; i<16; i++)
  {
    Serial_write(txPacket[i]);
  }
}

void buildrPacket()
{
  zeroTxPacket();
  txPacket[0] = 15;
  txPacket[1] = 'r';
  txPacket[2] = MIN_FREQ & 0xff;
  txPacket[3] = (MIN_FREQ >> 8) & 0xff;
  txPacket[4] = MAX_FREQ & 0xff;
  txPacket[5] = (MAX_FREQ >> 8) & 0xff;
  txPacket[6] = MAX_POWER & 0xff;
  txPacket[7] = (MAX_POWER >> 8) & 0xff;
  txPacket[14] = calcCrc(txPacket);  
}

void buildvPacket()
{
  zeroTxPacket();
  txPacket[0] = 15;
  txPacket[1] = 'v';
  txPacket[2] = currFreq & 0xff;
  txPacket[3] = (currFreq >> 8) & 0xff;
  txPacket[4] = currPower & 0xff;         // Configured transmitting power
  txPacket[5] = (currPower >> 8) & 0xff;  // Configured transmitting power
  txPacket[6] = 0;                        // trampControlMode
  txPacket[7] = pitMode;                  // trampPitMode
  txPacket[8] = currPower & 0xff;         // Actual transmitting power
  txPacket[9] = (currPower >> 8) & 0xff;  // Actual transmitting power
  txPacket[14] = calcCrc(txPacket);  
}

void buildsPacket()
{
  zeroTxPacket();
  txPacket[0] = 15;
  txPacket[1] = 's';
  txPacket[6] = temperature & 0xff;
  txPacket[7] = (temperature >> 8) & 0xff;
  txPacket[14] = calcCrc(txPacket);  
}

void processFPacket()
{
  currFreq = rxPacket[2]|(rxPacket[3] << 8);  
  
  rtc6705PowerAmpOn(false);
  setPower(0);
  rtc6705WriteFrequency(currFreq);
  rtc6705PowerAmpOn(true);
  setPower(currPower);
  
  EEPROM_put(0, currFreq);
}

void processPPacket()
{
  currPower = rxPacket[2]|(rxPacket[3] << 8);
  setPower(currPower);
  
  EEPROM_put(2, currPower);
}

void processIPacket()
{
  pitMode = rxPacket[2];
//  //TODO - fix and finish
//  if (pitMode)
//  {
//    setPitModePower(true);
//    setPower(0);
//  } else
//  {
//    setPitModePower(false); 
//    setPower(currPower);    
//  }
}

void setup()
{
  EEPROM_get(0, currFreq);
  EEPROM_get(2, currPower);
  
  Serial_begin(9600);
  while(!Serial)
  {
    ;
  }
  UART1_HalfDuplexCmd(ENABLE);
  pinMode(PD5, INPUT_PULLUP);

  spiPinSetup();

  rfPowerAmpPinSetup();
  setPower(0);

  // During testing this register got messed up. So now it gets reset on boot!
  rtc6705ResetState(); 
    
  // Spam rtc6705 with PA off cmd to try and have clean powerup
  // Need to check with a spectrum analyser
  while(millis() < 1000)
  {
    rtc6705PowerAmpOn(false);
    rtc6705WriteFrequency(currFreq); 
  }
  rtc6705PowerAmpOn(true);
  setPower(currPower);

  // clear any uart garbage
  emptyUartBuffers();
}

void loop()
{
  if (Serial_available() == 15) // stm8s buffer is 16 bytes but this doesnt work when == 16 :/
  {
    // delay to prevent rx/tx collisions
    delay(100); 

    // read in buffer
    for (int i=0; i<15; i++)
    {
      rxPacket[i] = Serial_read();
    }

    // packet type and check crc
    if (rxPacket[0] == 15) // tramp header
    {
      if(rxPacket[14] == calcCrc(rxPacket))
      {
        switch(rxPacket[1]) // command
        {
          case 'F':
            processFPacket();
            buildvPacket();
            trampSendPacket();
            break;
          case 'P':
            processPPacket();
            buildvPacket();
            trampSendPacket();
            break;
          case 'I':
            processIPacket();
            buildvPacket();
            trampSendPacket();
            break;
          case 'r':
            buildrPacket();
            trampSendPacket();
            break;
          case 'v':
            buildvPacket();
            trampSendPacket();
            break;
          case 's':
            buildsPacket();
            trampSendPacket();
            break;
        }
      }
    }
    
    // return to make serial monitor readable when debuging
//     Serial_print_c('\n');

    // wait for tx and then clear buffer so serial_read does not read it back in.
    delay(20);
    emptyUartBuffers();
  }
}
