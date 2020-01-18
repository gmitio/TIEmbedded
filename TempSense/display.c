/**
 * display.c   Contains display functionality for the TM4C123G to communicate with an
 * ST7735 display based on the ARM ST7735 library by Ravi Shrestha based off of the
 * Adafruit ST7735 arduino library
 */

#include "TempSense.h"

int lastTemp;   // Holds value of most recently read temperature

unsigned char num2char(unsigned int num)    // Converts unsigned number to ASCII char for display
{
    if(num <=0x09)
        return (num+0x30);

    else
        return (0x30+9);
}

void init_display(void)
{
    /** Initialize **/
    ST7735_InitR(INITR_BLACKTAB);
    ST7735_FillScreen(ST7735_BLACK);    // Background color
    ST7735_SetRotation(3);  // Horizontal
}


void write_display(char *writeThis)
{
    unsigned int j,k,l;
    k = 0;
    l = 0;
    for(j = 0; j < strlen(writeThis); j++)
    {
        ST7735_DrawChar(10+10*l, 10+10*k, writeThis[j], ST7735_WHITE, ST7735_BLACK, 1);
        l++;
        if(l > 12)  // Overflow condition; provides wrap logic
        {
            l = 0;
            k++;
        }

    }
}

void write_temp(int Temperature)    // Function to draw the current temperature to the display
{
    if(lastTemp != Temperature) // Only rewrite the display if temperature has changed
    {
        ST7735_DrawChar(30, 50, ' ', ST7735_BLACK, ST7735_BLACK, 4);    // Clear the display
        ST7735_DrawChar(50, 50, ' ', ST7735_BLACK, ST7735_BLACK, 4);
        ST7735_DrawChar(75, 50, ' ', ST7735_BLACK, ST7735_BLACK, 4);

        if(Temperature < -9)  { // Temperature < -9
            ST7735_DrawChar(35, 50, '-', ST7735_WHITE, ST7735_BLACK, 4);
            ST7735_DrawChar(50, 50, num2char(-1*Temperature/10 % 10), ST7735_WHITE, ST7735_BLACK, 4);
            ST7735_DrawChar(75, 50, num2char(-1*Temperature % 10), ST7735_WHITE, ST7735_BLACK, 4);
        }

        else if( (Temperature < 0) && (Temperature >= -9) )  {  // -9 <= Temperature < 0
            ST7735_DrawChar(40, 50, '-', ST7735_WHITE, ST7735_BLACK, 4);
            ST7735_DrawChar(75, 50, num2char(-1*Temperature % 10), ST7735_WHITE, ST7735_BLACK, 4);
        }

        else if( (Temperature <= 9) && (Temperature >= 0) )     // 0 <= Temperature <= 9
            ST7735_DrawChar(75, 50, num2char(Temperature % 10), ST7735_WHITE, ST7735_BLACK, 4);

        else if( (Temperature <= 99) && (Temperature >  9) )  {     // 9 < Temperature <= 99
            ST7735_DrawChar(50, 50, num2char(Temperature/10 % 10), ST7735_WHITE, ST7735_BLACK, 4);
            ST7735_DrawChar(75, 50, num2char(Temperature % 10), ST7735_WHITE, ST7735_BLACK, 4);
        }

        else  {     // Temperature > 99
            ST7735_DrawChar(30, 50, num2char(Temperature/100 % 10), ST7735_WHITE, ST7735_BLACK, 4);
            ST7735_DrawChar(50, 50, num2char(Temperature/10 % 10), ST7735_WHITE, ST7735_BLACK, 4);
            ST7735_DrawChar(75, 50, num2char(Temperature % 10), ST7735_WHITE, ST7735_BLACK, 4);
        }
    }

    ST7735_DrawChar(100, 50, 247, ST7735_WHITE, ST7735_BLACK, 2);   // Draw degrees sign
    ST7735_DrawChar(115, 50, 'C', ST7735_WHITE, ST7735_BLACK, 4);   // Draw C for celsius temperature

    lastTemp = Temperature;
}
