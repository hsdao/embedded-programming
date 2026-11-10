/*
 * 011master_rv_testing.c
 *
 *  Created on: Sep 15, 2026
 *      Author: ADMIN
 */
#include <stdio.h>
#include <string.h>
#include "stm32f407xx.h"

//flag variable
uint8_t rxCompIt = RESET;

#define COMMAND_READ_LENGTH  0x51
#define COMMAND_READ_DATA    0x52
#define My_ADDR 0x61
#define SlAVE_ADDR 0x68
void delay(void)
{
	for(volatile uint32_t i = 0; i < 500000/2; i++);
}

I2C_Handle_t I2C1Handle;

/*
 * PB6 -> SCL
 * PB9 -> SDA
 */

void I2C1_GPIOInit(void)
{
	GPIO_Handle_t I2CPin;
	I2CPin.pGPIOx = GPIOB;
	I2CPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2CPin.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	I2CPin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	I2CPin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	I2CPin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_FAST;

	//scl
	I2CPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
	GPIO_Init(&I2CPin);

	//sda
	I2CPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
	GPIO_Init(&I2CPin);
}

void I2C1_Inits(void)
{
	I2C1Handle.pI2Cx = I2C1;
	I2C1Handle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
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

    //I2C IRQ configuration
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);
    // Enable I2C peripheral
    I2C_PeripheralControl(I2C1, ENABLE);

    // Enable ACK
    I2C_ManageAcking(I2C1, I2C_ACK_ENABLE);

    uint8_t commandReadLength = COMMAND_READ_LENGTH;  // 0x51
    uint8_t commandReadData   = COMMAND_READ_DATA;    // 0x52

    printf("Program started\n");

    while(1)
    {
        uint8_t length_data = 0;

        // 32 bytes:
        // maximum 31 bytes data + 1 byte '\0'
        uint8_t rxBuffer[32] = {0};

        /*
         * =============================
         * WAIT FOR BUTTON
         * =============================
         */

        printf("\nWaiting button...\n");

        while(!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

        printf("Button pressed\n");

        // Small debounce delay
        delay();


        /*
         * =============================
         * STEP 1
         * SEND COMMAND 0x51
         * =============================
         *
         * Ask Arduino:
         * "How many bytes of data do you have?"
         */

        printf("Sending 0x51...\n");

        while(I2C_MasterSendDataIT(
            &I2C1Handle,
            &commandReadLength,
            1,
            SlAVE_ADDR,
			I2C_SR
        ) != I2C_READY);

        printf("0x51 sent\n");


        /*
         * =============================
         * RECEIVE 1 BYTE LENGTH
         * =============================
         */

        printf("Receiving length...\n");

        while(I2C_MasterReceiveDataIT(
            &I2C1Handle,
            &length_data,
            1,
            SlAVE_ADDR,
			I2C_SR
        ) != I2C_READY);

        printf("Length = %u\n", length_data);


        /*
         * Validate received length
         */

        if(length_data == 0 || length_data >= sizeof(rxBuffer))
        {
            printf("Invalid length!\n");

            // Wait until button is released
            while(GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

            delay();

            continue;
        }


        /*
         * =============================
         * STEP 2
         * SEND COMMAND 0x52
         * =============================
         *
         * Ask Arduino:
         * "Now send me the actual data"
         */

        while(I2C_MasterSendDataIT(
            &I2C1Handle,
            &commandReadData,
            1,
            SlAVE_ADDR,
			I2C_SR
        ) != I2C_READY);


        printf("0x52 sent\n");
        rxCompIt = RESET;

        /*
         * =============================
         * RECEIVE ACTUAL DATA
         * =============================
         *
         * Receive exactly length_data bytes
         */

        printf("Receiving data...\n");

        while(I2C_MasterReceiveDataIT(
            &I2C1Handle,
            rxBuffer,
            length_data,
            SlAVE_ADDR,
			I2C_NO_SR
        ) != I2C_READY);

        printf("Data received\n");

        //wait till Rx complete
        while(rxCompIt != SET);

        /*
         * Add string terminator
         *
         * Arduino does NOT send '\0'
         */
        rxBuffer[length_data] = '\0';


        /*
         * Print received string
         */

        printf("Data = %s\n", rxBuffer);


        /*
         * =============================
         * WAIT FOR BUTTON RELEASE
         * =============================
         *
         * Prevent one press from triggering
         * multiple I2C transactions.
         */

        rxCompIt = RESET;
        while(GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

        delay();
    }
}

void I2C1_EV_IRQHandler (void)
{
	I2C_EV_IRQHandling(&I2C1Handle);
}

void I2C1_ER_IRQHandler (void)
{
	I2C_ER_IRQHandling(&I2C1Handle);
}
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv)
{
	if(AppEv == I2C_EV_TX_CMPLT)
	{
		printf("Tx is completed");
	}else if(AppEv == I2C_EV_RX_CMPLT)
	{
		printf("Rx is completed");
		rxCompIt = SET;
	}else if(AppEv == I2C_ERROR_AF)
	{
		printf("Error : Ack failure");
		I2C_CloseSendData(pI2CHandle);

		//generate the stop condition to release the bus
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

		//Hang in infinite loop
		while(1);
	}
}
