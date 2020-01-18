/** General SysTick Wait functions for a 16MHz CPU Cycle
*   based on code from Prof. Valvano, for ST7735 display function
*   Website: http://users.ece.utexas.edu/~valvano/arm   **/


#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018))

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void)
{
    NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
    NVIC_ST_RELOAD_R = 16000000-1;        // Set reload value to 1 second for 16MHz clock
    NVIC_ST_CURRENT_R = 0;                // Clear SysTick
	NVIC_ST_CTRL_R |= 0x07;               // Enable SysTick with default system clock; interrupts enabled
	
}

void SysTick_Start(void)
{
	NVIC_ST_CTRL_R |= 0x01; // starts SysTick timer
}

void SysTick_Stop(void)
{
	NVIC_ST_CTRL_R &= ~0x01;   // Stop SysTick timer
}
	

// Time delay using busy wait.
// The delay parameter is in units of the core clock. (units of 62.5 nsec for 16 MHz clock)
void SysTick_Wait(unsigned long delay)
{
  volatile unsigned long elapsedTime;
  unsigned long startTime = NVIC_ST_CURRENT_R;
  do  {
    elapsedTime = (startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);
}
// Time delay using busy wait.
// This assumes 16 MHz system clock.
void SysTick_Wait10ms(unsigned long delay)
{
  unsigned long i;
  for(i=0; i<delay; i++)  {
    SysTick_Wait(160000);  // wait 10ms 
  }
}
// Time delay using busy wait.
// This assumes 16 MHz system clock.
void SysTick_Wait1ms(unsigned long delay)
{
  unsigned long i;
  for(i=0; i<delay; i++)  {
    SysTick_Wait(16000);   // wait 1ms 
  }
}
