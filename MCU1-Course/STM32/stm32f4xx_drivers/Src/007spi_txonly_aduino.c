/*
 * 007spi_txonly_aduino.c
 *
 *  Created on: Aug 27, 2026
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

// 1. Đã thêm từ khóa 'volatile' để tránh Compiler tối ưu hóa làm mất hàm delay
void delay(void)
{
	for(volatile uint32_t i = 0; i < 500000; i++);
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
    // 2. Đã thêm {0} để khởi tạo an toàn, tránh biến rác trong RAM
	GPIO_Handle_t GpioButton = {0};
	GpioButton.pGPIOx = GPIOA;
	GpioButton.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GpioButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GpioButton.GPIO_PinConfig.GPIO_PinSpeed = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinOPType = 0;
	GpioButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GpioButton);
}
// Chi dung khi khong can du lieu nhan ve.
static void SPI_DiscardReceivedData(SPI_RegDef_t *pSPIx)
{
    volatile uint32_t dummy;

    // Doc DR truoc, sau do doc SR de clear OVR.
    dummy = pSPIx->SPI_DR;
    dummy = pSPIx->SPI_SR;

    (void)dummy;
}

int main(void)
{
    char user_data[] = "Hanh choaaa";
    uint8_t dataLen = (uint8_t)strlen(user_data);

    SPI1_GPIOInit();
    SPI1_Inits();
    GPIO_ButtonInit();  // Khoi tao nut PA0

    SPI_SSOEConfig(SPI1, ENABLE);

    while (1)
    {
        // 1. Cho bam nut: PA0 = HIGH
        while (GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0) == 0);

        // 2. Chong doi nut, kiem tra lai
        delay();

        if (GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0) == 0)
        {
            continue;
        }

        // 3. Bat SPI, chon Arduino
        SPI_PeripheralControl(SPI1, ENABLE);

        // 4. Gui do dai va noi dung
        SPI_SendData(SPI1, &dataLen, 1);
        SPI_SendData(SPI1, (uint8_t *)user_data, dataLen);

        // 5. Cho truyen xong
        while (SPI_GetFlagStatus(SPI1, SPI_TXE_FLAG) == 0);
        while (SPI_GetFlagStatus(SPI1, SPI_BUSY_FLAG) != 0);

        SPI_PeripheralControl(SPI1, DISABLE);

        // Ham da them o cau tra loi truoc
        SPI_DiscardReceivedData(SPI1);

        // 6. Cho tha nut on dinh de tranh gui lai khi giu nut
        do
        {
            while (GPIO_ReadFromInputPin(
                       GPIOA, GPIO_PIN_NO_0) != 0);

            delay();
        }
        while (GPIO_ReadFromInputPin(
                   GPIOA, GPIO_PIN_NO_0) != 0);
    }
}
