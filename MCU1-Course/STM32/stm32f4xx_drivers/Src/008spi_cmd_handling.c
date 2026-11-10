/*
 * 008spi_cmd_handling.c
 *
 *  Created on: Sep 2, 2026
 *      Author: ADMIN
 */

/*
 * SPI1_NSS  --> PA4
 * SPI1_SCK  --> PA5
 * SPI1_MISO --> PA6
 * SPI1_MOSI --> PA7
 * ALT function mode -> AF5
 */
#include <string.h>
#include "stm32f407xx.h"


//command codes
#define COMMAND_LED_CTRL		0x50
#define COMMAND_SENSOR_READ		0x51
#define COMMAND_LED_READ		0x52
#define COMMAND_PRINT			0x53
#define COMMAND_ID_READ			0x54

#define LED_ON		1
#define LED_OFF		0

//aduino analog pins
#define ANALOG_PIN0				0
#define ANALOG_PIN1				1
#define ANALOG_PIN2				2
#define ANALOG_PIN3				3
#define ANALOG_PIN4				4

//aduino led
#define LED_PIN 				9

void delay(void)
{
	for(volatile uint32_t i = 0; i < 500000/2; i++);
}

void SPI1_GPIOInit(void)
{
	GPIO_Handle_t SPIPins = {0};
	SPIPins.pGPIOx = GPIOA;
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_FAST;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = GPIO_AF5;

	//NSS
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_4;
	GPIO_Init(&SPIPins);
	//SCK
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
	GPIO_Init(&SPIPins);
	//MISO
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
	GPIO_Init(&SPIPins);
	//MOSI
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
	GPIO_Init(&SPIPins);
}

void SPI1_Inits(void)
{
	SPI_Handle_t SPI1Handle = {0};
	SPI1Handle.pSPIx = SPI1;
	SPI1Handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI1Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
	SPI1Handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI1Handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	SPI1Handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
	SPI1Handle.SPIConfig.SPI_SSM = SPI_SSM_DI;
	SPI1Handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV256;

	SPI_Init(&SPI1Handle);
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

uint8_t SPI_VerifyResponse(uint8_t ackbyte)
{
	if(ackbyte == 0xF5)
	{
		//ack
		return 1;
	}
	return 0;
}

int main(void)
{

	uint8_t dummy_write = 0xff;
	uint8_t dummy_read;
	GPIO_ButtonInit();

	//this function is used to initialize the GPIO pins to behave as SPI1 pins
    SPI1_GPIOInit();

    //This function is used to initialize the SPI1 peripheral parameters
    SPI1_Inits();

    /*
     * making SSOE 1 does NSS output enable
     * the NSS pin is automatically managed by the hardware
     * i.e when SPE = 1, NSS will be pulled to low
     * and NSS pin will be high when SPE = 0
     */
    SPI_SSOEConfig(SPI1, ENABLE);

    while(1)
    {

    	//enable the SPI1 peripheral
    	SPI_PeripheralControl(SPI1, ENABLE);

    	uint8_t ackbyte;
    	uint8_t args[2];

    	//1 . CMD_LED_CTRL	<pin no(1)>		<value(1)>
        //wait till button is pressed
    	while(!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

    	//to avoid button de-bouncing realted issues
    	delay();
    	uint8_t commandcode = COMMAND_LED_CTRL;

    	//send command
    	SPI_SendData(SPI1, &commandcode, 1);

    	//do dummy read to clear off RXNE
    	SPI_RecieveData(SPI1, &dummy_read, 1);

    	//send some dummy bits (1 bytes) to fetch the response from the slave
    	SPI_SendData(SPI1, &dummy_write, 1);

    	//read the ack byte received
    	SPI_RecieveData(SPI1, &ackbyte, 1);

    	if( SPI_VerifyResponse(ackbyte) )
    	{
    		// send arguments
    		args[0] = LED_PIN;
    		args[1] = LED_ON;
    		for(uint8_t i = 0; i < 2; i++)
    		{
        		SPI_SendData(SPI1, &args[i], 1);
        		SPI_RecieveData(SPI1, &dummy_read, 1);
    		}
    	}
    	//end of the COMMAND_LED_CTRL

    	//2. CMD_SENSOR_READ	<analog pin number(1)>
    	while(!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

    	//to avoid button de-bouncing realted issues
    	delay();

    	commandcode = COMMAND_SENSOR_READ;
    	//send command
    	SPI_SendData(SPI1, &commandcode, 1);

    	//do dummy read to clear off RXNE
    	SPI_RecieveData(SPI1, &dummy_read, 1);

    	//send some dummy bits (1 bytes) to fetch the response from the slave
    	SPI_SendData(SPI1, &dummy_write, 1);

    	//read the ack byte received
    	SPI_RecieveData(SPI1, &ackbyte, 1);

    	if( SPI_VerifyResponse(ackbyte) )
    	{
    		args[0] = ANALOG_PIN0;

    		// send arguments
    		SPI_SendData(SPI1, args, 1);

        	//do dummy read to clear off RXNE
        	SPI_RecieveData(SPI1, &dummy_read, 1);

        	//insert some delay to that slave can ready with the data
        	delay();

        	//send some dummy bits (1 bytes) to fetch the response from the slave
        	SPI_SendData(SPI1, &dummy_write, 1);

        	//read the sensor analog received
        	uint8_t analog_read;
        	SPI_RecieveData(SPI1, &analog_read, 1);

    	}
    	//end of the COMMAND_LED_READ

    	// 3. CMD_LED_READ	<pin no(1)>

    	//wait till button is pressed
    	while( !GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

		//to avoid button de-bouncing related issues 200ms of delay
		delay();

		commandcode = COMMAND_LED_READ;

		//send command
		SPI_SendData(SPI1, &commandcode, 1);

		//do dummy read to clear off RXNE
		SPI_RecieveData(SPI1, &dummy_read, 1);

		//send some dummy bits(1 bytes) to fetch the response from the slave
		SPI_SendData(SPI1, &dummy_write, 1);

		//read the ack bytes recieved
		SPI_RecieveData(SPI1, &ackbyte, 1);

		if(SPI_VerifyResponse(ackbyte))
		{
			args[0] = LED_PIN;

			//send arguments
			SPI_SendData(SPI1, args, 1);

			//do dummy read to clear off RXNE
			SPI_RecieveData(SPI1, &dummy_read, 1);

			//insert some delay to that slave can ready with the data
			delay();

        	//send some dummy bits (1 bytes) to fetch the response from the slave
        	SPI_SendData(SPI1, &dummy_write, 1);

        	//read the led status
        	uint8_t led_status;
        	SPI_RecieveData(SPI1, &led_status, 1);

		}
		//end


		//4. CMD_PRINT    <len(2)>    <message(len)>
    	//wait till button is pressed
    	while( !GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

		//to avoid button de-bouncing related issues 200ms of delay
		delay();

		commandcode = COMMAND_PRINT;

		//send command
		SPI_SendData(SPI1, &commandcode, 1);

		//do dummy read to clear off RXNE
		SPI_RecieveData(SPI1, &dummy_read, 1);

		//send some dummy bits(1 bytes) to fetch the response from the slave
		SPI_SendData(SPI1, &dummy_write, 1);

		//read the ack bytes recieved
		SPI_RecieveData(SPI1, &ackbyte, 1);

		//message
		uint8_t message[] = "Hello ! How are you ??";

		if(SPI_VerifyResponse(ackbyte))
		{
			args[0] = strlen((char *)message);

			//send arguments
			SPI_SendData(SPI1, args, 1); // send length

			//do dummy read to clear off RXNE
			SPI_RecieveData(SPI1, &dummy_read, 1);

			delay();

			//send message
        	for(int i = 0; i < args[0]; i++){
        		SPI_SendData(SPI1, &message[i], 1);
        		SPI_RecieveData(SPI1, &dummy_read, 1);
        	}

		}
		//end


	    //5. CMD_ID_READ
		//wait till button is pressed
		while( !GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

		//to avoid button de-bouncing related issues 200ms of delay
		delay();

		commandcode = COMMAND_ID_READ;

		//send command
		SPI_SendData(SPI1, &commandcode, 1);

		//do dummy read to clear off RXNE
		SPI_RecieveData(SPI1, &dummy_read, 1);

		//send some dummy bits(1 bytes) to fetch the response from the slave
		SPI_SendData(SPI1, &dummy_write, 1);

		//read the ack bytes recieved
		SPI_RecieveData(SPI1, &ackbyte, 1);

		uint8_t id[11];
		if(SPI_VerifyResponse(ackbyte))
		{
			for(int i = 0; i < 10; i++){
				//send dummy byte to fetch data from slave
				SPI_SendData(SPI1, &dummy_write, 1);
				SPI_RecieveData(SPI1, &id[i], 1);
			}

			id[10] = '\0';

		}
		//lets confirm SPI is not busy
		while(SPI_GetFlagStatus(SPI1, SPI_BUSY_FLAG));

		//Disable the SPI1 peripheral
		SPI_PeripheralControl(SPI1, DISABLE);
    }

    return 0;
}
