/*
 * 010i2c_master_tx_testing.c
 *
 *  Created on: Sep 15, 2026
 *      Author: ADMIN
 */

#include <stdio.h>
#include <string.h>
#include "stm32f407xx.h"

#define My_ADDR 0x61
#define SlAVE_ADDR 0x68

void delay(void)
{
	for(volatile uint32_t i = 0; i < 500000/2; i++);
}

I2C_Handle_t I2C1Handle;


//some data
uint8_t data[] = "we are hunter haha\n";
/*
 * PB6 -> SCL
 * PB9 -> SDA
 */
void I2C1_GPIOInit(void)
{
	GPIO_Handle_t I2CPins;
	I2CPins.pGPIOx = GPIOB;
	I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2CPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	I2CPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_FAST;

	//scl
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
	GPIO_Init(&I2CPins);

	//sda
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
	GPIO_Init(&I2CPins);
}

void I2C1_Inits(void)
{
	I2C1Handle.pI2Cx = I2C1;
	I2C1Handle.I2C_Config.I2C_DeviceAddress = My_ADDR;
	I2C1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
	I2C1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;

	I2C_Init(&I2C1Handle);
}

void GPIO_ButtonInit(void)
{
	GPIO_Handle_t GpioButton = {0};
	GpioButton.pGPIOx = GPIOA;
	GpioButton.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GpioButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GpioButton.GPIO_PinConfig.GPIO_PinSpeed = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinOPType = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GpioButton);
}

int main(void)
{
    GPIO_ButtonInit();

    I2C1_GPIOInit();

    I2C1_Inits();

    // Enable the I2C peripheral: CR1.PE = 1.
    // Use the actual function name defined in your driver.
    I2C_PeripheralControl(I2C1, ENABLE);

    while (1)
    {
        // Wait for button press.
        while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

        delay();

        // Ignore a press that did not remain HIGH.
        if (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0))
        {
            continue;
        }

        // Send once.
        I2C_MasterSendData(
            &I2C1Handle,
            data,
            strlen((const char *)data),
            SlAVE_ADDR
        );

        // Wait for release before allowing another send.
        while (GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

        delay();
    }
}

