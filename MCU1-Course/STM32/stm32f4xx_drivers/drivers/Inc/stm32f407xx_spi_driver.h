/*
 * stm32f4xx_spi_driver.h
 *
 *  Created on: Aug 22, 2026
 *      Author: ADMIN
 */

#ifndef INC_STM32F407XX_SPI_DRIVER_H_
#define INC_STM32F407XX_SPI_DRIVER_H_



#include "stm32f407xx.h"
/*
 * This is a Configuration structure for a SPI
 */
typedef struct
{
	uint8_t SPI_DeviceMode; /* Cấu hình bit MSTR */
	uint8_t SPI_BusConfig;  /* Cấu hình các bit BIDIMODE, BIDIOE, RXONLY (Full/Half-duplex) */
	uint8_t SPI_SclkSpeed;  /* Cấu hình các bit BR[2:0] */
	uint8_t SPI_DFF;        /* Cấu hình bit DFF */
	uint8_t SPI_CPOL;       /* Cấu hình bit CPOL */
	uint8_t SPI_CPHA;       /* Cấu hình bit CPHA */
	uint8_t SPI_SSM;        /* Cấu hình bit SSM */
}SPI_Config_t;


/*
 * This is a Handle structure for a SPI
 */




typedef struct
{
	SPI_RegDef_t *pSPIx;
	SPI_Config_t SPIConfig;
	uint8_t *pTxBuffer; /* !<To store the app. Tx buffer address > */
	uint8_t *pRxBuffer; /* !<To store the app. Rx buffer address > */
	uint32_t TxLen;
	uint32_t RxLen;
	uint8_t TxState;
	uint8_t RxState;
}SPI_Handle_t;


/*
 * @SPI_DeviceMode
 */
#define SPI_DEVICE_MODE_MASTER		1
#define SPI_DEVICE_MODE_SLAVE		0

/*
 * @SPI_BusConfig
 */
#define SPI_BUS_CONFIG_FD						1
#define SPI_BUS_CONFIG_HD						2
#define SPI_BUS_CONFIG_SIMPLEX_RXONLY			3

/*
 * @SPI_SclkSpeed
 */
#define SPI_SCLK_SPEED_DIV2				0
#define SPI_SCLK_SPEED_DIV4				1
#define SPI_SCLK_SPEED_DIV8				2
#define SPI_SCLK_SPEED_DIV16			3
#define SPI_SCLK_SPEED_DIV32			4
#define SPI_SCLK_SPEED_DIV64			5
#define SPI_SCLK_SPEED_DIV128			6
#define SPI_SCLK_SPEED_DIV256			7
/*
 * @SPI_DFF
 */
#define SPI_DFF_8BITS			0
#define SPI_DFF_16BITS			1
/*
 * @SPI_CPOL
 */
#define	SPI_CPOL_HIGH			1
#define	SPI_CPOL_LOW			0
/*
 * @SPI_CPHA
 */
#define	SPI_CPHA_HIGH			1
#define	SPI_CPHA_LOW			0
/*
 * @SPI_SSM
 */
#define	SPI_SSM_EN			1
#define	SPI_SSM_DI			0


/*
 * @SPI_SR_BSY
 */
#define SPI_TXE_FLAG        (1 << SPI_SR_TXE)
#define SPI_RXNE_FLAG       (1 << SPI_SR_RXNE)
#define SPI_BUSY_FLAG       (1 << SPI_SR_BSY)

/*
 * @Possible SPI Application States
 */
#define SPI_READY			0
#define SPI_BUSY_IN_RX		1
#define SPI_BUSY_IN_TX		2

/*
 * Possioble SPI Application events
 */
#define  SPI_EVENT_TX_CMPLT		1
#define  SPI_EVENT_RX_CMPLT		2
#define  SPI_EVENT_OVR_ERR		3
#define  SPI_EVENT_CRC_ERR		4


/************
 * 							APIs supported by this driver
 * 				For more information about the APIs check the function definitions
 ***********/
void SPI_PerClkControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi); /* Peripheral Clock setup */

/*
 * Init and De-init
 */
void SPI_Init(SPI_Handle_t *pSPIHandle);
void SPI_DeInit(SPI_RegDef_t *pSPIOx);

/*
 * Data read and receive
 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len);
void SPI_RecieveData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len);

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len);
uint8_t SPI_RecieveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len);


/*
 * IRQ Configuration and ISR handling
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void SPI_IRQPriorityConfig(uint8_t IRQPriority, uint8_t IRQNumber);
void SPI_IRQHandling(SPI_Handle_t *pHandle);

/*
 * Other Peripheral Control APIs
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t Enordi);

void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t Enordi);

void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t Enordi);

uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName);

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx);
void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle);
void SPI_CloseReception(SPI_Handle_t *pSPIHandle);
/*
 * Application callback
 */
void SPI_ApplicatioEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv);
#endif /* INC_STM32F407XX_SPI_DRIVER_H_ */
