/*
   https://www.eachine.com/Eachine-TX801-5_8G-72CH-0_01mW-5mW-25mW-50mW-100mW-200mW-400mW-600mW-Switched-AV-VTX-FPV-Transmitter-p-842.html
*/

#define MAX_POWER             600 // mW

#define SPI_SS                PC6
#define SPI_CLOCK             PC5
#define SPI_DATA              PC7

#define POWER_AMP_1           PA2
#define POWER_AMP_2           PD4
#define POWER_AMP_3           PD3
#define POWER_AMP_4           PD2
#define POWER_AMP_5           PC3
#define POWER_AMP_6           PC4

void rfPowerAmpPinSetup()
{
  pinMode(POWER_AMP_1, OUTPUT);
  pinMode(POWER_AMP_2, OUTPUT);
  pinMode(POWER_AMP_3, OUTPUT);
  pinMode(POWER_AMP_4, OUTPUT);
  pinMode(POWER_AMP_5, OUTPUT);
  pinMode(POWER_AMP_6, OUTPUT);
}

//TODO verify output
void setPower(uint16_t power)
{
  if (power == 0 || pitMode)
  {
    digitalWrite(POWER_AMP_1, LOW);
    digitalWrite(POWER_AMP_2, LOW);
    digitalWrite(POWER_AMP_3, LOW);
    digitalWrite(POWER_AMP_4, LOW);
    digitalWrite(POWER_AMP_5, LOW);
    digitalWrite(POWER_AMP_6, LOW);
  } else if (power == 25)
  {
    digitalWrite(POWER_AMP_1, LOW);
    digitalWrite(POWER_AMP_2, HIGH);
    digitalWrite(POWER_AMP_3, LOW);
    digitalWrite(POWER_AMP_4, LOW);
    digitalWrite(POWER_AMP_5, LOW);
    digitalWrite(POWER_AMP_6, LOW);
  } else if (power == 100)
  {
    digitalWrite(POWER_AMP_1, LOW);
    digitalWrite(POWER_AMP_2, LOW);
    digitalWrite(POWER_AMP_3, LOW);
    digitalWrite(POWER_AMP_4, HIGH);
    digitalWrite(POWER_AMP_5, LOW);
    digitalWrite(POWER_AMP_6, LOW);
  } else if (power == 400)
  {
    digitalWrite(POWER_AMP_1, LOW);
    digitalWrite(POWER_AMP_2, LOW);
    digitalWrite(POWER_AMP_3, LOW);
    digitalWrite(POWER_AMP_4, LOW);
    digitalWrite(POWER_AMP_5, LOW);
    digitalWrite(POWER_AMP_6, HIGH);
  }
}