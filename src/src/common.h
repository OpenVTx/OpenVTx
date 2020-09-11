
void clearSerialBuffer()
{
  while (Serial_available())
  {
    Serial_read();
  }
}

void zeroRxPacket()
{
  for (uint8_t i = 0; i < 16; i++)
  {
    rxPacket[i] = 0;
  }
}

void zeroTxPacket()
{
  for (uint8_t i = 0; i < 18; i++)
  {
    txPacket[i] = 0;
  }
}
