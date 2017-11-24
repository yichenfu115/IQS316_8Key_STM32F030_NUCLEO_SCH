/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HWCFG_H
#define __HWCFG_H

#ifdef __cplusplus
extern "C" {
#endif

	/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

	/** @addtogroup WDL_HW_LOW_LEVEL
	  * @{
	  */
#define LED3_Pin GPIO_PIN_5
#define LED3_GPIO_Port GPIOA
#define SPI1_SS_Pin GPIO_PIN_8
#define SPI1_SS_GPIO_Port GPIOA
#define MCLR_Pin GPIO_PIN_10
#define MCLR_GPIO_Port GPIOA
#define RDY_Pin GPIO_PIN_8
#define RDY_GPIO_Port GPIOB
#define TOUT_Pin GPIO_PIN_12
#define TOUT_GPIO_Port GPIOC
#define POUT_Pin GPIO_PIN_10
#define POUT_GPIO_Port GPIOC

#define MCLR_LOW()                       HAL_GPIO_WritePin(MCLR_GPIO_Port, MCLR_Pin, GPIO_PIN_RESET)
#define MCLR_HIGH()                      HAL_GPIO_WritePin(MCLR_GPIO_Port, MCLR_Pin, GPIO_PIN_SET)
#define SPI1_SS_LOW()                    HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_RESET)
#define SPI1_SS_HIGH()                   HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_SET)
#define RRDY()                           HAL_GPIO_ReadPin(RDY_GPIO_Port, RDY_Pin)
#define RTOUT()                          HAL_GPIO_ReadPin(TOUT_GPIO_Port, TOUT_Pin)
#define RPOUT()                          HAL_GPIO_ReadPin(POUT_GPIO_Port, POUT_Pin)



	/**
	  * @}
	  */
	void SystemClock_Config(void);
	void MX_GPIO_Init(void);
	void MX_I2C1_Init(void);
	void MX_SPI1_Init(void);
	void MX_USART2_UART_Init(void);
#ifdef __GNUC__
	/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
	   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


	/**
	  * @}
	  */

#endif

