/** Main File: TempSense.c
 *
 * TempSense, a basic temperature sensor/display application
 * designed to run on the TM4C123G to communicate with a
 * Microchip TCN75A I2C Temperature Sensor on I2C0 (SCL PB2, SDA PB3)
 *
 * By Graison Mitio, (C) 2020
 */

#include "TempSense.h"  // Main include for the application; handles all other includes


// small lame delay
void delay (void) {
    long unsigned i;
    for (i = 0; i < 999999; i++);

}


int main(void)
{
    init_display(); // Initialize the display to a black background
    i2c_init();     // Initialize I2C0 to communicate with sensor
    int temp;   // Variable to store the temperature
    //write_display("Test message");    // Debugging
    while(1)    // Main program loop
    {
        i2c_transmit(0);    // Ping the sensor for temperature data
        temp = I2C0_MDR_R & 0xFF;   // Store the retreived temperature data
        write_temp(temp);   // Write the updated temperature to the display
        delay();    // Delay
        delay();
        delay();
        delay();

    }
	return 0;   // Should never get here
}
