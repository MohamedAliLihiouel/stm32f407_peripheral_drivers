/**
 * @file    stm32f407_RCC_driver.h
 * @brief   Public interface of the STM32F407 RCC (clock) driver.
 *
 * @details
 * Provides the peripheral bus clock frequencies (PCLK1, PCLK2). They are
 * needed by the I2C driver (CR2.FREQ, CCR, TRISE) and by the USART driver
 * (baud rate register BRR).
 *
 * @date    Nov 23, 2026
 * @author  Mohamed Ali, Lihiouel
 */

#ifndef STM32F407_RCC_DRIVER_H_
#define STM32F407_RCC_DRIVER_H_

#include "stm32f407.h"

/**
 * @defgroup RCC_Driver_APIs RCC Driver APIs
 * @{
 */
extern uint32_t RCC_GetPCLK1Value(void);   /*!< APB1 clock in Hz (I2C, USART2/3, SPI2/3) */
extern uint32_t RCC_GetPCLK2Value(void);   /*!< APB2 clock in Hz (USART1/6, SPI1/4)      */
/** @} */

#endif /* STM32F407_RCC_DRIVER_H_ */
