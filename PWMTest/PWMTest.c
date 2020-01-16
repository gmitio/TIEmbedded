#include "PWMTest.h"

/**
 * Main file to test Pulse Width Modulation on the TM4C123
 */
unsigned long duty = 500;
unsigned long period  = 1000;
void init_pwm(void)
{
    SYSCTL_RCGC0_R = 0x00100000;    // Enable PWM Clock
    SYSCTL_RCGC2_R = 0x02;  // Enable GPIO Clock

    GPIO_PORTB_AFSEL_R |= 0x40; // Enable AFSEL on PB6
    GPIO_PORTB_DEN_R |= 0x40;   // Enable digital functionality on PB6
    GPIO_PORTB_ODR_R &= ~0x40;  // Disable open drain configuration on PB6
    GPIO_PORTB_AMSEL_R &= ~0x40;    // Disable analog functionality
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xF0FFFFFF) + 0x04000000;  // Set port control to use PWM0

    SYSCTL_RCC_R |= 0x00100000; // Use PWM divider
    SYSCTL_RCC_R &= ~0x000E0000;

    PWM0_0_CTL_R = 0x00;
    PWM0_0_GENA_R = 0x8C;
    PWM0_0_LOAD_R = period - 1;
    PWM0_0_CMPA_R = duty - 1;
    PWM0_0_CTL_R |= 0x01;
    PWM0_ENABLE_R |= 0x03;
}

int main(void)
{
	init_pwm();
	while(1);
}
