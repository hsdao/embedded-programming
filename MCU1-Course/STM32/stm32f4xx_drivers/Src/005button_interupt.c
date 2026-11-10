/*
 * 005button_interupt.c
 *
 *  Created on: Aug 28, 2026
 *      Author: ADMIN
 */

#include "stm32f407xx.h"

#define HIGH 1
#define LOW 0
#define BTN_PRESSED LOW

void delay(void)
{
	for(volatile uint32_t i = 0; i < 500000/2; i++);
}


int main(void){
	// LED
	GPIO_Handle_t GpioLed = {0};
	GpioLed.pGPIOx = GPIOD;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PerClkControl(GPIOD, ENABLE);
	GPIO_Init(&GpioLed);

	// Button
	GPIO_Handle_t GpioButton = {0};
	GpioButton.pGPIOx = GPIOD;
	GpioButton.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
	GpioButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
	GpioButton.GPIO_PinConfig.GPIO_PinSpeed = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinOPType = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

	GPIO_PerClkControl(GPIOD, ENABLE);
	SYSCFG_PCLK_EN();
	GPIO_Init(&GpioButton);

	//IRQ configuration
	GPIO_IRQPriorityConfig(IRQ_NO_EXTI9_5,NVIC_IRQ_PRI15);
	GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5, ENABLE);

	while(1);
}
void EXTI9_5_IRQHandler(void)
{
	delay();
	GPIO_IRQHandling(GPIO_PIN_NO_5);
	GPIO_ToggleOutputPin(GPIOD,GPIO_PIN_NO_12);
}
