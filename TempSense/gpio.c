/*
 * gpio.c
 *
 *  Created on: Feb 27, 2020
 *      Author: graison
 */

#include "TempSense.h"

void init_gpio()    // Configure Port E for outputs
{

    GPIO_PORTE_DIR_R |= 0x08;   // Set output pin on PE3
    GPIO_PORTE_DEN_R |= 0x08;

}

