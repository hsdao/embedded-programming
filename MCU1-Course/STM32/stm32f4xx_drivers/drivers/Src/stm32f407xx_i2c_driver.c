/*
 * stm32f407xx_i2c_driver.c
 *
 *  Created on: Sep 8, 2026
 *      Author: ADMIN
 */
#include "stm32f407xx_spi_driver.h"
#include <stdio.h>


static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);

static void I2C_ExcuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ExcuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);

static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle);

static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle);

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)
{
	pI2Cx->I2C_CR1 |= (1 << I2C_CR1_START);
}

static void I2C_ExcuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
	SlaveAddr = SlaveAddr << 1;
	SlaveAddr &= ~(1);
	pI2Cx->I2C_DR = SlaveAddr;
}
static void I2C_ExcuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
	SlaveAddr = SlaveAddr << 1;
	SlaveAddr |= 1;
	pI2Cx->I2C_DR = SlaveAddr;
}

static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle)
{
	uint32_t dummyRead;
	//check for device mode
	if(pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_MSL))
	{
		//device is in master mode
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			if(pI2CHandle->RxSize == 1)
			{
				//first disable the ack
				pI2CHandle->pI2Cx->I2C_CR1 &= ~(1 << 10);
			}
				//clear the ADDR flag
				dummyRead = pI2CHandle->pI2Cx->I2C_SR1;
				dummyRead = pI2CHandle->pI2Cx->I2C_SR2;
				(void)dummyRead;
		}
		else
		{
			//first disable the ack
			pI2CHandle->pI2Cx->I2C_CR1 &= ~(1 << 10);

			//clear the ADDR flag
			dummyRead = pI2CHandle->pI2Cx->I2C_SR1;
			dummyRead = pI2CHandle->pI2Cx->I2C_SR2;
			(void)dummyRead;
		}

	}else
	{
		//device is in slave mode
		//clear the ADDR flag
		dummyRead = pI2CHandle->pI2Cx->I2C_SR1;
		dummyRead = pI2CHandle->pI2Cx->I2C_SR2;
		(void)dummyRead;
	}
}

void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx)
{
	pI2Cx->I2C_CR1 |= (1 << I2C_CR1_STOP);
}

/*********************************************************************
 * @fn      		  - I2C_PeripheralControl
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pI2Cx->I2C_CR1 |= (1 << I2C_CR1_PE);
		//pI2cBaseAddress->CR1 |= I2C_CR1_PE_Bit_Mask;
	}else
	{
		pI2Cx->I2C_CR1 &= ~(1 << 0);
	}

}


/*********************************************************************
 * @fn      		  - I2C_PeriClockControl
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		if(pI2Cx == I2C1)
		{
			I2C1_PCLK_EN();
		}else if (pI2Cx == I2C2)
		{
			I2C2_PCLK_EN();
		}else if (pI2Cx == I2C3)
		{
			I2C3_PCLK_EN();
		}
	}
	else
	{
		//TODO
	}

}

/*********************************************************************
 * @fn      		  - I2C_Init
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void I2C_Init(I2C_Handle_t *pI2CHandle)
{
	uint32_t tempreg = 0;
	uint8_t trise;


	//enable the clock for the i2cx peripheral
	I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);

	//ack control bit
	tempreg |=  pI2CHandle->I2C_Config.I2C_ACKControl << 10;
	pI2CHandle->pI2Cx->I2C_CR1 = tempreg;

	//configure the FREQ field of CR2
	tempreg = 0;
	tempreg = RCC_GetPCLK1Value() / 1000000U;
	pI2CHandle->pI2Cx->I2C_CR2 = tempreg & 0x3F;

	//program the device own address
	tempreg = pI2CHandle->I2C_Config.I2C_DeviceAddress << 1;
	tempreg |= 1 << 14;
	pI2CHandle->pI2Cx->I2C_OAR1 = tempreg;

	//CCR calculation
	uint16_t ccr_value = 0;
	tempreg = 0;
	if(pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
	{
		//mode is stardard mode
		ccr_value = RCC_GetPCLK1Value() / (2 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		tempreg |= ccr_value & 0xFFF;
	}else
	{
		//mode is fast mode
		tempreg |= (1 << 15);
		tempreg |= (pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14);
		if(pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
		{
			ccr_value = RCC_GetPCLK1Value() / (3 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		}else
		{
			ccr_value = RCC_GetPCLK1Value() / (25 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		}
		tempreg |= ccr_value & 0xFFF;
	}
	pI2CHandle->pI2Cx->I2C_CCR = tempreg;

	//TRISE configuration
	if(pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
	{
		//mode is stardard mode
		trise = (RCC_GetPCLK1Value() / 1000000U) + 1;

	}else
	{
		//mode is fast mode
		trise = (uint8_t)(
		    ((uint64_t)RCC_GetPCLK1Value() * 300U)
		    / 1000000000ULL
		    + 1U
		);
	}
	pI2CHandle->pI2Cx->I2C_TRISE = (trise & 0x3F);
}


/*********************************************************************
 * @fn      		  - I2C_DeInit
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{
	if(pI2Cx == I2C1)
	{
		I2C1_PCLK_DI();
	}else if(pI2Cx == I2C2)
	{
		I2C2_PCLK_DI();
	}else if(pI2Cx == I2C3)
	{
		I2C3_PCLK_DI();
	}
}

void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if(EnorDi == I2C_ACK_ENABLE)
    {
        // Set ACK bit in CR1
        pI2Cx->I2C_CR1 |= (1 << I2C_CR1_ACK);
    }
    else
    {
        // Clear ACK bit in CR1
        pI2Cx->I2C_CR1 &= ~(1 << I2C_CR1_ACK);
    }
}
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx,
                        uint32_t FlagName)
{
    return (pI2Cx->I2C_SR1 & FlagName) ? SET : RESET;
}

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle,
                        uint8_t *pTxbuffer,
                        uint32_t Len,
                        uint8_t SlaveAddr,
						uint8_t Sr)
{
	printf("SEND: generate START\n");

	printf("Before START: CR1=0x%08lX SR1=0x%08lX SR2=0x%08lX\n",
	       pI2CHandle->pI2Cx->I2C_CR1,
	       pI2CHandle->pI2Cx->I2C_SR1,
	       pI2CHandle->pI2Cx->I2C_SR2);

	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	printf("After START:  CR1=0x%08lX SR1=0x%08lX SR2=0x%08lX\n",
	       pI2CHandle->pI2Cx->I2C_CR1,
	       pI2CHandle->pI2Cx->I2C_SR1,
	       pI2CHandle->pI2Cx->I2C_SR2);

	printf("SEND: waiting SB\n");

	while(!I2C_GetFlagStatus(
	    pI2CHandle->pI2Cx,
	    I2C_FLAG_SB
	));

    printf("SEND: SB OK\n");

    I2C_ExcuteAddressPhaseWrite(
        pI2CHandle->pI2Cx,
        SlaveAddr
    );

    printf("SEND: waiting ADDR\n");

    while(!I2C_GetFlagStatus(
        pI2CHandle->pI2Cx,
        I2C_FLAG_ADDR
    ));

    printf("SEND: ADDR OK\n");

    I2C_ClearADDRFlag(pI2CHandle);

    while(Len > 0)
    {
        printf("SEND: waiting TXE\n");

        while(!I2C_GetFlagStatus(
            pI2CHandle->pI2Cx,
            I2C_FLAG_TXE
        ));

        printf("SEND: TXE OK\n");

        pI2CHandle->pI2Cx->I2C_DR = *pTxbuffer;

        pTxbuffer++;
        Len--;
    }

    printf("SEND: waiting final TXE\n");

    while(!I2C_GetFlagStatus(
        pI2CHandle->pI2Cx,
        I2C_FLAG_TXE
    ));

    printf("SEND: waiting BTF\n");

    while(!I2C_GetFlagStatus(
        pI2CHandle->pI2Cx,
        I2C_FLAG_BTF
    ));

    printf("SEND: generate STOP\n");
    if(Sr == I2C_NO_SR)
    I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

    printf("SEND: DONE\n");
}

void I2C_MasterRecieveData(I2C_Handle_t *pI2CHandle,
                           uint8_t *pRxbuffer,
                           uint32_t Len,
                           uint8_t SlaveAddr,
						   uint8_t Sr)
{
    uint32_t dummyRead;

    /* 1. Generate START */
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    /* 2. Wait until SB = 1 */
    while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB));

    /* 3. Send slave address + READ */
    I2C_ExcuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);

    /* 4. Wait until ADDR = 1 */
    while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));


    /*
     * ==========================================
     * CASE 1: RECEIVE ONLY 1 BYTE
     * ==========================================
     */
    if(Len == 1)
    {
        /* Disable ACK because this is the last byte */
        I2C_ManageAcking(
            pI2CHandle->pI2Cx,
            I2C_ACK_DISABLE
        );

        /* Clear ADDR */
        dummyRead = pI2CHandle->pI2Cx->I2C_SR1;
        dummyRead = pI2CHandle->pI2Cx->I2C_SR2;
        (void)dummyRead;

        if(Sr == I2C_NO_SR)
        /* Generate STOP */
        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

        /* Wait for byte */
        while(!I2C_GetFlagStatus(
            pI2CHandle->pI2Cx,
            I2C_FLAG_RXNE
        ));

        /* Read byte */
        *pRxbuffer = pI2CHandle->pI2Cx->I2C_DR;
    }


    /*
     * ==========================================
     * CASE 2: RECEIVE MORE THAN 1 BYTE
     * ==========================================
     */
    else if(Len > 1)
    {
        /*
         * IMPORTANT:
         * ACK must still be ENABLED here.
         */
        I2C_ManageAcking(
            pI2CHandle->pI2Cx,
            I2C_ACK_ENABLE
        );

        /*
         * Clear ADDR manually.
         * Do NOT disable ACK here.
         */
        dummyRead = pI2CHandle->pI2Cx->I2C_SR1;
        dummyRead = pI2CHandle->pI2Cx->I2C_SR2;
        (void)dummyRead;


        for(uint32_t i = Len; i > 0; i--)
        {
            /* Wait until a byte arrives */
            while(!I2C_GetFlagStatus(
                pI2CHandle->pI2Cx,
                I2C_FLAG_RXNE
            ));


            /*
             * When only 2 bytes remain,
             * prepare to NACK the final byte.
             */
            if(i == 2)
            {
                I2C_ManageAcking(
                    pI2CHandle->pI2Cx,
                    I2C_ACK_DISABLE
                );
                if(Sr == I2C_NO_SR)
                I2C_GenerateStopCondition(
                    pI2CHandle->pI2Cx
                );
            }


            /* Read received byte */
            *pRxbuffer = pI2CHandle->pI2Cx->I2C_DR;

            pRxbuffer++;
        }
    }


    /*
     * Restore ACK for next transaction
     */
    if(pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE)
    {
        I2C_ManageAcking(
            pI2CHandle->pI2Cx,
            I2C_ACK_ENABLE
        );
    }
}

/*********************************************************************
 * @fn      		  - I2C_MasterSendDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -  Complete the below code . Also include the function prototype in header file

 */
uint8_t  I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pTxBuffer, uint32_t Len,uint8_t SlaveAddr,uint8_t Sr)
{

	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pTxBuffer = pTxBuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= ( 1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= ( 1 << I2C_CR2_ITEVTEN);

	}

	return busystate;

}
/*********************************************************************
 * @fn      		  - I2C_MasterReceiveDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the below code . Also include the fn prototype in header file

 */
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr,uint8_t Sr)
{

	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pRxBuffer = pRxBuffer;
		pI2CHandle->RxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
		pI2CHandle->RxSize = Len; //Rxsize is used in the ISR code to manage the data reception
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= ( 1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= ( 1 << I2C_CR2_ITEVTEN);

	}

	return busystate;
}

void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
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

void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority){
	//1. Find out the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
	*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}

static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle)
{
	if(pI2CHandle->TxLen > 0)
	{
		//1. load the data into DR
		pI2CHandle->pI2Cx->I2C_DR = *(pI2CHandle->pTxBuffer);
		//2.decrement the TxLen
		pI2CHandle->TxLen--;
		//3.Increment the buffer address
		pI2CHandle->pTxBuffer++;
	}
}

static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle)
{
	if(pI2CHandle->RxSize == 1)
	{
		*pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->I2C_DR;
		pI2CHandle->RxLen--;
	}
	if(pI2CHandle->RxSize > 1)
	{
		if(pI2CHandle->RxLen == 2)
		{
			//clear the ack bit
			pI2CHandle->pI2Cx->I2C_CR1 &= ~(1 << 10);
		}
		//read DR
		*(pI2CHandle->pRxBuffer) = pI2CHandle->pI2Cx->I2C_DR;
		pI2CHandle->pTxBuffer++;
		pI2CHandle->RxLen--;
	}
	if(pI2CHandle->RxLen == 0)
	{
		//close the I2C data reception and notify the application

		//1.generate the stop condition
		if(pI2CHandle->Sr == I2C_DISABLE_SR)
			I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

		//2.close the I2C rx
		I2C_CloseReceiveData(pI2CHandle);

		//3.Notify the application
		I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
	}
}


void I2C_SlaveSendData(I2C_RegDef_t *pI2C, uint8_t data)
{
	pI2C->I2C_DR = data;
}
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2C)
{
	return (uint8_t)pI2C->I2C_DR;
}

void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle)
{
	//Interrupt handling for both master and slave mode of a device
	uint8_t temp1, temp2, temp3;

	temp1 = pI2CHandle->pI2Cx->I2C_CR2 & (1 << I2C_CR2_ITEVTEN);
	temp2 = pI2CHandle->pI2Cx->I2C_CR2 & (1 << I2C_CR2_ITBUFEN);

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_SB);
	//1. Handle for interrupt generated by SB event
	// Note: SB flag is only applicable in Master mode
	if(temp1 && temp3)
	{
		//The interrupt is generated because of SB event
		//This block will not be executed in slave mode because for slave SB is always zero
		//In this block lets executed the address slave
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			I2C_ExcuteAddressPhaseWrite(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}
		else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			I2C_ExcuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}
	}
	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_ADDR);
	//2. Handle for interrupt generated by ADDR event
	// Note: When master mode: Address is sent
	//		 When Slave mode: Address matched with own address
	if(temp1 && temp3)
	{
		//ADDR flag is set
		I2C_ClearADDRFlag(pI2CHandle);

	}
	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_BTF);
	//3. Handle for interrupt generated by BTF(Byte Transfer Finished) event
	if(temp1 && temp3)
	{
		//BTF flag is set
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			//make sure that TXE is also set
			if(pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_TXE))
			{
				//BTF, TXE = 1 -> close the transmission
				if(pI2CHandle->TxLen == 0)
				{
					//1.generate the STOP condition
					if(pI2CHandle->Sr == I2C_DISABLE_SR)
						I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
					//2.reset all the members elements of the handle structure
					I2C_CloseSendData(pI2CHandle);
					//3.notify the application about transmission complete
					I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
				}
			}
		}
		else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{

		}
	}
	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_STOPF);
	//4. Handle for interrupt generated by STOPF event
	//Note: Stop detection flag is applicable only slave mode. For master this flag will be
	if(temp1 && temp3)
	{
		//STOPF flag is set
		//clear the STOPF (i.e 1) read SR1 2) Write to CR1 )

		pI2CHandle->pI2Cx->I2C_CR1 |= 0x0000;

		//Notify the application that STOP is detected
		I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOP);

	}
	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_TXE);
	//5.Handle for interrupt generated by TXE event
	if(temp1 && temp2 && temp3)
	{
		//check for device mode
		if(pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_MSL))
		{
			//TXE flag is set
			//Data transmission

			if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
			{
				I2C_MasterHandleTXEInterrupt(pI2CHandle);
			}
		}else
		{
			//slave
			//make sure that the slave is really in transmitter mode
			if(pI2CHandle->pI2Cx->I2C_SR2 & (1<< I2C_SR2_TRA))
			{
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
			}

		}
	}
	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_RXNE);
	//6.Handle for interrupt generated by RNXE event
	if(temp1 && temp2 && temp3)
	{
		//check for device mode
		if(pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_MSL))
		{
			//the device is master
			//RXNE flag is set
			//Data receive
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
			{
				I2C_MasterHandleRXNEInterrupt(pI2CHandle);
			}

		}else
		{
			//slave
			//make sure that the slave is really in transmitter mode
			if(!(pI2CHandle->pI2Cx->I2C_SR2 & (1<< I2C_SR2_TRA)))
			{
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);
			}
		}
	}
}


/*********************************************************************
 * @fn      		  - I2C_ER_IRQHandling
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the code also define these macros in the driver
						header file
						#define I2C_ERROR_BERR  3
						#define I2C_ERROR_ARLO  4
						#define I2C_ERROR_AF    5
						#define I2C_ERROR_OVR   6
						#define I2C_ERROR_TIMEOUT 7

 */

void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle)
{

	uint32_t temp1,temp2;

    //Know the status of  ITERREN control bit in the CR2
	temp2 = (pI2CHandle->pI2Cx->I2C_CR2) & ( 1 << I2C_CR2_ITERREN);


/***********************Check for Bus error************************************/
	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1<< I2C_SR1_BERR);
	if(temp1  && temp2 )
	{
		//This is Bus error

		//Implement the code to clear the buss error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_BERR);

		//Implement the code to notify the application about the error
	   I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_BERR);
	}

/***********************Check for arbitration lost error************************************/
	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1 << I2C_SR1_ARLO );
	if(temp1  && temp2)
	{
		//This is arbitration lost error

		//Implement the code to clear the arbitration lost error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_ARLO);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_ARLO);
	}

/***********************Check for ACK failure  error************************************/

	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1 << I2C_SR1_AF);
	if(temp1  && temp2)
	{
		//This is ACK failure error

	    //Implement the code to clear the ACK failure error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_AF);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_AF);
	}

/***********************Check for Overrun/underrun error************************************/
	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1 << I2C_SR1_OVR);
	if(temp1  && temp2)
	{
		//This is Overrun/underrun

	    //Implement the code to clear the Overrun/underrun error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_OVR);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_OVR);
	}

/***********************Check for Time out error************************************/
	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1 << I2C_SR1_TIMEOUT);
	if(temp1  && temp2)
	{
		//This is Time out error

	    //Implement the code to clear the Time out error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_TIMEOUT);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_TIMEOUT);
	}

}

void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle)
{
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~(1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVTEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~(1 << I2C_CR2_ITEVTEN);

	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pRxBuffer = NULL;
	pI2CHandle->RxLen = 0;
	pI2CHandle->RxSize = 0;
	if(pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE)
	{
		pI2CHandle->pI2Cx->I2C_CR1 |= (1 << 10);
	}

}
void I2C_CloseSendData(I2C_Handle_t *pI2CHandle)
{
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~(1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVTEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~(1 << I2C_CR2_ITEVTEN);

	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pTxBuffer = NULL;
	pI2CHandle->TxLen = 0;
}
