/*
 * stm32f407xx_gpio_driver.h
 *
 *  Created on: Aug 16, 2026
 *      Author: ADMIN
 */

#ifndef INC_STM32F407XX_GPIO_DRIVER_H_
#define INC_STM32F407XX_GPIO_DRIVER_H_

#include "stm32f407xx.h"


/*
 * This is a Configuration structure for a GPIO pin
 */
typedef struct
{
	uint8_t GPIO_PinNumber;
	uint8_t GPIO_PinMode;			/*|< possible values from @GPIO_PIN_MODES >*/
	uint8_t GPIO_PinSpeed;			/*|< possible values from @GPIO_PIN_SPEEDS >*/
	uint8_t GPIO_PinPuPdControl;	/*|< possible values from @GPIO_PIN_PUPD >*/
	uint8_t GPIO_PinOPType;			/*|< possible values from @GPIO_PIN_OP_TYPES >*/
	uint8_t GPIO_PinAltFunMode;
}GPIO_PinConfig_t;


/*
 * This is a Handle structure for a GPIO pin
 */

typedef struct
{
	GPIO_RegDef_t *pGPIOx;				/*|< This holds the base address of the GPIO port to which the pin belongs >*/
	GPIO_PinConfig_t GPIO_PinConfig;	/*|< This holds GPIO pin configuration setting >*/

}GPIO_Handle_t;

/*
 * @GPIO_PIN_NUMBERS
 * GPIO pin numbers
 */
#define GPIO_PIN_NO_0	0
#define GPIO_PIN_NO_1	1
#define GPIO_PIN_NO_2	2
#define GPIO_PIN_NO_3	3
#define GPIO_PIN_NO_4	4
#define GPIO_PIN_NO_5	5
#define GPIO_PIN_NO_6	6
#define GPIO_PIN_NO_7	7
#define GPIO_PIN_NO_8	8
#define GPIO_PIN_NO_9	9
#define GPIO_PIN_NO_10	10
#define GPIO_PIN_NO_11	11
#define GPIO_PIN_NO_12	12
#define GPIO_PIN_NO_13	13
#define GPIO_PIN_NO_14	14
#define GPIO_PIN_NO_15	15


/*
 * @GPIO_PIN_MODES
 * GPIO pin possible modes
 */
#define GPIO_MODE_IN		0
#define GPIO_MODE_OUT		1
#define GPIO_MODE_ALTFN		2
#define GPIO_MODE_ANALOG	3
#define GPIO_MODE_IT_FT		4
#define GPIO_MODE_IT_RT		5
#define GPIO_MODE_IT_RFT	6

/*
 * GPIO_PIN_OP_TYPES
 * GPIO pin possible output types
 */
#define GPIO_OP_TYPE_PP		0
#define GPIO_OP_TYPE_OD		1

/*
 * @GPIO_PIN_SPEEDS
 * GPIO pin possible output speeds
 */
#define GPIO_OP_SPEED_LOW		0
#define GPIO_OP_SPEED_MEDIUM	1
#define GPIO_OP_SPEED_FAST		2
#define GPIO_OP_SPEED_HIGH		3

/*
 * @GPIO_PIN_PUPD
 * GPIO pin pull up and pull down configuration macros
 */
#define GPIO_NO_PUPD		0
#define GPIO_PIN_PU			1
#define GPIO_PIN_PD			2

/*
 * GPIO Alternate Function Macros (từ AF0 đến AF15)
 */
#define GPIO_AF0    0
#define GPIO_AF1    1
#define GPIO_AF2    2
#define GPIO_AF3    3
#define GPIO_AF4    4
#define GPIO_AF5    5
#define GPIO_AF6    6
#define GPIO_AF7    7
#define GPIO_AF8    8
#define GPIO_AF9    9
#define GPIO_AF10   10
#define GPIO_AF11   11
#define GPIO_AF12   12
#define GPIO_AF13   13
#define GPIO_AF14   14
#define GPIO_AF15   15

/************
 * 							APIs supported by this driver
 * 				For more information about the APIs check the function definitions
 ***********/
void GPIO_PerClkControl(GPIO_RegDef_t *pGPIOx, uint8_t EnorDi); /* Peripheral Clock setup */

/*
 * Init and De-init
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx);

/*
 * Data read and write
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx);
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t Value);
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value);
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);

/*
 * IRQ Configuration and ISR handling
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void GPIO_IRQPriorityConfig(uint8_t IRQPriority, uint8_t IRQNumber);
void GPIO_IRQHandling(uint8_t PinNumber);


/*
 * Other Peripheral Control APIs
 */






#endif /* INC_STM32F407XX_GPIO_DRIVER_H_ */
