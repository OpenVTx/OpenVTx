
#define SynthesizerRegisterB          0x01
#define PredriverandPAControlRegister 0x07
#define StateRegister                 0x0F

#define MIN_FREQ              5000
#define MAX_FREQ              5999

void spiPinSetup()
{
  pinMode(SPI_CLOCK, OUTPUT);
  digitalWrite(SPI_CLOCK, LOW);
  pinMode(SPI_DATA, OUTPUT);
  digitalWrite(SPI_DATA, LOW);
  pinMode(SPI_SS, OUTPUT);  
  digitalWrite(SPI_SS, HIGH);
}

void sendBits(long data)
{
  digitalWrite(SPI_SS, LOW);
  delayMicroseconds(1);
  
  for (uint8_t i = 0; i < 25; i++)
  {        
    digitalWrite(SPI_CLOCK, LOW);
    delayMicroseconds(1);
    digitalWrite(SPI_DATA, data & 0x1);
    delayMicroseconds(1);
    digitalWrite(SPI_CLOCK, HIGH);
    delayMicroseconds(1);
    
    data >>= 1;
  }

  digitalWrite(SPI_CLOCK, LOW);
  digitalWrite(SPI_DATA, LOW);
  digitalWrite(SPI_SS, HIGH);
  delayMicroseconds(1);

  return;
}

void rtc6705ResetState() 
{
  long data = StateRegister | (1 << 4) | (0b0 << 5);   
  sendBits(data);
}

void rtc6705PowerAmpOn(bool On)
{  
  if(On)
  {  
    long data = PredriverandPAControlRegister | (1 << 4) | (0b00000100111110111101 << 5);   
    sendBits(data);    
  } else
  { 
    long data = PredriverandPAControlRegister | (1 << 4) | (0b00000100000000111101 << 5);  
    sendBits(data);    
  }

  return;  
}

//void setPitModePower(bool On)
//{
//  if(On)
//  {  
//    long data = PredriverandPAControlRegister | (1 << 4) | (0b00000100111110111101 << 5);   
//    sendBits(data);    
//  } else
//  { 
//    long data = PredriverandPAControlRegister | (1 << 4) | (0b00000100000000111101 << 5);  
//    sendBits(data);    
//  }
//
//  return;  
//}

void rtc6705WriteFrequency(long newFreq)
{  
  long freq = 1000 * newFreq;
  freq /= 40;  
  long SYN_RF_N_REG = freq / 64;    
  long SYN_RF_A_REG = freq % 64;
  
  long data = SynthesizerRegisterB | (1 << 4) | (SYN_RF_A_REG << 5) | (SYN_RF_N_REG << 12);

  sendBits(data);

  return;  
}
