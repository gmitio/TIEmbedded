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

/** Writes a string message to the LCD display
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

/********** End of Function Prototypes *********/


#endif /* TEMPSENSE_H_ */
