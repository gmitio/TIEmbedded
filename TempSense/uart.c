/*
 * uart.c
 *
 *  Created on: Jan 18, 2020
 *      Author: Graison Mitio
 */

#include "TempSense.h"

void init_uart(unsigned long BRDI, unsigned long BRDF)
{

    // Enable UART1 (PB0, PB1)
    volatile unsigned long delay_clk;
    SYSCTL_RCGCUART_R |= 0x02;  // Enable UART1
    SYSCTL_RCGCGPIO_R |= 0x22;  // Enable GPIO pins for UART1 (PORT B and PORT F for general stuff)

    delay_clk = SYSCTL_RCGCGPIO_R;

    GPIO_PORTB_AFSEL_R |= 0x03;
    //GPIO_PORTB_DR8R_R |= 0x03;  // Enable 8mA drive
    GPIO_PORTB_DEN_R |= 0x03;
    //GPIO_PORTB_SLR_R |= 0x03;   // Enable slew rate control
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFFFF00) | 0x11;    // Set port control for UART

    UART1_CTL_R &= ~0x01;   // Disable the UART
    UART1_IBRD_R = BRDI;
    UART1_FBRD_R = BRDF;
    UART1_LCRH_R |= 0x60;
    //UART1_CC_R = 0x00;
    UART1_IM_R |= 0x10; // Enable RX interrupt
    UART1_CTL_R |= 0x11;    // Enable the UART

    NVIC_EN0_R = 1<<6; // Enable interrupt 6 for UART1
    NVIC_PRI1_R = (NVIC_PRI1_R & 0xFF0FFFFF) | 0x00A00000;    // Set priority level 5

}



void UART_OutString(char *string_out)
{
    int j = 0;
    for(j = 0; j < strlen(string_out); j++)
    {
        while( (UART1_FR_R & 0x0020) != 0 );
            UART1_DR_R = string_out[j];
    }
}

void UART_OutChar(unsigned char data)   // Writes to the serial data
{
    while( (UART1_FR_R & 0x0020) != 0 );
    UART1_DR_R = data;
}
