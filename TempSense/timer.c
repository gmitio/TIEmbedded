/*
 * timer.c
 *
 * A better way to control the sensor polling, rather than just busy-waiting through
 * the whole main loop
 *
 *  Created on: Jan 18, 2020
 *      Author: Graison Mitio
 */

#include "TempSense.h"

#define TIMER0_LOAD_VALUE   80000000UL

void Timer0_init(void)  // Initialize Timer0 with interrupts to poll the temperature sensor at specified intervals
{
      SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
      TIMER0_CTL_R = 0x00;    // 1) disable TIMER0A during setup
      TIMER0_CFG_R = 0x00;    // 2) configure for 32-bit mode (Pg. 680-681 datasheet)
      TIMER0_TAMR_R = 0x02;   // 3) configure for periodic mode, default down-count settings
      TIMER0_TAILR_R = TIMER0_LOAD_VALUE;   // 4) reload value 100ms
      TIMER0_TAPR_R = 0;            // 5) bus clock resolution
      TIMER0_ICR_R = 0x01;    // 6) clear TIMER0A timeout flag
      TIMER0_IMR_R = 0x01;    // 7) arm timeout interrupt
      NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // 8) priority 4 interrupts enabled in the main program after all devices initialized vector number 35, interrupt number 19
      NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
      TIMER0_CTL_R = 0x01;    // 10) enable TIMER0A
}





