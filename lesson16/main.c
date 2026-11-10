#include <stdint.h>

#include "delay.h"
#include "tm4c_cmsis.h"

#define LED_RED (1U << 1)
#define LED_BLUE (1U << 2)
#define LED_GREEN (1U << 3)

int main()
{ 
  
  SYSCTL->RCGC2 |= (1U << 5);
  SYSCTL->GPIOHSCTL |= (1U << 5);
  GPIOF_HS->DIR |= (LED_BLUE | LED_GREEN | LED_RED);
  GPIOF_HS->DEN |= (LED_BLUE | LED_GREEN | LED_RED);
  //GPIO_PORTF_DATA_R |= LED_BLUE;
 // GPIOF_HS->DATA_Bits[LED_BLUE] = LED_BLUE;
  while(1) {            
     GPIOF_HS->DATA_Bits[LED_RED] ^= LED_RED;
      delay(500000);
  }
}

