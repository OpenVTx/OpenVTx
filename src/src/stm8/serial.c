#include "serial.h"
#include "targets.h"

void serial_begin(uint32_t baud, uint32_t tx_pin, uint32_t rx_pin)
{
    uint8_t halfduplex = (tx_pin == rx_pin);
    Serial_begin(baud);
    while (!Serial)
        ;
    if (halfduplex)
        UART1_HalfDuplexCmd(ENABLE);
#ifdef UART_TX
    pinMode(UART_TX, INPUT_PULLUP);
#endif
}

void Serial_write_len(uint8_t *data, uint32_t size)
{
    while(size--)
        Serial_write(*data++);
}
