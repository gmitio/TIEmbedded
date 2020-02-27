/** Main File: TempSense.c
 *
 * TempSense, a basic temperature sensor/display application
 * designed to run on the TM4C123G to communicate with a
 * Microchip TCN75A I2C Temperature Sensor on I2C0 (SCL PB2, SDA PB3)
 *
 * By Graison Mitio, (C) 2020
 */

#include "TempSense.h"  // Main include for the application; handles all other includes

volatile unsigned short readTemp = 0;   // Flag set by Timer0 interrupt
volatile int temp;   // Variable to store the temperature

volatile char tmp[100]; // Holds received serial data from UART1 interrupt
volatile int i = 0; // Interrupt incrementor
volatile int rx_finish = 0; // Interrupt flag

unsigned long BRDI = 16000000 / (16 * 9600);
unsigned long BRDF = (0.666666 * 64 + 0.5);


// small lame delay
void delay (void) {
    long unsigned i;
    for (i = 0; i < 999999; i++);

}

void ReadTemperature(void)
{
    i2c_transmit(0);    // Ping the sensor for temperature data
    temp = I2C0_MDR_R & 0xFF;   // Store the retreived temperature data
    write_temp(temp);   // Write the updated temperature to the display
    readTemp = 0;   // Bump the flag until the next timer interrupt
}

char temp_str[4];
volatile int test;
void main(void)
{

    init_display(); // Initialize the display to a black background
    i2c_init();     // Initialize I2C0 to communicate with sensor
    init_uart(BRDI, BRDF);  // Initialize UART1 for serial data transmission with its integer and fraction parts of baud rate
    Timer0_init();  // Initialize timer 0 to control the polling of the sensor
    init_gpio();

    ReadTemperature();  // Get the initial reading
    delay();
    ReadTemperature();  // Allow reading to stabilize

    while(1)    // Main program loop
    {

        if(readTemp)        // Check if it's time to read the sensor
        {
            ReadTemperature();
            if(temp > 24)
                GPIO_PORTE_DATA_R |= 0x08;
            else
                GPIO_PORTE_DATA_R &= ~0x08;


        }

        if(rx_finish)  {    // If command received

            if(!strcmp((char *)tmp,"/temperature"))  {  // If /temperature command
                ReadTemperature();
                UART_OutString(itotemp(temp));
                UART_OutChar('\r'); // Followed by return character
            }

            memset((char *)tmp, '\0', 100); // Clear the command buffer
            rx_finish = 0;  // Reset interrupt flag
        }


    }

}

// Timer0A ISR, sets flag to poll temperature sensor
void Timer0A_Handler(void)
{
    readTemp = 1;

    TIMER0_ICR_R = 0x01;    // Acknowledge the timer interrupt
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

