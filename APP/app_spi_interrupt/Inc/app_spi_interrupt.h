/*
- File: app_spi_interrupt.h
 *
- Description:
- This header file contains the configuration macros and public function
- prototypes for the interrupt-driven SPI application on the STM32F407
- microcontroller.
 *
- It declares interfaces to initialize the required GPIO pins and SPI2
- peripheral, execute interrupt-based master data transmission and reception,
- and handle asynchronous SPI application event callbacks.
 *
- @date    Nov 19, 2026
- @author  Mohamed Ali, Lihiouel
 */

#ifndef APP_SPI_INTERRUPT_H_
#define APP_SPI_INTERRUPT_H_

#include "stm32f407_SPI_driver.h"

/**
- @brief  Feature switch to enable or disable the interrupt-driven SPI application.
 */
#define APP_SPI_INTERRUPT STD_ON

#if (APP_SPI_INTERRUPT == STD_ON)

/**
- @brief  Configures and initializes all GPIO pins required for SPI2,
-         the user push-button, and the status LED.
- @param  None
- @retval None
 */
extern void SPI_Int_ConfigGPIOPins(void);

/**
- @brief  Configures and initializes the SPI2 peripheral in master mode
-         along with its NVIC interrupt line.
- @param  None
- @retval None
 */
extern void SPI_Int_ConfigAll(void);

/**
- @brief  Waits for a button press and transmits a test string over SPI2
-         in interrupt mode while simultaneously receiving incoming bytes.
- @param  None
- @retval None
 */
extern void SPI_Int_MasterSendData(void);

/**
- @brief  Application callback invoked by the SPI driver upon asynchronous
-         interrupt events (e.g., reception complete, overrun error).
- @param  pSPIHandle: Pointer to the SPI handle structure that triggered the event.
- @param  AppEv:      SPI application event identifier.
- @retval None
 */
extern void SPI_ApplEventCallback(SPI_Hndl_t *pSPIHandle, uint8_t AppEv);

#endif /* APP_SPI_INTERRUPT == STD_ON */

#endif /* APP_SPI_INTERRUPT_H_ */