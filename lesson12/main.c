#include <stdint.h>

#include "lm4f120h5qr.h"
#include "delay.h"

#define LED_RED (1U << 1)
#define LED_BLUE (1U << 2)
#define LED_GREEN (1U << 3)

typedef struct{
  uint16_t x;
  uint8_t y;
} Point;

Point p1, p2;
int main()
{ 
 
    p1.x = sizeof(Point);
    p1.y = p1.x - 3U;
  
  
  SYSCTL_RCGCGPIO_R |= (1U << 5);
  SYSCTL_GPIOHBCTL_R |= (1U << 5);
  GPIO_PORTF_AHB_DIR_R |= (LED_BLUE | LED_GREEN | LED_RED);
  GPIO_PORTF_AHB_DEN_R |= (LED_BLUE | LED_GREEN | LED_RED);
  //GPIO_PORTF_DATA_R |= LED_BLUE;
  GPIO_PORTB_AHB_DATA_BITS_R[LED_BLUE] = LED_BLUE;
  while(1) {            
      //GPIO_PORTF_DATA_R |= LED_RED;
      //*((unsigned long volatile *)(0x40025000 + (LED_RED << 2))) = LED_RED;
      //*(GPIO_PORTF_DATA_BITS_R + LED_RED) = LED_RED;
    GPIO_PORTB_AHB_DATA_BITS_R[LED_RED] = LED_RED;
      int volatile counter = 0;
      delay();
      //GPIO_PORTF_DATA_R &= ~LED_RED;
      GPIO_PORTB_AHB_DATA_BITS_R[LED_RED] = 0;
      counter = 0;
      delay();
  }
}

