#include "tm4c123gh6pm.h"
#include <string.h>
#include <stdio.h>
#include <ST7735.h>
#include <stdlib.h>
/**
 * main.c for UART serial control
 * PC4 RX and PC5 TX
 */


unsigned long BRDI = 16000000 / (16 * 9600);
unsigned long BRDF = (0.666666 * 64 + 0.5);

volatile char tmp[100]; // Holds received serial data from UART1 interrupt
volatile int i = 0; // Interrupt incrementor
volatile int rx_finish = 0; // Interrupt flag

void init_uart(void)
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

void init_adc(void) // Initializes the Analog/Digital Converter on PE4
{
    volatile unsigned long dummy_clk;
    SYSCTL_RCGCADC_R |= 0x01;   // Enable clocking on RCGCADC register
    SYSCTL_RCGCGPIO_R |= 0x10;  // Enable clocking on RCGCGPIO
    dummy_clk = SYSCTL_RCGCGPIO_R;  // NOP instruction to let the clock enable

    GPIO_PORTE_AFSEL_R |= 0x10; // Set PE4 AFSEL bit low
    GPIO_PORTE_DEN_R &= ~0x10;  // Set PE4 DEN low
    GPIO_PORTE_AMSEL_R |= 0x10; // Set PE4 AMSEL high

    ADC0_SSPRI_R = 0x0123; // SS3 lowest, SS0 highest priority
    ADC0_ACTSS_R &= ~0x08;   // Disable the sample sequencer
    ADC0_PC_R |= 0x01;  // Set sampling rate 125khz
    ADC0_EMUX_R &= ~0xFFFF; // Configure the trigger event in ADCEMUX, default
    ADC0_SSMUX3_R &= ~0x0F;   // Clear SS3 1st sample input select field
    ADC0_SSMUX3_R |= 0x09;  // Configure the corresponding input in ADCSSMUX
    ADC0_SSCTL3_R = 0x06;   // Configure the sample control bits
    ADC0_IM_R &= ~0x08;  // If interrupts are used, sed the mask in ADCIM
    ADC0_SAC_R = 0x06;
    ADC0_ACTSS_R |=  0x08;  // Enable sample sequencer

    NVIC_EN0_R |= 0x20000;
    NVIC_PRI4_R = (NVIC_PRI4_R & 0xFFFF00FF) | 0x00004000;  // Set priority 2

}

unsigned long read_adc(void)    // Reads the Analog/Digital Converter on PE4
{
    unsigned long result;
    ADC0_PSSI_R = 0x08;
    while( !(ADC0_RIS_R & 0x08) ) {};   // Wait for read
    result = ADC0_SSFIFO3_R & 0xFFA;
    ADC0_ISC_R = 0x08;  // Clear the interrupt
    return result;
}

void init_gpio(void)    // Some brief initialization for the GPIO to use leds (NOTE: clock gating is handled in the init_uart() function)
{
    GPIO_PORTF_DEN_R |= 0x04;
    GPIO_PORTF_DIR_R |= 0x04;
    GPIO_PORTF_AFSEL_R &= ~0x04;
}

void init_display(void)
{
    /** Initialize **/
    ST7735_InitR(INITR_BLACKTAB);
    ST7735_FillScreen(ST7735_BLACK);    // Background color
    ST7735_SetRotation(1);  // Horizontal
}

void write_display(char *writeThis, unsigned int length)    // Fix this using strlen()?
{
    unsigned int j,k,l;
    k = 0;
    l = 0;
    for(j = 0; j < length; j++)
    {
        ST7735_DrawChar(10+10*l, 10+10*k, writeThis[j], ST7735_WHITE, ST7735_BLACK, 1);
        l++;
        if(l > 12)  // Overflow condition; provides wrap logic
        {
            l = 0;
            k++;
        }

    }
}

void UART_OutChar(unsigned char data)   // Writes to the serial data
{
    while( (UART1_FR_R & 0x0020) != 0 );
    UART1_DR_R = data;
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

unsigned char num2char(unsigned int num)    // Converts unsigned number to ASCII char for display
{
    if(num <=0x09)
        return (num+0x30);

    else
        return (0x30+9);
}

unsigned char data_in = 0;

char finals[100];   // Holds the string of data read from UART1
char clear[100];    // Holds a string of whitespace to clear messages from the display upon '/clear' command


int adcval;//Declared global for debugging
int main(void)
{

    init_display();
    init_uart();
    init_adc();
    init_gpio();

    memset(clear,' ', 100); // Set the whitespace array up

    while(1)
    {
        if(rx_finish && (strcmp(tmp,"/clear") != 0) && (strcmp(tmp,"/readadc") != 0) )
        {
            GPIO_PORTF_DATA_R |= 0x04;    // Set LED to show transmission completed
            strcpy(finals,tmp);
            memset(tmp, '\0', 100);
            rx_finish = 0;  // Reset the flag
            write_display(finals,128);
        }

        if(rx_finish && !strcmp(tmp,"/clear") ) // Function to clear the display
        {
            write_display(clear,100);
            memset(tmp, '\0', 100);
            memset(finals, '\0', 100);
            rx_finish=0;
        }

        if(rx_finish && !strcmp(tmp,"/readadc") )   // Function to read a decimal value from the PE4 ADC
        {

            adcval = read_adc();
            char adc_str[12];
            adc_str[0] = num2char((adcval/1000) % 10);
            adc_str[1] = num2char((adcval/100) % 10);
            adc_str[2] = num2char((adcval/10) % 10);
            adc_str[3] = num2char(adcval % 10);

            write_display(adc_str,4);
            UART_OutString(adc_str);
            UART_OutChar('\r');
            memset(tmp, '\0', 100);
            memset(finals, '\0', 100);
            rx_finish=0;
        }
    }
}

void UART1_Handler(void)
{
    // If receive MIS status is set, write the data to a buffer like in the AVRTest program
    if(UART1_MIS_R & 0x10)
    {
        tmp[i] = UART1_DR_R & 0xFF; // Write lowest byte of FIFO to the buffer

        if(tmp[i] == '\r') // Check for end of message
        {
            rx_finish = 1;  // Set flags and reset iterator
            tmp[i] = 0;
            i = 0;
        }

        else
            i++;    // Else read the next character
    }

    UART1_ICR_R |= 0x10;  // Clear RX interrupt
}

void SysTick_Handler(void)  // Here because idfk
{
}
