#include "uart.h"
#include "rcc.h"
#include "nvic.h"
#include "gpio.h"

static volatile command_t last_command = CMD_NONE;

void usart2_init(void)
{
    configure_gpio_for_usart();

    *RCC_APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    // Disable USART
    USART2->CR1 &= ~USART_CR1_UE;

    // Set data length to 8 bits (clear M bit)
    USART2->CR1 &= ~USART_CR1_M;

    // Select 1 stop bit (clear STOP bits in CR2)
    USART2->CR2 &= ~USART_CR2_STOP;

    // Set parity control as no parity (clear PCE bit)
    USART2->CR1 &= ~USART_CR1_PCE;

    // Oversampling by 16 (clear OVER8 bit)
    USART2->CR1 &= ~USART_CR1_OVER8;

    // Set Baud rate to 9600 using APB frequency (4 MHz)
    USART2->BRR = BAUD_9600_4MHZ;

    // Enable transmission and reception
    USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    // Enable USART
    USART2->CR1 |= USART_CR1_UE;

    // Verify that USART is ready for transmission
    while ((USART2->ISR & USART_ISR_TEACK) == 0);

    // Verify that USART is ready for reception
    while ((USART2->ISR & USART_ISR_REACK) == 0);


    // TODO: Configurar UART2

    // Activar interrupción de RXNE
    USART2->CR1 |= USART_CR1_RXNEIE; 
    NVIC->ISER[1] |= (1 << 6);
}

void usart2_send_char(const char ch) {
    // Wait until transmit data register is empty
    while ((USART2->ISR & (1 << 7)) == 0);

    // Send character
    USART2->TDR = ch;
}

void usart2_send_string(const char *str)
{
    while (*str) {
        while (!(USART2->ISR & USART_ISR_TXE));
        USART2->TDR = *str++;
    }
}

uint8_t  usart2_receive_char() {
    // Wait until data is received
    while ((USART2->ISR & (1 << 5)) == 0);

    // Read received data
    return USART2->RDR;
}

void usart2_receive_string(uint8_t *buffer, uint8_t len) {
    uint8_t i = 0;
    while (i < len) {
        buffer[i] = usart2_receive_char();
        i++;
    }
}

command_t usart2_get_command(void)
{
    command_t cmd = last_command;
    last_command = CMD_NONE;
    return cmd;
}

uint8_t *rx_buffer;
uint8_t rx_len;
uint8_t rx_index;
uint8_t rx_ready;

void usart2_receive_it(uint8_t *buffer, uint8_t len)
{
    // Enable receive interrupt
    USART2->CR1 |= (1 << 5);
    // Set buffer and length
    rx_buffer = buffer;
    rx_len = len;
    rx_index = 0;
}

void USART2_IRQHandler(void)
{
    uint32_t isr = USART2->ISR;
    if (isr & USART_ISR_RXNE) {
        char command = USART2->RDR;
        if (command == 'O') {
            last_command = CMD_OPEN;
        } else if (command == 'C') {
            last_command = CMD_CLOSE;
        }
    }
}