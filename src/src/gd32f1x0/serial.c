#include "Arduino.h"
#define SKIP_CODE
#include "targets.h"
#undef SKIP_CODE
#include <gd32f1x0_gpio.h>
#include <gd32f1x0_usart.h>
#include <gd32f1x0_rcu.h>

static volatile uint8_t rx_buffer[64];
static volatile uint8_t rx_head, rx_tail;

void USARTx_IRQHandler(uint32_t usart_periph)
{
    if (RESET != usart_interrupt_flag_get(usart_periph, USART_INT_FLAG_RBNE)) {
        /* receive data */
        uint8_t next = rx_head;
        uint8_t data = usart_data_receive(usart_periph);
        if (((next + 1) % sizeof(rx_buffer)) != rx_tail) {
            rx_buffer[next] = data;
            rx_head = (next + 1) % sizeof(rx_buffer);
        }
    }
}


void USART0_IRQHandler(void)
{
    USARTx_IRQHandler(USART0);
}

void USART1_IRQHandler(void)
{
    USARTx_IRQHandler(USART1);
}

static void config_uart_pin(uint32_t pin, uint32_t af)
{
    uint32_t gpio_periph = GPIO_BASE + 0x400 * GPIO2PORT(pin);
    uint32_t gpio_pin = 0x1 << GPIO2IDX(pin);

    /* enable the clock */
    rcu_periph_clock_enable(RCU_REGIDX_BIT(IDX_AHBEN, (17U + GPIO2PORT(pin))));
    /* connect port to USARTx */
    gpio_af_set(gpio_periph, af, gpio_pin);
    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(gpio_periph, GPIO_MODE_AF, GPIO_PUPD_PULLUP, gpio_pin);
    gpio_output_options_set(gpio_periph, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ,
                            gpio_pin);
}

void Serial_begin(uint32_t baud)
{
    /* TODO/FIXME: Configure USARTx and AF according to pins! */
    uint32_t usart_periph = USART1;
    uint32_t usart_af = GPIO_AF_1;

    config_uart_pin(UART_RX, usart_af);
    config_uart_pin(UART_TX, usart_af);

    /* enable USART clock */
    rcu_periph_clock_enable((usart_periph == USART1) ? RCU_USART1 : RCU_USART0);

    /* USART configure */
    usart_deinit(usart_periph);
    /* 8N1 (standard) */
    usart_baudrate_set(usart_periph, baud);
    usart_parity_config(usart_periph, USART_PM_NONE);
    usart_word_length_set(usart_periph, USART_WL_8BIT);
    usart_stop_bit_set(usart_periph, USART_STB_1BIT);
    usart_transmit_config(usart_periph, USART_TRANSMIT_ENABLE);
    usart_receive_config(usart_periph, USART_RECEIVE_ENABLE);
    usart_interrupt_enable(usart_periph, USART_INT_RBNE);
    usart_enable(usart_periph);

    nvic_irq_enable((usart_periph == USART1) ? USART1_IRQn : USART0_IRQn, 0, 0);
}

uint8_t Serial_available(void)
{
  //uint8_t head = rx_head, tail = rx_tail;
  //return (uint8_t)(head - tail);
  return (rx_head != rx_tail);
}

uint8_t Serial_read(void)
{
    uint8_t data = rx_buffer[rx_tail++];
    rx_tail %= sizeof(rx_buffer);
    return data;
}

void Serial_write(uint8_t data)
{
    /* wait until end of transmit */
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE));
    usart_data_transmit(USART1, data);
}

void Serial_flush(void)
{
    // not needed...
}

void UART1_HalfDuplexCmd(uint8_t state)
{
    (void)state;
    usart_halfduplex_enable(USART1);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
}
