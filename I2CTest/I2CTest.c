

/**
 * I2CTest.c    Main file for I2CTest, a program designed to run on the TM4C123G to communicate with an
 * Microchip TCN75A I2C Temperature Sensor on I2C0 (SCL PB2, SDA PB3)
 */

#include "tm4c123gh6pm.h"

void i2c_init(void) // Function to initialize transmission to the sensor
{
    SYSCTL_RCGCI2C_R |= 0x01;   // Enable I2C clock in RCGCI2C
    SYSCTL_RCGCGPIO_R |= 0x02;  // Enable GPIO clocking on Port B for I2C0
    GPIO_PORTB_AFSEL_R |= 0x0C; // Set port AFSEL to I2C
    GPIO_PORTB_ODR_R |= 0x08;   // Enable open-drain operation on SDA (datasheet pg. 676)
    I2C0_MCR_R |= 0x10; // Initialize I2C as master (I2CMCR)
    I2C0_MTPR_R = 7;    // Set desired clock speed (I2CMTPR), 100kbps SCL on 16MHz system clock
                              //TPR = (System Clock / (2*SCL_LP + SCL_HP) * SCL_CLK) - 1
    I2C0_MSA_R = 0x90;  // Specify slave address (1001000) and next operation (I2CMSA)
}

int i2c_transmit(unsigned char data)   // For now let's just try transmitting one byte of data
{
    /** Transmit one byte of data across to the sensor..we'll see if this is successful using Waveforms ADM2    **/

    I2C0_MDR_R = data & 0xFF;   // Write data to be transmitted to I2CMDR
    I2C0_MCS_R = 0x07;  // Initiate transmission via I2CMCS

    while(I2C0_MCS_R & 0x40);   // Busy wait for transmission to complete by polling I2CMCS BUSBSY bit

    if(I2C0_MCS_R & 0x02)   // Check the ERROR bits in I2CMCS to ensure transmission completed successfully
        return 1;   // Error status
    else
        return 0;

}

int main(void)
{
    i2c_init();
    i2c_transmit(0);

    while(1);   // Main program loop

	return 0;   // Should never get here
}
