/*
 * 001leg_toggle.c
 *
 *  Created on: Aug 20, 2026
 *      Author: ADMIN
 */
#include "stm32f407xx.h"

void delay(void)
{
	for(uint32_t i = 0; i < 1000000; i++);
}

void print_number(int cnt){
	GPIO_WriteToOutputPort(GPIOD, DISABLE);
	if((cnt >> 0) & 0x00000001)
	{
		GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_13);
	}
	if((cnt >> 1) & 0x00000001)
	{
		GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_12);
	}
	if((cnt >> 2) & 0x00000001)
	{
		GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_14);
	}
	if((cnt >> 3) & 0x00000001)
	{
		GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_15);
	}
}
int main(void){
	// LED3
	GPIO_Handle_t GpioLed3;
	GpioLed3.pGPIOx = GPIOD;
	GpioLed3.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GpioLed3.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed3.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_FAST;
	GpioLed3.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed3.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PerClkControl(GPIOD, ENABLE);
	GPIO_Init(&GpioLed3);

	// LED4
	GPIO_Handle_t GpioLed4;
	GpioLed4.pGPIOx = GPIOD;
	GpioLed4.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GpioLed4.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed4.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_FAST;
	GpioLed4.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed4.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PerClkControl(GPIOD, ENABLE);
	GPIO_Init(&GpioLed4);

	// LED5
	GPIO_Handle_t GpioLed5;
	GpioLed5.pGPIOx = GPIOD;
	GpioLed5.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	GpioLed5.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed5.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_FAST;
	GpioLed5.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed5.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PerClkControl(GPIOD, ENABLE);
	GPIO_Init(&GpioLed5);


	// LED6
	GPIO_Handle_t GpioLed6;
	GpioLed6.pGPIOx = GPIOD;
	GpioLed6.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
	GpioLed6.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed6.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_FAST;
	GpioLed6.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed6.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PerClkControl(GPIOD, ENABLE);
	GPIO_Init(&GpioLed6);
	// Button
	GPIO_Handle_t GpioButton;
	GpioButton.pGPIOx = GPIOA;
	GpioButton.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GpioButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GpioButton.GPIO_PinConfig.GPIO_PinSpeed = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinOPType = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PerClkControl(GPIOA, ENABLE);
	GPIO_Init(&GpioButton);
	uint8_t cnt = 0;
	while(1){
		if(GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0)){
			while(1){
				delay();
				cnt++;
				cnt = cnt % 16;
				print_number(cnt);
				delay();
			}
		}
	}
	return 0;
}
