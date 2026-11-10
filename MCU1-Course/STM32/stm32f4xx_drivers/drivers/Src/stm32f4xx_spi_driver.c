/*
 * stm32f4xx_spi_driver.c
 *
 *  Created on: Aug 22, 2026
 *      Author: ADMIN
 */


#include <stm32f407xx_spi_driver.h>

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);

/*
 * SPI_PeriClockControl
 */

void SPI_PerClkControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		if(pSPIx == SPI1){
			SPI1_PCLK_EN();
		}else if(pSPIx == SPI2)
		{
			SPI2_PCLK_EN();
		}else if(pSPIx == SPI3)
		{
			SPI3_PCLK_EN();
		}
	}else
	{
		if(pSPIx == SPI1){
			SPI1_PCLK_DI();
		}else if(pSPIx == SPI2)
		{
			SPI2_PCLK_DI();
		}else if(pSPIx == SPI3)
		{
			SPI3_PCLK_DI();
		}
	}
}
/*
 * Init and De-init
 */
void SPI_Init(SPI_Handle_t *pSPIHandle)
{
	//peripheral clock enable
	SPI_PerClkControl(pSPIHandle->pSPIx, ENABLE);

	//Configure the SPI_CR1 register

	uint32_t tempreg = 0;
	//1. configure the device mode
	tempreg |= pSPIHandle->SPIConfig.SPI_DeviceMode << 2;

	//2.configure the bus config
	if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD)
	{
		tempreg &= ~(1 << 15);
	}else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD)
	{
		tempreg |= (1 << SPI_CR1_BIDIMODE);
	}else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY)
	{
		tempreg &= ~(1 << SPI_CR1_BIDIMODE);
		tempreg |= (1 << SPI_CR1_RXONLY);
	}
	//3. SPI_SclkSpeed
	tempreg |= pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR;
	//4. Configure the DFF
	tempreg |= pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF;
	//5. Configure the CPOL
	tempreg |= pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL;
	//6. Configure the CPHA
	tempreg |= pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA;
	//7. Configure the SSM
	tempreg |= pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM;
	pSPIHandle->pSPIx->SPI_CR1 = tempreg;
}
void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
	if(pSPIx == SPI1)
	{
		SPI1_PCLK_RESET();
	}else if(pSPIx == SPI2)
	{
		SPI2_PCLK_RESET();
	}else if(pSPIx == SPI3)
	{
		SPI3_PCLK_RESET();
	}
}

/*
 * Data read and receive
 */
void SPI_RecieveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len)
{
	while(Len > 0){
		//1. Wait until RXNE is set
		while( !((pSPIx->SPI_SR >> SPI_SR_RXNE) & 1) );

		//2. check the dff bit in cr1
		if( (pSPIx->SPI_CR1 >> SPI_CR1_DFF) & 1)
		{
			//16 bit dff
			//1.load the data from DR to RxBuffer
			*((uint16_t*)pRxBuffer) = pSPIx->SPI_DR;
			pRxBuffer+=2;
			Len -= 2;
		}else
		{
			// 8 bit DFF
			*pRxBuffer = pSPIx->SPI_DR;
			pRxBuffer++;
			Len -= 1;
		}
	}
}

void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len)
{
	while(Len > 0){
		while( !((pSPIx->SPI_SR >> SPI_SR_TXE) & 1) );
		if( (pSPIx->SPI_CR1 >> SPI_CR1_DFF) & 1)
		{
			pSPIx->SPI_DR = *((uint16_t*)pTxBuffer);
			pTxBuffer+=2;
			Len -= 2;
		}else
		{
			pSPIx->SPI_DR = *pTxBuffer;
			pTxBuffer++;
			Len -= 1;
		}
	}
}

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len)
{
	uint8_t state = pSPIHandle->TxState;
	if(state != SPI_BUSY_IN_TX){
		//1. Save the Tx buffer address and Len information in some global variables
		pSPIHandle->pTxBuffer = pTxBuffer;
		pSPIHandle->TxLen = Len;

		//2.Mark the SPI state as busy in transmission so that
		// no other code can take over same SPI peripheral until transmission is over
		pSPIHandle->TxState = SPI_BUSY_IN_TX;

		//3.Enable the TXEIE control bit to get interrupt whenever TXE flag is set in SR
		pSPIHandle->pSPIx->SPI_CR2 |= (1 << SPI_CR2_TXEIE);

		//4.Data Transmission will be handled by the ISR code
	}

	return state;
}
uint8_t SPI_RecieveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len)
{
	uint8_t state = pSPIHandle->RxState;
	if(state != SPI_BUSY_IN_RX){
		//1. Save the Rx buffer address and Len information in some global variables
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = Len;

		//2.Mark the SPI state as busy in transmission so that
		// no other code can take over same SPI peripheral until recieve is over
		pSPIHandle->RxState = SPI_BUSY_IN_RX;

		//3.Enable the TXEIE control bit to get interrupt whenever TXE flag is set in SR
		pSPIHandle->pSPIx->SPI_CR2 |= (1 << SPI_CR2_RXNEIE);

		//4.Data Transmission will be handled by the ISR code
	}

	return state;
}

void SPI_IRQHandling(SPI_Handle_t *pSPIHandle)
{
	uint8_t temp1, temp2;
	//first lets check for TXE
	temp1 = pSPIHandle->pSPIx->SPI_SR & (1 << SPI_SR_TXE);
	temp2 = pSPIHandle->pSPIx->SPI_SR & (1 << SPI_CR2_TXEIE);

	if(temp1 && temp2)
	{
		// handle TXE
		spi_txe_interrupt_handle(pSPIHandle);
	}


	// check for RXE
	temp1 = pSPIHandle->pSPIx->SPI_SR & (1 << SPI_SR_RXNE);
	temp2 = pSPIHandle->pSPIx->SPI_SR & (1 << SPI_CR2_RXNEIE);

	if(temp1 && temp2)
	{
		// handle RXE
		spi_rxne_interrupt_handle(pSPIHandle);
	}


	// check for OVR
	temp1 = pSPIHandle->pSPIx->SPI_SR & (1 << SPI_SR_OVR);
	temp2 = pSPIHandle->pSPIx->SPI_SR & (1 << SPI_CR2_ERRIE);

	if(temp1 && temp2)
	{
		// handle ovr error
		spi_ovr_err_interrupt_handle(pSPIHandle);
	}

}











/*
 * IRQ Configuration and ISR handling
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{

	if(EnorDi == ENABLE)
	{
		if(IRQNumber <= 31)
		{
			*NVIC_ISER0 |= (1 << IRQNumber);
		}else if(IRQNumber > 31 && IRQNumber < 64)
		{
			*NVIC_ISER1 |= (1 << (IRQNumber % 32));
		}else if(IRQNumber >= 64 && IRQNumber < 96)
		{
			*NVIC_ISER2 |= (1 << (IRQNumber % 32));
		}
	}else
	{
		if(IRQNumber <= 31)
		{
			*NVIC_ICER0 = (1 << IRQNumber);
		}else if(IRQNumber > 31 && IRQNumber < 64)
		{
			*NVIC_ICER1 = (1 << (IRQNumber % 32));
		}else if(IRQNumber >= 64 && IRQNumber < 96)
		{
			*NVIC_ICER2 = (1 << (IRQNumber % 32));
		}
	}
}

void SPI_IRQPriorityConfig(uint8_t IRQPriority, uint8_t IRQNumber){
	//1. Find out the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
	*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t Enordi)
{
	if(Enordi == ENABLE)
	{
		pSPIx->SPI_CR1 |= (1 << SPI_CR1_SPE);
	}else
	{
		pSPIx->SPI_CR1 &= ~(1 << SPI_CR1_SPE);
	}
}

void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t Enordi)
{
	if(Enordi == ENABLE)
	{
		pSPIx->SPI_CR1 |= (1 << SPI_CR1_SSI);
	}else
	{
		pSPIx->SPI_CR1 &= ~(1 << SPI_CR1_SSI);
	}
}
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t Enordi)
{
	if(Enordi == ENABLE)
	{
		pSPIx->SPI_CR2 |= (1 << SPI_CR2_SSOE);
	}else
	{
		pSPIx->SPI_CR2 &= ~(1 << SPI_CR2_SSOE);
	}
}
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
	if(pSPIx->SPI_SR & FlagName)
	{
		return SET; // SET = 1
	}
	return RESET;   // RESET = 0
}

//some helper function implementation
static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	//check the DFF bit in CR1
	if( (pSPIHandle->pSPIx->SPI_CR1 >> SPI_CR1_DFF) & 1)
	{
		//16 bit DFF
		//1. Load the data in to the DR
		pSPIHandle->pSPIx->SPI_DR = *((uint16_t*)pSPIHandle->pTxBuffer);
		pSPIHandle->pTxBuffer+=2;
		pSPIHandle->TxLen -= 2;
	}else
	{
		//8 bit DFF
		//1. Load the data in to the DR
		pSPIHandle->pSPIx->SPI_DR = *(pSPIHandle->pTxBuffer);
		pSPIHandle->pTxBuffer++;
		pSPIHandle->TxLen -= 1;
	}

	if(! pSPIHandle->TxLen)
	{
		//TxLen is zero, so close the spi tranmission and inform the
		//application that TX is over
		//this prevents interrupts from setting up of TXE flag
		SPI_CloseTransmission(pSPIHandle);
		SPI_ApplicatioEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
	}
}
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	//1. check the dff bit in cr1
	if( (pSPIHandle->pSPIx->SPI_CR1 >> SPI_CR1_DFF) & 1)
	{
		//16 bit dff
		//1.load the data from DR to RxBuffer
		*((uint16_t*)pSPIHandle->pRxBuffer) = pSPIHandle->pSPIx->SPI_DR;
		pSPIHandle->pRxBuffer+=2;
		pSPIHandle->RxLen -= 2;
	}else
	{
		// 8 bit DFF
		*pSPIHandle->pRxBuffer = pSPIHandle->pSPIx->SPI_DR;
		pSPIHandle->pRxBuffer++;
		pSPIHandle->RxLen -= 1;
	}

	if(! pSPIHandle->RxLen)
	{
		//RxLen is zero, so close the spi tranmission and inform the
		//application that RX is over
		//this prevents interrupts from setting up of RXE flag
		SPI_CloseReception(pSPIHandle);
		SPI_ApplicatioEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
	}
}
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	uint8_t temp;
	//1.clear the ovr flag
	if(pSPIHandle->TxState != SPI_BUSY_IN_TX)
	{
		temp = pSPIHandle->pSPIx ->SPI_DR;
		temp = pSPIHandle->pSPIx->SPI_SR;
	}
	(void)temp;
	//2.inform the application
	SPI_ApplicatioEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}
void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->SPI_CR2 &= ~ (1 << SPI_CR2_TXEIE);
	pSPIHandle->pTxBuffer = NULL;
	pSPIHandle->TxLen = 0;
	pSPIHandle->TxState = SPI_READY;
}
void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->SPI_CR2 &= ~ (1 << SPI_CR2_RXNEIE);
	pSPIHandle->pRxBuffer = NULL;
	pSPIHandle->RxLen = 0;
	pSPIHandle->RxState = SPI_READY;
}
void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
	uint8_t temp;
	temp = pSPIx ->SPI_DR;
	temp = pSPIx->SPI_SR;
	(void)temp;
}

__weak void SPI_ApplicatioEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv)
{

	//This is a weak implementation . the application may override this function

}
