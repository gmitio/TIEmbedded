/***************************************************
  This is a library for the Adafruit 1.8" SPI display.
  This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
  as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

/*********************************************************
ST7735 Driver source modified for CME392 LAB experiments
 Author: Ravi Shrestha
 Organization: USASK, ECE
 Revision: 1.0
 Refer website for detail lab experiments
 
 Change Log: 1.0
 * Added Comments to make the code understandable
 * Added Register definitions required for SSI communication support for LM4F120 Cortex M4 Processor
 
 Change Log 1.1
 * Added Function for drawing Circle
 
 References:
 
 Refer pg 936 of LM4F120H5QR datasheet for SSI usage guidelines
 *********************************************************/
 
 /*****************************************************
 TFT LCD connection with LM4F120 launchpad Pinouts 

  TFT LCD (Pin)         LAUNCHPAD    (Pin)
---------------------------------------------------	
 LITE 		(pin 10) 			+3.3 V					J1-1
 MISO 		(pin 9) 			NOT USED				
 SCK 			(pin 8) 				PA2 (SSI0Clk)	J2-10
 MOSI 		(pin 7) 				PA5 (SSI0Tx)	J1-8
 TFT_CS 	(pin 6) 				PA3 (SSI0Fss)	J2-9
 CARD_CS 	(pin 5) 			NOT USED			
 D/C 			(pin 4) 				PA6 (GPIO)		J1-9
 RESET (pin 3) 						PA7 (GPIO)		J1-10
 VCC (pin 2) 							+3.3 V				J1-1
 GND (pin 1) 							GND						J3-2

*******************************************************/

#include "ST7735.h"
#include "SysTick.h"
#include <math.h>


/******************* REGISTERS MAPPINGS***************************
Please refer the LM4F120 datasheet reference page numbers indicated in the each line for detail information
********************************************************************/

#define TFT_CS                  (*((volatile unsigned long *)0x40004020)) // refer pg 644. Base address of PORTA (APB) is 0x40004000, Data Register offset is 0x00, 
#define TFT_CS_LOW              0           // CS normally controlled by hardware
#define TFT_CS_HIGH             0x08
#define DC                      (*((volatile unsigned long *)0x40004100))
#define DC_COMMAND              0
#define DC_DATA                 0x40
#define RESET                   (*((volatile unsigned long *)0x40004200))
#define RESET_LOW               0
#define RESET_HIGH              0x80
/*** Definition of BASE registers for GPIO ***/
#define GPIO_PORTA_DIR_R        (*((volatile unsigned long *)0x40004400)) // refer pg 645. Base address of PORTA (APB) is 0x40004000 Direction offset is 0x400, Necessary for assignment fo digital I/O
#define GPIO_PORTA_AFSEL_R      (*((volatile unsigned long *)0x40004420)) // refer pg 654. Base address of PORTA (APB) is 0x40004000 Alternate Function offset is 0x400, Necessary for choose between GPIO or other function
#define GPIO_PORTA_DEN_R        (*((volatile unsigned long *)0x4000451C)) // refer pg 665. Base address of PORTA (APB) is 0x40004000 Digital Enable offset is 0x51C, Necessary for assignment fo digital I/O
#define GPIO_PORTA_AMSEL_R      (*((volatile unsigned long *)0x40004528)) // refer pg 669. Base address of PORTA (APB) is 0x40004000 Analog Mode Select offset is 0x528, Necessary for assignment fo digital I/O
#define GPIO_PORTA_PCTL_R       (*((volatile unsigned long *)0x4000452C)) // refer pg 670. Base address of PORTA (APB) is 0x40004000 Port Control offset is 0x52C, Necessary for assignment fo digital I/O
/*** Definition of BASE registers for SSI0 ***/
#define SSI0_CR0_R              (*((volatile unsigned long *)0x40008000)) // refer pg 937. Base address of SSI0 is 0x40008000 Control0 register offset is 0x000, Necessary for using SSI0 
#define SSI0_CR1_R              (*((volatile unsigned long *)0x40008004)) // refer pg 937. Base address of SSI0 is 0x40008000 Control1 register offset is 0x004, Necessary for using SSI0 
#define SSI0_DR_R               (*((volatile unsigned long *)0x40008008)) // refer pg 937. Base address of SSI0 is 0x40008000 SSI data register offset is 0x008, Necessary for using SSI0 
#define SSI0_SR_R               (*((volatile unsigned long *)0x4000800C)) // refer pg 937. Base address of SSI0 is 0x40008000 SSI status register offset is 0x00C, Necessary for using SSI0 
#define SSI0_CPSR_R             (*((volatile unsigned long *)0x40008010)) // refer pg 937. Base address of SSI0 is 0x40008000 SSI clock prescale register offset is 0x010, Necessary for using SSI0 
#define SSI0_CC_R               (*((volatile unsigned long *)0x40008FC8)) // refer pg 938/ 954. Base address of SSI0 is 0x40008000 Clock configuration offset is 0xFC8, Necessary for using SSI0 

/*** Register Masking for SSI register setups ***/
#define SSI_CR0_SCR_M           0x0000FF00  // refer pg 939. SSI Serial Clock Rate 
#define SSI_CR0_SPH             0x00000080  // refer pg 939. SSI Serial Clock Phase
#define SSI_CR0_SPO             0x00000040  // refer pg 939. SSI Serial Clock Polarity
#define SSI_CR0_FRF_M           0x00000030  // refer pg 939. SSI Frame Format Select

/*** Register Data masking for SSI setup  ****
* These Register maskings are specially coded for support of SPI to use with ST7735 TFT LCD
***********************************************/

#define SSI_CR0_FRF_MOTO        0x00000000  // refer pg 940.Freescale SPI Frame Format , FRF bits value is 0x0
#define SSI_CR0_DSS_M           0x0000000F  // refer pg 939. SSI Data Size Select Masking of last 4 bits
#define SSI_CR0_DSS_8           0x00000007  // refer pg 940. SSI Data Size Select ,8-bit data , 0x07
#define SSI_CR1_MS              0x00000004  // refer pg 941. SSI Master/Slave Select Masking of 4th bit
#define SSI_CR1_SSE             0x00000002  // refer pg 941. SSI Synchronous Serial Port Masking of 2nd bit
                                            
#define SSI_SR_BSY              0x00000010  // refer pg 944. SSI Busy Bit, 5th bit in SSI status register
#define SSI_SR_TNF              0x00000002  // refer pg 944. SSI Transmit FIFO Not Full, 2nd bit is SSI status register
#define SSI_CPSR_CPSDVSR_M      0x000000FF  // refer pg 946. SSI Clock Prescale Divisor, last 8 bits in SSI Clock Prescale register
#define SSI_CC_CS_M             0x0000000F  // refer pg 954. SSI Baud Clock Source, last 4 bits of clock configuration register
#define SSI_CC_CS_SYSPLL        0x00000000  // refer pg 954. SSI clock source is System clock ( based on clock source and divisor factor)

/*** Definition for BASE register for Module clocking  ****
* These Register maskings are specially coded for support of SPI and GPIOA
***********************************************/
#define SYSCTL_RCGC1_R          (*((volatile unsigned long *)0x400FE104)) // refer pg 450. This register will used to enable clock for SSI, I2C, TIMER, UART
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108)) // refer pg 453. This register will used to enable clock for GPIO
#define SYSCTL_RCGC1_SSI0       0x00000010  // refer pg 450. SSI0 Clock Gating Control, Set 4th Bit of RCGC1 to enable clock for SSI0
#define SYSCTL_RCGC2_GPIOA      0x00000001  // refer pg 453. port A Clock Gating Control, Set 1st Bit of RCGC2 to enable clock for GPIOA


// standard ascii 5x7 font
// originally from glcdfont.c from Adafruit project
static const unsigned char Font[] = {
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
  0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
  0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
  0x18, 0x3C, 0x7E, 0x3C, 0x18,
  0x1C, 0x57, 0x7D, 0x57, 0x1C,
  0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
  0x00, 0x18, 0x3C, 0x18, 0x00,
  0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
  0x00, 0x18, 0x24, 0x18, 0x00,
  0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
  0x30, 0x48, 0x3A, 0x06, 0x0E,
  0x26, 0x29, 0x79, 0x29, 0x26,
  0x40, 0x7F, 0x05, 0x05, 0x07,
  0x40, 0x7F, 0x05, 0x25, 0x3F,
  0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
  0x7F, 0x3E, 0x1C, 0x1C, 0x08,
  0x08, 0x1C, 0x1C, 0x3E, 0x7F,
  0x14, 0x22, 0x7F, 0x22, 0x14,
  0x5F, 0x5F, 0x00, 0x5F, 0x5F,
  0x06, 0x09, 0x7F, 0x01, 0x7F,
  0x00, 0x66, 0x89, 0x95, 0x6A,
  0x60, 0x60, 0x60, 0x60, 0x60,
  0x94, 0xA2, 0xFF, 0xA2, 0x94,
  0x08, 0x04, 0x7E, 0x04, 0x08,
  0x10, 0x20, 0x7E, 0x20, 0x10,
  0x08, 0x08, 0x2A, 0x1C, 0x08,
  0x08, 0x1C, 0x2A, 0x08, 0x08,
  0x1E, 0x10, 0x10, 0x10, 0x10,
  0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
  0x30, 0x38, 0x3E, 0x38, 0x30,
  0x06, 0x0E, 0x3E, 0x0E, 0x06,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x5F, 0x00, 0x00,
  0x00, 0x07, 0x00, 0x07, 0x00,
  0x14, 0x7F, 0x14, 0x7F, 0x14,
  0x24, 0x2A, 0x7F, 0x2A, 0x12,
  0x23, 0x13, 0x08, 0x64, 0x62,
  0x36, 0x49, 0x56, 0x20, 0x50,
  0x00, 0x08, 0x07, 0x03, 0x00,
  0x00, 0x1C, 0x22, 0x41, 0x00,
  0x00, 0x41, 0x22, 0x1C, 0x00,
  0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
  0x08, 0x08, 0x3E, 0x08, 0x08,
  0x00, 0x80, 0x70, 0x30, 0x00,
  0x08, 0x08, 0x08, 0x08, 0x08,
  0x00, 0x00, 0x60, 0x60, 0x00,
  0x20, 0x10, 0x08, 0x04, 0x02,
  0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
  0x00, 0x42, 0x7F, 0x40, 0x00, // 1
  0x72, 0x49, 0x49, 0x49, 0x46, // 2
  0x21, 0x41, 0x49, 0x4D, 0x33, // 3
  0x18, 0x14, 0x12, 0x7F, 0x10, // 4
  0x27, 0x45, 0x45, 0x45, 0x39, // 5
  0x3C, 0x4A, 0x49, 0x49, 0x31, // 6
  0x41, 0x21, 0x11, 0x09, 0x07, // 7
  0x36, 0x49, 0x49, 0x49, 0x36, // 8
  0x46, 0x49, 0x49, 0x29, 0x1E, // 9
  0x00, 0x00, 0x14, 0x00, 0x00,
  0x00, 0x40, 0x34, 0x00, 0x00,
  0x00, 0x08, 0x14, 0x22, 0x41,
  0x14, 0x14, 0x14, 0x14, 0x14,
  0x00, 0x41, 0x22, 0x14, 0x08,
  0x02, 0x01, 0x59, 0x09, 0x06,
  0x3E, 0x41, 0x5D, 0x59, 0x4E,
  0x7C, 0x12, 0x11, 0x12, 0x7C, // A
  0x7F, 0x49, 0x49, 0x49, 0x36, // B
  0x3E, 0x41, 0x41, 0x41, 0x22, // C
  0x7F, 0x41, 0x41, 0x41, 0x3E, // D
  0x7F, 0x49, 0x49, 0x49, 0x41, // E
  0x7F, 0x09, 0x09, 0x09, 0x01, // F
  0x3E, 0x41, 0x41, 0x51, 0x73, // G
  0x7F, 0x08, 0x08, 0x08, 0x7F, // H
  0x00, 0x41, 0x7F, 0x41, 0x00, // I
  0x20, 0x40, 0x41, 0x3F, 0x01, // J
  0x7F, 0x08, 0x14, 0x22, 0x41, // K
  0x7F, 0x40, 0x40, 0x40, 0x40, // L
  0x7F, 0x02, 0x1C, 0x02, 0x7F, // M
  0x7F, 0x04, 0x08, 0x10, 0x7F, // N
  0x3E, 0x41, 0x41, 0x41, 0x3E, // O
  0x7F, 0x09, 0x09, 0x09, 0x06, // P
  0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
  0x7F, 0x09, 0x19, 0x29, 0x46, // R
  0x26, 0x49, 0x49, 0x49, 0x32, // S
  0x03, 0x01, 0x7F, 0x01, 0x03, // T
  0x3F, 0x40, 0x40, 0x40, 0x3F, // U
  0x1F, 0x20, 0x40, 0x20, 0x1F, // V
  0x3F, 0x40, 0x38, 0x40, 0x3F, // W
  0x63, 0x14, 0x08, 0x14, 0x63, // X
  0x03, 0x04, 0x78, 0x04, 0x03, // Y
  0x61, 0x59, 0x49, 0x4D, 0x43, // Z
  0x00, 0x7F, 0x41, 0x41, 0x41,
  0x02, 0x04, 0x08, 0x10, 0x20,
  0x00, 0x41, 0x41, 0x41, 0x7F,
  0x04, 0x02, 0x01, 0x02, 0x04,
  0x40, 0x40, 0x40, 0x40, 0x40,
  0x00, 0x03, 0x07, 0x08, 0x00,
  0x20, 0x54, 0x54, 0x78, 0x40, // a
  0x7F, 0x28, 0x44, 0x44, 0x38, // b
  0x38, 0x44, 0x44, 0x44, 0x28, // c
  0x38, 0x44, 0x44, 0x28, 0x7F, // d
  0x38, 0x54, 0x54, 0x54, 0x18, // e
  0x00, 0x08, 0x7E, 0x09, 0x02, // f
  0x18, 0xA4, 0xA4, 0x9C, 0x78, // g
  0x7F, 0x08, 0x04, 0x04, 0x78, // h
  0x00, 0x44, 0x7D, 0x40, 0x00, // i
  0x20, 0x40, 0x40, 0x3D, 0x00, // j
  0x7F, 0x10, 0x28, 0x44, 0x00, // k
  0x00, 0x41, 0x7F, 0x40, 0x00, // l
  0x7C, 0x04, 0x78, 0x04, 0x78, // m
  0x7C, 0x08, 0x04, 0x04, 0x78, // n
  0x38, 0x44, 0x44, 0x44, 0x38, // o
  0xFC, 0x18, 0x24, 0x24, 0x18, // p
  0x18, 0x24, 0x24, 0x18, 0xFC, // q
  0x7C, 0x08, 0x04, 0x04, 0x08, // r
  0x48, 0x54, 0x54, 0x54, 0x24, // s
  0x04, 0x04, 0x3F, 0x44, 0x24, // t
  0x3C, 0x40, 0x40, 0x20, 0x7C, // u
  0x1C, 0x20, 0x40, 0x20, 0x1C, // v
  0x3C, 0x40, 0x30, 0x40, 0x3C, // w
  0x44, 0x28, 0x10, 0x28, 0x44, // x
  0x4C, 0x90, 0x90, 0x90, 0x7C, // y
  0x44, 0x64, 0x54, 0x4C, 0x44, // z
  0x00, 0x08, 0x36, 0x41, 0x00,
  0x00, 0x00, 0x77, 0x00, 0x00,
  0x00, 0x41, 0x36, 0x08, 0x00,
  0x02, 0x01, 0x02, 0x04, 0x02,
  0x3C, 0x26, 0x23, 0x26, 0x3C,
  0x1E, 0xA1, 0xA1, 0x61, 0x12,
  0x3A, 0x40, 0x40, 0x20, 0x7A,
  0x38, 0x54, 0x54, 0x55, 0x59,
  0x21, 0x55, 0x55, 0x79, 0x41,
  0x21, 0x54, 0x54, 0x78, 0x41,
  0x21, 0x55, 0x54, 0x78, 0x40,
  0x20, 0x54, 0x55, 0x79, 0x40,
  0x0C, 0x1E, 0x52, 0x72, 0x12,
  0x39, 0x55, 0x55, 0x55, 0x59,
  0x39, 0x54, 0x54, 0x54, 0x59,
  0x39, 0x55, 0x54, 0x54, 0x58,
  0x00, 0x00, 0x45, 0x7C, 0x41,
  0x00, 0x02, 0x45, 0x7D, 0x42,
  0x00, 0x01, 0x45, 0x7C, 0x40,
  0xF0, 0x29, 0x24, 0x29, 0xF0,
  0xF0, 0x28, 0x25, 0x28, 0xF0,
  0x7C, 0x54, 0x55, 0x45, 0x00,
  0x20, 0x54, 0x54, 0x7C, 0x54,
  0x7C, 0x0A, 0x09, 0x7F, 0x49,
  0x32, 0x49, 0x49, 0x49, 0x32,
  0x32, 0x48, 0x48, 0x48, 0x32,
  0x32, 0x4A, 0x48, 0x48, 0x30,
  0x3A, 0x41, 0x41, 0x21, 0x7A,
  0x3A, 0x42, 0x40, 0x20, 0x78,
  0x00, 0x9D, 0xA0, 0xA0, 0x7D,
  0x39, 0x44, 0x44, 0x44, 0x39,
  0x3D, 0x40, 0x40, 0x40, 0x3D,
  0x3C, 0x24, 0xFF, 0x24, 0x24,
  0x48, 0x7E, 0x49, 0x43, 0x66,
  0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
  0xFF, 0x09, 0x29, 0xF6, 0x20,
  0xC0, 0x88, 0x7E, 0x09, 0x03,
  0x20, 0x54, 0x54, 0x79, 0x41,
  0x00, 0x00, 0x44, 0x7D, 0x41,
  0x30, 0x48, 0x48, 0x4A, 0x32,
  0x38, 0x40, 0x40, 0x22, 0x7A,
  0x00, 0x7A, 0x0A, 0x0A, 0x72,
  0x7D, 0x0D, 0x19, 0x31, 0x7D,
  0x26, 0x29, 0x29, 0x2F, 0x28,
  0x26, 0x29, 0x29, 0x29, 0x26,
  0x30, 0x48, 0x4D, 0x40, 0x20,
  0x38, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x38,
  0x2F, 0x10, 0xC8, 0xAC, 0xBA,
  0x2F, 0x10, 0x28, 0x34, 0xFA,
  0x00, 0x00, 0x7B, 0x00, 0x00,
  0x08, 0x14, 0x2A, 0x14, 0x22,
  0x22, 0x14, 0x2A, 0x14, 0x08,
  0xAA, 0x00, 0x55, 0x00, 0xAA,
  0xAA, 0x55, 0xAA, 0x55, 0xAA,
  0x00, 0x00, 0x00, 0xFF, 0x00,
  0x10, 0x10, 0x10, 0xFF, 0x00,
  0x14, 0x14, 0x14, 0xFF, 0x00,
  0x10, 0x10, 0xFF, 0x00, 0xFF,
  0x10, 0x10, 0xF0, 0x10, 0xF0,
  0x14, 0x14, 0x14, 0xFC, 0x00,
  0x14, 0x14, 0xF7, 0x00, 0xFF,
  0x00, 0x00, 0xFF, 0x00, 0xFF,
  0x14, 0x14, 0xF4, 0x04, 0xFC,
  0x14, 0x14, 0x17, 0x10, 0x1F,
  0x10, 0x10, 0x1F, 0x10, 0x1F,
  0x14, 0x14, 0x14, 0x1F, 0x00,
  0x10, 0x10, 0x10, 0xF0, 0x00,
  0x00, 0x00, 0x00, 0x1F, 0x10,
  0x10, 0x10, 0x10, 0x1F, 0x10,
  0x10, 0x10, 0x10, 0xF0, 0x10,
  0x00, 0x00, 0x00, 0xFF, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0xFF, 0x10,
  0x00, 0x00, 0x00, 0xFF, 0x14,
  0x00, 0x00, 0xFF, 0x00, 0xFF,
  0x00, 0x00, 0x1F, 0x10, 0x17,
  0x00, 0x00, 0xFC, 0x04, 0xF4,
  0x14, 0x14, 0x17, 0x10, 0x17,
  0x14, 0x14, 0xF4, 0x04, 0xF4,
  0x00, 0x00, 0xFF, 0x00, 0xF7,
  0x14, 0x14, 0x14, 0x14, 0x14,
  0x14, 0x14, 0xF7, 0x00, 0xF7,
  0x14, 0x14, 0x14, 0x17, 0x14,
  0x10, 0x10, 0x1F, 0x10, 0x1F,
  0x14, 0x14, 0x14, 0xF4, 0x14,
  0x10, 0x10, 0xF0, 0x10, 0xF0,
  0x00, 0x00, 0x1F, 0x10, 0x1F,
  0x00, 0x00, 0x00, 0x1F, 0x14,
  0x00, 0x00, 0x00, 0xFC, 0x14,
  0x00, 0x00, 0xF0, 0x10, 0xF0,
  0x10, 0x10, 0xFF, 0x10, 0xFF,
  0x14, 0x14, 0x14, 0xFF, 0x14,
  0x10, 0x10, 0x10, 0x1F, 0x00,
  0x00, 0x00, 0x00, 0xF0, 0x10,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
  0xFF, 0xFF, 0xFF, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0xFF,
  0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
  0x38, 0x44, 0x44, 0x38, 0x44,
  0x7C, 0x2A, 0x2A, 0x3E, 0x14,
  0x7E, 0x02, 0x02, 0x06, 0x06,
  0x02, 0x7E, 0x02, 0x7E, 0x02,
  0x63, 0x55, 0x49, 0x41, 0x63,
  0x38, 0x44, 0x44, 0x3C, 0x04,
  0x40, 0x7E, 0x20, 0x1E, 0x20,
  0x06, 0x02, 0x7E, 0x02, 0x02,
  0x99, 0xA5, 0xE7, 0xA5, 0x99,
  0x1C, 0x2A, 0x49, 0x2A, 0x1C,
  0x4C, 0x72, 0x01, 0x72, 0x4C,
  0x30, 0x4A, 0x4D, 0x4D, 0x30,
  0x30, 0x48, 0x78, 0x48, 0x30,
  0xBC, 0x62, 0x5A, 0x46, 0x3D,
  0x3E, 0x49, 0x49, 0x49, 0x00,
  0x7E, 0x01, 0x01, 0x01, 0x7E,
  0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
  0x44, 0x44, 0x5F, 0x44, 0x44,
  0x40, 0x51, 0x4A, 0x44, 0x40,
  0x40, 0x44, 0x4A, 0x51, 0x40,
  0x00, 0x00, 0xFF, 0x01, 0x03,
  0xE0, 0x80, 0xFF, 0x00, 0x00,
  0x08, 0x08, 0x6B, 0x6B, 0x08,
  0x36, 0x12, 0x36, 0x24, 0x36,
  0x06, 0x0F, 0x09, 0x0F, 0x06,
  0x00, 0x00, 0x18, 0x18, 0x00,
  0x00, 0x00, 0x10, 0x10, 0x00,
  0x30, 0x40, 0xFF, 0x01, 0x01,
  0x00, 0x1F, 0x01, 0x01, 0x1E,
  0x00, 0x19, 0x1D, 0x17, 0x12,
  0x00, 0x3C, 0x3C, 0x3C, 0x3C,
  0x00, 0x00, 0x00, 0x00, 0x00,
};


static unsigned char ColStart, RowStart; // some displays need this changed
static unsigned char Rotation;           // 0 to 3
static enum initRFlags TabColor;
static short _width = ST7735_TFTWIDTH;   // this could probably be a constant, except it is used in Adafruit_GFX and depends on image rotation
static short _height = ST7735_TFTHEIGHT;


// The Data/Command pin must be valid when the eighth bit is
// sent.  The SSI module has hardware input and output FIFOs
// that are 8 locations deep.  Based on the observation that
// the LCD interface tends to send a few commands and then a
// lot of data, the FIFOs are not used when writing
// commands, and they are used when writing data.  This
// ensures that the Data/Command pin status matches the byte
// that is actually being transmitted.
// The write command operation waits until all data has been
// sent, configures the Data/Command pin for commands, sends
// the command, and then waits for the transmission to
// finish.
// The write data operation waits until there is room in the
// transmit FIFO, configures the Data/Command pin for data,
// and then adds the data to the transmit FIFO.
// NOTE: These functions will crash or stall indefinitely if
// the SSI0 module is not initialized and enabled.
void static writecommand(unsigned char c) {
                                        // wait until SSI0 not busy/transmit FIFO empty
  while((SSI0_SR_R&SSI_SR_BSY)==SSI_SR_BSY){};
  DC = DC_COMMAND;
  SSI0_DR_R = c;                        // Write data to SSI Data Register
                                        // wait until SSI0 not busy/transmit FIFO empty
  while((SSI0_SR_R&SSI_SR_BSY)==SSI_SR_BSY){};
}


void static writedata(unsigned char c) {
  while((SSI0_SR_R&SSI_SR_TNF)==0){};   // wait until transmit FIFO not full
  DC = DC_DATA;
  SSI0_DR_R = c;                        // data out
}


// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in ROM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80
static const unsigned char
  Bcmd[] = {                  // Initialization commands for 7735B screens
    18,                       // 18 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, no args, w/delay
      50,                     //     50 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, no args, w/delay
      255,                    //     255 = 500 ms delay
    ST7735_COLMOD , 1+DELAY,  //  3: Set color mode, 1 arg + delay:
      0x05,                   //     16-bit color
      10,                     //     10 ms delay
    ST7735_FRMCTR1, 3+DELAY,  //  4: Frame rate control, 3 args + delay:
      0x00,                   //     fastest refresh
      0x06,                   //     6 lines front porch
      0x03,                   //     3 lines back porch
      10,                     //     10 ms delay
    ST7735_MADCTL , 1      ,  //  5: Memory access ctrl (directions), 1 arg:
      0x08,                   //     Row addr/col addr, bottom to top refresh
    ST7735_DISSET5, 2      ,  //  6: Display settings #5, 2 args, no delay:
      0x15,                   //     1 clk cycle nonoverlap, 2 cycle gate
                              //     rise, 3 cycle osc equalize
      0x02,                   //     Fix on VTL
    ST7735_INVCTR , 1      ,  //  7: Display inversion control, 1 arg:
      0x0,                    //     Line inversion
    ST7735_PWCTR1 , 2+DELAY,  //  8: Power control, 2 args + delay:
      0x02,                   //     GVDD = 4.7V
      0x70,                   //     1.0uA
      10,                     //     10 ms delay
    ST7735_PWCTR2 , 1      ,  //  9: Power control, 1 arg, no delay:
      0x05,                   //     VGH = 14.7V, VGL = -7.35V
    ST7735_PWCTR3 , 2      ,  // 10: Power control, 2 args, no delay:
      0x01,                   //     Opamp current small
      0x02,                   //     Boost frequency
    ST7735_VMCTR1 , 2+DELAY,  // 11: Power control, 2 args + delay:
      0x3C,                   //     VCOMH = 4V
      0x38,                   //     VCOML = -1.1V
      10,                     //     10 ms delay
    ST7735_PWCTR6 , 2      ,  // 12: Power control, 2 args, no delay:
      0x11, 0x15,
    ST7735_GMCTRP1,16      ,  // 13: Magical unicorn dust, 16 args, no delay:
      0x09, 0x16, 0x09, 0x20, //     (seriously though, not sure what
      0x21, 0x1B, 0x13, 0x19, //      these config values represent)
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    ST7735_GMCTRN1,16+DELAY,  // 14: Sparkles and rainbows, 16 args + delay:
      0x0B, 0x14, 0x08, 0x1E, //     (ditto)
      0x22, 0x1D, 0x18, 0x1E,
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
      10,                     //     10 ms delay
    ST7735_CASET  , 4      ,  // 15: Column addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 2
      0x00, 0x81,             //     XEND = 129
    ST7735_RASET  , 4      ,  // 16: Row addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 1
      0x00, 0x81,             //     XEND = 160
    ST7735_NORON  ,   DELAY,  // 17: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,   DELAY,  // 18: Main screen turn on, no args, w/delay
      255 };                  //     255 = 500 ms delay
static const unsigned char
  Rcmd1[] = {                 // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list: // indicates there are 15 commands
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay // CMD : 0x01, ST7735 datasheet page No. 83 , then we need to have delay
      150,                    //     150 ms delay 
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay // CMD: 0x11,  ST7735 datasheet page No. 94, then we have to wait for at least 120ms
      255,                    //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args: // CMD: 0xB1, Pg. 126, 
      0x01, 0x2C, 0x2D,       //     Rate = fosc/((0X01)x2+40) * (LINE+(0X2C)+(0X2D)), here frame rate is: ?? , fosc= 625KHz, LINE=
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args: // // CMD: 0xB2, Pg. 127,
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args: // CMD: 0xB3, Pg. 128,
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay: // CMD: 0xB4, Pg. 129, 
      0x07,                   //     No inversion, ( i,e all are in Line inversion mode)
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay: // CMD: 0xC0, Pg. 130,
      0xA2,										// 			Binary: 101 00010, AVDD=5V, GVDD= 4.6
      0x02,                   //     GVCL=-4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay, CMD 0x20, pg 97
    ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
      0xC8,                   //     row addr/col addr, bottom to top refresh
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 };                 //     16-bit color
static const unsigned char
  Rcmd2green[] = {            // Init for 7735R, part 2 (green tab only)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay: pg 102
      0x00, 0x02,             //     XSTART = 0
      0x00, 0x7F+0x02,        //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x01,             //     XSTART = 0
      0x00, 0x9F+0x01 };      //     XEND = 159
static const unsigned char
  Rcmd2red[] = {              // Init for 7735R, part 2 (red tab only)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F };           //     XEND = 159
static const unsigned char
  Rcmd3[] = {                 // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in ROM byte array.
void static commandList(const unsigned char *addr) {

  unsigned char numCommands, numArgs;
  unsigned short ms;

  numCommands = *(addr++);               // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writecommand(*(addr++));             //   Read, issue command
    numArgs  = *(addr++);                //   Number of args to follow
    ms       = numArgs & DELAY;          //   If hibit set, delay follows args i,e 10000000 = 0x80 denotes that we have to delay after command
    numArgs &= ~DELAY;                   //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata(*(addr++));              //     Read, issue argument
    }

    if(ms) {
      ms = *(addr++);             // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      SysTick_Wait1ms(ms);				// wait using the SysTick
    }
  }
}

/**** Initialization function for SSI0  Module ********/
void init_SSI0(void)
{
  GPIO_PORTA_AFSEL_R |= 0x2C;           // enable alt funct on PA2,PA3,PA5 binary 0010 1100
  GPIO_PORTA_DEN_R |= 0x2C;             // enable digital I/O on PA2,PA3,PA5 binary 0010 1100
                                        // configure PA2,PA3,PA5 as SSI
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFF0F00FF)+0x00202200;  // refer pg 670 Port MUX control for GPIO1 ,GPIO2 and GPIO 4 ( PA2, PA3, PA4)m writing 0x02 gives its SSI functionality
  GPIO_PORTA_AMSEL_R &= ~0x2C;          // disable analog functionality on PA2,PA3,PA5
  SSI0_CR1_R &= ~SSI_CR1_SSE;           // disable SSI (SSE=0)( we need to disable SSI before any control registers are reporgrammed!! pg, 942)
  SSI0_CR1_R &= ~SSI_CR1_MS;            // Enable master mode ( MS=0)
                                        // refer pg 954. configure for system clock/PLL baud clock source
  SSI0_CC_R = (SSI0_CC_R&~SSI_CC_CS_M)+SSI_CC_CS_SYSPLL; // Mask last 4 bits and fill it with zeros simply
                                        // refer pg 946. clock divider for 3.125 MHz SSIClk we have BR=SysClk/(CPSDVSR * (1 + SCR)), here BR=3.125Mbps, SysClk =50Mhz, SCR=0 so CPSDVSR=16
  SSI0_CPSR_R = (SSI0_CPSR_R&~SSI_CPSR_CPSDVSR_M)+16;
  SSI0_CR0_R &= ~(SSI_CR0_SCR_M |       // SCR = 0 (3.125 Mbps data rate) BR=SysClk/(CPSDVSR * (1 + SCR))
                  SSI_CR0_SPH |         // SPH = 0 Data is captured on the first clock edge transistion, (rising edge)
                  SSI_CR0_SPO);         // SPO = 0 Steady state Low value is placed on the SSIClk pin (default state of clk pin is LOW)
                                        // FRF = Freescale format
  SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_FRF_M)+SSI_CR0_FRF_MOTO;
                                        // DSS = 8-bit data
  SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_DSS_M)+SSI_CR0_DSS_8;
  SSI0_CR1_R |= SSI_CR1_SSE;            // Finally enable SSI after  configuration of all parameters	
	
}
// Initialization code common to both 'B' and 'R' type displays // Remember we have R type of display
void static commonInit(const unsigned char *cmdList) {
  volatile unsigned long delay;
  ColStart  = RowStart = 0; // May be overridden in init func

  SysTick_Init();                       // initialize SysTick timer
	//SysTick_Stop();
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_SSI0;  // activate SSI0
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // activate port A
  delay = SYSCTL_RCGC2_R;               // allow time to finish activating

  // toggle RST low to reset; CS low so it'll listen to us
  // SSI0Fss is temporarily used as GPIO
  GPIO_PORTA_DIR_R |= 0xC8;             // make PA3,6,7 out
  GPIO_PORTA_AFSEL_R &= ~0xC8;          // disable alt funct on PA3,6,7
  GPIO_PORTA_DEN_R |= 0xC8;             // enable digital I/O on PA3,6,7
                                        // configure PA3,6,7 as GPIO
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0x00FF0FFF)+0x00000000;
  GPIO_PORTA_AMSEL_R &= ~0xC8;          // disable analog functionality on PA3,6,7
  TFT_CS = TFT_CS_LOW;										// Set default CS as LOW
  RESET = RESET_HIGH;											// Pull RESET pin HIGH
  SysTick_Wait1ms(500);										// wait for about 500ms
  RESET = RESET_LOW;											//	Pull RESET pin LOW			
  SysTick_Wait1ms(500);										// wait for 500ms
  RESET = RESET_HIGH;											// Pull Reset pin HIGH	
  SysTick_Wait1ms(500);										// wait for 500ms	

  init_SSI0();														// Initialize SSI0 port
	if(cmdList) commandList(cmdList);				// Send initialization Commands to TFT as cmdList argument
}


//------------ST7735_InitB------------
// Initialization for ST7735B screens.
// Input: none
// Output: none
void ST7735_InitB(void) {
  commonInit(Bcmd);
}


//------------ST7735_InitR------------
// Initialization for ST7735R screens (green or red tabs).
// Input: option one of the enumerated options depending on tabs
// Output: none
void ST7735_InitR(enum initRFlags option) {
  commonInit(Rcmd1);  // initialize the display
  if(option == INITR_GREENTAB)
		{ 
    commandList(Rcmd2green);
    ColStart = 2;
    RowStart = 1;
		} 
	else 
		{
    // colstart, rowstart left at default '0' values
    commandList(Rcmd2red);
		}
  commandList(Rcmd3);
 // if black, change MADCTL color filter
  if (option == INITR_BLACKTAB) {
    writecommand(ST7735_MADCTL);
    writedata(0xC0);
  }
  TabColor = option;
}


// Set the region of the screen RAM to be modified
// Pixel colors are sent left to right, top to bottom
// (same as Font table is encoded; different from regular bitmap)
// Requires 11 bytes of transmission
void static setAddrWindow(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1) {

  writecommand(ST7735_CASET); // Column addr set
  writedata(0x00);
  writedata(x0+ColStart);     // XSTART
  writedata(0x00);
  writedata(x1+ColStart);     // XEND

  writecommand(ST7735_RASET); // Row addr set
  writedata(0x00);
  writedata(y0+RowStart);     // YSTART
  writedata(0x00);
  writedata(y1+RowStart);     // YEND

  writecommand(ST7735_RAMWR); // write to RAM
}


// Send two bytes of data, most significant byte first
// Requires 2 bytes of transmission
void static pushColor(unsigned short color) {
  writedata((unsigned char)(color >> 8));
  writedata((unsigned char)color);
}


//------------ST7735_DrawPixel------------
// Color the pixel at the given coordinates with the given color.
// Requires 13 bytes of transmission
// Input: x     horizontal position of the pixel, columns from the left edge
//               must be less than 128
//               0 is on the left, 126 is near the right
//        y     vertical position of the pixel, rows from the top edge
//               must be less than 160
//               159 is near the wires, 0 is the left side opposite the wires
//        color 16-bit color, which can be produced by ST7735_Color565()
// Output: none
void ST7735_DrawPixel(short x, short y, unsigned short color) {

  if((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

  setAddrWindow(x,y,x+1,y+1);

  pushColor(color);
}


//------------ST7735_DrawFastVLine------------
// Draw a vertical line at the given coordinates with the given height and color.
// A vertical line is parallel to the longer side of the rectangular display
// Requires (11 + 2*h) bytes of transmission (assuming image fully on screen)
// Input: x     horizontal position of the start of the line, columns from the left edge
//        y     vertical position of the start of the line, rows from the top edge
//        h     vertical height of the line
//        color 16-bit color, which can be produced by ST7735_Color565()
// Output: none
void ST7735_DrawFastVLine(short x, short y, short h, unsigned short color) {
  unsigned char hi = color >> 8, lo = color;

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((y+h-1) >= _height) h = _height-y;
  setAddrWindow(x, y, x, y+h-1);

  hi = color >> 8;
  lo = color;
  while (h--) {
    writedata(hi);
    writedata(lo);
  }
}

void ST7735_DrawCircle(short center_x, short center_y, short radius_r, unsigned short color) {
  int x,y;
	int Len=radius_r * 0.70710678118; // compute x=r*Cos(pi/4)
//	unsigned char hi = color >> 8, lo = color;

  // Rudimentary clipping
  if((center_x >= _width) || (center_y >= _height)) return;
  
	ST7735_DrawPixel(center_x,center_y,color); // draw center point
	
  setAddrWindow(center_x, center_y, center_x+radius_r, center_y+radius_r);

	for(x=0;x<=Len;x++)
	{
		 y = (int) sqrt ((double) (radius_r * radius_r) - (x * x));
			ST7735_DrawPixel(x+center_x,y+center_y,color); 
			ST7735_DrawPixel(x+center_x,-y+center_y,color);
			ST7735_DrawPixel(-x+center_x,y+center_y,color);
			ST7735_DrawPixel(-x+center_x,-y+center_y,color);
		
			ST7735_DrawPixel(y+center_y,x+center_x,color); 
			ST7735_DrawPixel(y+center_y,-x+center_x,color);
			ST7735_DrawPixel(-y+center_y,x+center_x,color);
			ST7735_DrawPixel(-y+center_y,-x+center_x,color);
		}
}


void ST7735_DrawLine(short x1, short y1, short x2, short y2, unsigned short color) {
 // unsigned char hi = color >> 8, lo = color;
	//int x=x1;
	//int y=y1;
	
	int dy=y2-y1;
	int dx=x2-x1;
	double m=dy/dx;
	double c=y1-m*x1;
	if((x1 >= _width) || (y1 >= _height) || (x2 >= _width) || (y2 >= _height) ) return; 
	setAddrWindow(x1, y1, x1+x2-1, y2);
	while(x1<=x2)
		{
		if(m<=1)
			{
			x1=x1+1;
			y1=m*x1+c;
				ST7735_DrawPixel(x1,y1,color);
			}
		else
			{
			y1=y1+1;
			x1=(y1-c)/m;
				ST7735_DrawPixel(x1,y1,color);
			}
		}
	 
}

//------------ST7735_DrawFastHLine------------
// Draw a horizontal line at the given coordinates with the given width and color.
// A horizontal line is parallel to the shorter side of the rectangular display
// Requires (11 + 2*w) bytes of transmission (assuming image fully on screen)
// Input: x     horizontal position of the start of the line, columns from the left edge
//        y     vertical position of the start of the line, rows from the top edge
//        w     horizontal width of the line
//        color 16-bit color, which can be produced by ST7735_Color565()
// Output: none
void ST7735_DrawFastHLine(short x, short y, short w, unsigned short color) {
  unsigned char hi = color >> 8, lo = color;

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  setAddrWindow(x, y, x+w-1, y);

  while (w--) {
    writedata(hi);
    writedata(lo);
  }
}


//------------ST7735_FillScreen------------
// Fill the screen with the given color.
// Requires 40,971 bytes of transmission
// Input: color 16-bit color, which can be produced by ST7735_Color565()
// Output: none
void ST7735_FillScreen(unsigned short color) {
  ST7735_FillRect(0, 0, _width, _height, color);
}


//------------ST7735_FillRect------------
// Draw a filled rectangle at the given coordinates with the given width, height, and color.
// Requires (11 + 2*w*h) bytes of transmission (assuming image fully on screen)
// Input: x     horizontal position of the top left corner of the rectangle, columns from the left edge
//        y     vertical position of the top left corner of the rectangle, rows from the top edge
//        w     horizontal width of the rectangle
//        h     vertical height of the rectangle
//        color 16-bit color, which can be produced by ST7735_Color565()
// Output: none
void ST7735_FillRect(short x, short y, short w, short h, unsigned short color) {
  unsigned char hi = color >> 8, lo = color;

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  setAddrWindow(x, y, x+w-1, y+h-1);

  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      writedata(hi);
      writedata(lo);
    }
  }
}


//------------ST7735_Color565------------
// Pass 8-bit (each) R,G,B and get back 16-bit packed color.
// Input: r red value
//        g green value
//        b blue value
// Output: 16-bit color
unsigned short ST7735_Color565(unsigned char r, unsigned char g, unsigned char b) {
  return ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3);
}


//------------ST7735_SwapColor------------
// Swaps the red and blue values of the given 16-bit packed color;
// green is unchanged.
// Input: x 16-bit color in format B, G, R
// Output: 16-bit color in format R, G, B
unsigned short ST7735_SwapColor(unsigned short x) {
  return (x << 11) | (x & 0x07E0) | (x >> 11);
}


//------------ST7735_DrawBitmap------------
// Displays a 16-bit color BMP image.  A bitmap file that is created
// by a PC image processing program has a header and may be padded
// with dummy columns so the data have four byte alignment.  This
// function assumes that all of that has been stripped out, and the
// array image[] has one 16-bit halfword for each pixel to be
// displayed on the screen (encoded in reverse order, which is
// standard for bitmap files).  An array can be created in this
// format from a 24-bit-per-pixel .bmp file using the associated
// converter program.
// (x,y) is the screen location of the lower left corner of BMP image
// Requires (11 + 2*w*h) bytes of transmission (assuming image fully on screen)
// Input: x     horizontal position of the bottom left corner of the image, columns from the left edge
//        y     vertical position of the bottom left corner of the image, rows from the top edge
//        image pointer to a 16-bit color BMP image
//        w     number of pixels wide
//        h     number of pixels tall
// Output: none
// Must be less than or equal to 128 pixels wide by 160 pixels high
void ST7735_DrawBitmap(short x, short y, const unsigned short *image, short w, short h){
  short skipC=0;                        // non-zero if columns need to be skipped due to clipping
  short originalWidth = w;              // save this value; even if not all columns fit on the screen, the image is still this width in ROM
  int i = w*(h - 1);

  if((x >= _width) || ((y - h + 1) >= _height) || ((x + w) <= 0) || (y < 0)){
    return;                             // image is totally off the screen, do nothing
  }
  if((w > _width) || (h > _height)){    // image is too wide for the screen, do nothing
    //***This isn't necessarily a fatal error, but it makes the
    //following logic much more complicated, since you can have
    //an image that exceeds multiple boundaries and needs to be
    //clipped on more than one side.
    return;
  }
  if((x + w - 1) >= _width){            // image exceeds right of screen
    skipC = (x + w) - _width;           // skip cut off columns
    w = _width - x;
  }
  if((y - h + 1) < 0){                  // image exceeds top of screen
    i = i - (h - y - 1)*originalWidth;  // skip the last cut off rows
    h = y + 1;
  }
  if(x < 0){                            // image exceeds left of screen
    w = w + x;
    skipC = -1*x;                       // skip cut off columns
    i = i - x;                          // skip the first cut off columns
    x = 0;
  }
  if(y >= _height){                     // image exceeds bottom of screen
    h = h - (y - _height + 1);
    y = _height - 1;
  }

  setAddrWindow(x, y-h+1, x+w-1, y);

  for(y=0; y<h; y=y+1){
    for(x=0; x<w; x=x+1){
                                        // send the top 8 bits
      writedata((unsigned char)(image[i] >> 8));
                                        // send the bottom 8 bits
      writedata((unsigned char)image[i]);
      i = i + 1;                        // go to the next pixel
    }
    i = i + skipC;
    i = i - 2*originalWidth;
  }
}


//------------ST7735_DrawCharS------------
// Simple character draw function.  This is the same function from
// Adafruit_GFX.c but adapted for this processor.  However, each call
// to ST7735_DrawPixel() calls setAddrWindow(), which needs to send
// many extra data and commands.  If the background color is the same
// as the text color, no background will be printed, and text can be
// drawn right over existing images without covering them with a box.
// Requires (11 + 2*size*size)*6*8 (image fully on screen; textcolor != bgColor)
// Input: x         horizontal position of the top left corner of the character, columns from the left edge
//        y         vertical position of the top left corner of the character, rows from the top edge
//        c         character to be printed
//        textColor 16-bit color of the character
//        bgColor   16-bit color of the background
//        size      number of pixels per character pixel (e.g. size==2 prints each pixel of font as 2x2 square)
// Output: none
void ST7735_DrawCharS(short x, short y, unsigned char c, short textColor, short bgColor, unsigned char size){
  unsigned char line; // vertical column of pixels of character in font
  char i, j;
  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 5 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  for (i=0; i<6; i++ ) {
    if (i == 5)
      line = 0x0;
    else
      line = Font[(c*5)+i];
    for (j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          ST7735_DrawPixel(x+i, y+j, textColor);
        else {  // big size
          ST7735_FillRect(x+(i*size), y+(j*size), size, size, textColor);
        }
      } else if (bgColor != textColor) {
        if (size == 1) // default size
          ST7735_DrawPixel(x+i, y+j, bgColor);
        else {  // big size
          ST7735_FillRect(x+i*size, y+j*size, size, size, bgColor);
        }
      }
      line >>= 1;
    }
  }
}


//------------ST7735_DrawChar------------
// Advanced character draw function.  This is similar to the function
// from Adafruit_GFX.c but adapted for this processor.  However, this
// function only uses one call to setAddrWindow(), which allows it to
// run at least twice as fast.
// Requires (11 + size*size*6*8) bytes of transmission (assuming image fully on screen)
// Input: x         horizontal position of the top left corner of the character, columns from the left edge
//        y         vertical position of the top left corner of the character, rows from the top edge
//        c         character to be printed
//        textColor 16-bit color of the character
//        bgColor   16-bit color of the background
//        size      number of pixels per character pixel (e.g. size==2 prints each pixel of font as 2x2 square)
// Output: none
void ST7735_DrawChar(short x, short y, unsigned char c, short textColor, short bgColor, unsigned char size){
  unsigned char line; // horizontal row of pixels of character
  char col, row, i, j;// loop indices
  if(((x + 5*size - 1) >= _width)  || // Clip right
     ((y + 8*size - 1) >= _height) || // Clip bottom
     ((x + 5*size - 1) < 0)        || // Clip left
     ((y + 8*size - 1) < 0)){         // Clip top
    return;
  }

  setAddrWindow(x, y, x+6*size-1, y+8*size-1);

  line = 0x01;        // print the top row first
  // print the rows, starting at the top
  for(row=0; row<8; row=row+1){
    for(i=0; i<size; i=i+1){
      // print the columns, starting on the left
      for(col=0; col<5; col=col+1){
        if(Font[(c*5)+col]&line){
          // bit is set in Font, print pixel(s) in text color
          for(j=0; j<size; j=j+1){
            pushColor(textColor);
          }
        } else{
          // bit is cleared in Font, print pixel(s) in background color
          for(j=0; j<size; j=j+1){
            pushColor(bgColor);
          }
        }
      }
      // print blank column(s) to the right of character
      for(j=0; j<size; j=j+1){
        pushColor(bgColor);
      }
    }
    line = line<<1;   // move up to the next row
  }
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

//------------ST7735_SetRotation------------
// Change the image rotation.
// Requires 2 bytes of transmission
// Input: m new rotation value (0 to 3)
// Output: none
void ST7735_SetRotation(unsigned char m) {

  writecommand(ST7735_MADCTL);
  Rotation = m % 4; // can't be higher than 3
  switch (Rotation) {
   case 0:
     if (TabColor == INITR_BLACKTAB) {
       writedata(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
     } else {
       writedata(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
     }
     _width  = ST7735_TFTWIDTH;
     _height = ST7735_TFTHEIGHT;
     break;
   case 1:
     if (TabColor == INITR_BLACKTAB) {
       writedata(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
     } else {
       writedata(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
     }
     _width  = ST7735_TFTHEIGHT;
     _height = ST7735_TFTWIDTH;
     break;
  case 2:
     if (TabColor == INITR_BLACKTAB) {
       writedata(MADCTL_RGB);
     } else {
       writedata(MADCTL_BGR);
     }
     _width  = ST7735_TFTWIDTH;
     _height = ST7735_TFTHEIGHT;
    break;
   case 3:
     if (TabColor == INITR_BLACKTAB) {
       writedata(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
     } else {
       writedata(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
     }
     _width  = ST7735_TFTHEIGHT;
     _height = ST7735_TFTWIDTH;
     break;
  }
}


//------------ST7735_InvertDisplay------------
// Send the command to invert all of the colors.
// Requires 1 byte of transmission
// Input: i 0 to disable inversion; non-zero to enable inversion
// Output: none
void ST7735_InvertDisplay(int i) {
  if(i){
    writecommand(ST7735_INVON);
  } else{
    writecommand(ST7735_INVOFF);
  }
}
