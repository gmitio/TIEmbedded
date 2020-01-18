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
int temp;   // Variable to store the temperature

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

int main(void)
{
    init_display(); // Initialize the display to a black background
    i2c_init();     // Initialize I2C0 to communicate with sensor
    Timer0_init();  // Initialize timer 0 to control the polling of the sensor

    ReadTemperature();  // Get the initial reading
    delay();
    ReadTemperature();  // Allow reading to stabilize

    while(1)    // Main program loop
    {

        if(readTemp)    // Check if it's time to read the sensor
            ReadTemperature();

    }
	return 0;   // Should never get here
}

// Timer0A ISR, sets flag to poll temperature sensor
void Timer0A_Handler(void)
{
    readTemp = 1;

    TIMER0_ICR_R = 0x01;    // Acknowledge the timer interrupt
}

