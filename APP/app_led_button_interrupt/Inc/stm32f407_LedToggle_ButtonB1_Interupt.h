/*
 * stm32f407_LedToggle_ButtonB1_Interupt.h
 *
 * Description:
 * Header file for the GPIO configuration used in the LED toggle
 * application with user button B1 on the STM32F407 microcontroller.
 *
 * This file declares the GPIO_ConfigAll() function, which configures
 * the GPIO pins for the LED output and button interrupt input.
 *
 * Hardware:
 * - LED:          PD12
 * - User Button:  PA0 (EXTI0 interrupt)
 *
 * @date    Oct 16, 2026
 * @author  Mohamed Ali, Lihiouel
 */

#ifndef STM32F407_LEDTOGGLE_BUTTONB1_INTERUPT_H_
#define STM32F407_LEDTOGGLE_BUTTONB1_INTERUPT_H_

#include "stm32F407.h"

#define STM32F407_LEDTOGGLE_BUTTONB1_INTERUPT STD_OFF

#if (STM32F407_LEDTOGGLE_BUTTONB1_INTERUPT == STD_ON)
/* Function prototype for configuring the required GPIO pins and interrupt. */
extern void GPIO_ConfigAll(void);
#endif

#endif /* STM32F407_LEDTOGGLE_BUTTONB1_INTERUPT_H_ */
