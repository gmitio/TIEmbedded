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
 
 
 References:
 

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

#ifndef _ST7735H_
#define _ST7735H_
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

// some flags for ST7735_InitR()
enum initRFlags{
  none,
  INITR_GREENTAB,
  INITR_REDTAB,
  INITR_BLACKTAB
};

#define ST7735_TFTWIDTH  128
#define ST7735_TFTHEIGHT 160

#define ST7735_NOP     0x00 
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20  // Display Inversion OFF , pg 97
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
/*  Colors are 565 RGB (5 bits Red, 6 bits green, 5 bits blue) */
#define  ST7735_BLACK   0x0000
#define  ST7735_BLUE    0x001F
#define  ST7735_RED     0xF800
#define  ST7735_GREEN   0x07E0
#define  ST7735_CYAN    0x07FF
#define  ST7735_MAGENTA 0xF81F
#define  ST7735_YELLOW  0xFFE0
#define  ST7735_WHITE   0xFFFF

// Initialization for ST7735B screens
void ST7735_InitB(void);

// Initialization for ST7735R screens (green or red tabs)
// ST7735_InitR(INITR_GREENTAB);
void ST7735_InitR(enum initRFlags option);

//void setAddrWindow(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1); // function is now private

//void pushColor(unsigned short color); // function is now private

void ST7735_DrawPixel(short x, short y, unsigned short color);

void ST7735_DrawFastVLine(short x, short y, short h, unsigned short color);

void ST7735_DrawFastHLine(short x, short y, short w, unsigned short color);

void ST7735_FillScreen(unsigned short color);

void ST7735_FillRect(short x, short y, short w, short h, unsigned short color);

unsigned short ST7735_Color565(unsigned char r, unsigned char g, unsigned char b);

unsigned short ST7735_SwapColor(unsigned short x);

void ST7735_DrawBitmap(short x, short y, const unsigned short *image, short w, short h);

void ST7735_DrawCharS(short x, short y, unsigned char c, short textColor, short bgColor, unsigned char size);

void ST7735_DrawChar(short x, short y, unsigned char c, short textColor, short bgColor, unsigned char size);

void ST7735_DrawCircle(short center_x, short center_y, short radius_r, unsigned short color);
void ST7735_DrawLine(short x1, short y1, short x2, short y2, unsigned short color);

void ST7735_SetRotation(unsigned char m);

void ST7735_InvertDisplay(int i);

#endif
