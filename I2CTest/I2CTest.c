

/**
 * I2CTest.c    Main file for I2CTest, a program designed to run on the TM4C123G to communicate with an
 * Microchip TCN75A I2C Temperature Sensor on I2C0 (SCL PB2, SDA PB3)
 */

#include "tm4c123gh6pm.h"

void i2c_init()
{
    SYSCTL_RCGCI2C_R |= 0x01;   // Enable I2C clock in RCGCI2C
    SYSCTL_RCGCGPIO_R |= 0x02;  // Enable GPIO clocking on Port B for I2C0
    GPIO_PORTB_AFSEL_R |= 0x0C; // Set port AFSEL to I2C
    // Enable I2CSDA with open-drain operation (datasheet pg. 676)
    // Initialize I2C master (I2CMCR)
    I2C0_MTPR_R = 7;    // Set desired clock speed (I2CMTPR), 100kbps SCL on 16MHz system clock
                              //TPR = (System Clock / (2*SCL_LP + SCL_HP) * SCL_CLK) - 1
    I2C0_MSA_R = 0x90;  // Specify slave address (1001000) and next operation (I2CMSA)
}

int main(void)
{
	return 0;
}
