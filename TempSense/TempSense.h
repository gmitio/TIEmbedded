/*
 * TempSense.h
 *
 *  Created on: Jan 17, 2020
 *      Author: Graison Mitio
 */

#include "ST7735.h"     // Include this header file to use functions related to ST7735 display
#include "Systick.h"    // Include this header file to initialize Systick timer
#include "tm4c123gh6pm.h"
#include <string.h>

#ifndef TEMPSENSE_H_
#define TEMPSENSE_H_

/************* Function Prototypes ************/

/** Function to initialize I2C communication to
 * the sensor on I2C0 for repeated start condition
 * to send and receive sensor data
 * Defined in i2c.c **/
void i2c_init(void);

/** Function to transmit a byte to the sensor to read back data
 * Takes transmitted byte as argument; returns sensor data
 * Defined in i2c.c **/
int i2c_transmit(unsigned long data);

/** Initializes the display to a black background
 * Defined in display.c**/
void init_display(void);

/** Writes a string message to the LCD display; probably fix to make a more general int to string conversion function
 * Defined in display.c **/
void write_display(char *writeThis);

/** Writes the current temperature to the LCD display
 * Takes temperature data as signed integer argument
 * Defined in display.c **/
void write_temp(int Temperature);

/** Converts unsigned number to ASCII char for display
 * Takes unsigned number as argument; returns unsigned char
 * Defined in TempSense.c **/
unsigned char num2char(unsigned int num);

/** Cheap delay function, probably avoid using this
 * Defined in TempSense.c **/
void delay (void);

/** High level function to control the repetitive tasks associated
 * with accessing i2c, writing data to display, clearing timer flags, etc
 * Defined in TempSense.c **/
void ReadTemperature(void);

/** Initializes system timer 0 with interrupts in 32-bit mode
 * Defined in timer.c  **/
void Timer0_init(void);

/** Initializes UART1 serial communication at a specified baud rate
 * See datasheet for integer (BRDI) and fractional (BRDF) baud rate equations
 * Defined in uart.c
 */
void init_uart(unsigned long BRDI, unsigned long BRDF);

/** Outputs serial message on UART1
 * Defined in uart.c
 */
void UART_OutString(char *string_out);

/** Outputs serial character on UART1
 * Defined in uart.c
 */
void UART_OutChar(unsigned char data);

/** Converts the sensor value to a string containing temperature data
 * Defined in display.c
 */
char *itotemp(int Temperature);

/********** End of Function Prototypes *********/


#endif /* TEMPSENSE_H_ */
