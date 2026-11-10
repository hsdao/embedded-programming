/*
 * 001leg_toggle.c
 *
 *  Created on: Aug 20, 2026
 *      Author: ADMIN
 */
#include "stm32f407xx.h"

void delay(void)
{
	for(uint32_t i = 0; i < 500000/2; i++);
}


int main(void){
	// LED
	GPIO_Handle_t GpioLed;
	GpioLed.pGPIOx = GPIOA;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PerClkControl(GPIOA, ENABLE);
	GPIO_Init(&GpioLed);

	// Button
	GPIO_Handle_t GpioButton;
	GpioButton.pGPIOx = GPIOD;
	GpioButton.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_4;
	GpioButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GpioButton.GPIO_PinConfig.GPIO_PinSpeed = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinOPType = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PerClkControl(GPIOD, ENABLE);
	GPIO_Init(&GpioButton);
	while(1){
		if(GPIO_ReadFromInputPin(GPIOD, GPIO_PIN_NO_4) == 0)
		{
			delay();

			GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_14);

			while(GPIO_ReadFromInputPin(GPIOD, GPIO_PIN_NO_4) == 0);

			delay();
		}
	}
	return 0;
}
