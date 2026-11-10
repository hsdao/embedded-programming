#include "delay.h"

void delay(void){
  int volatile counter = 0;
  while(counter < 1000000){++counter;}
}