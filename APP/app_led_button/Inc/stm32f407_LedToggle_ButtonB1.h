/*
 * stm32f407_LedToggle_ButtonB1.h
 *
 *  Created on: Oct 16, 2026
 *      Author: Moham
 */

#ifndef STM32F407_LEDTOGGLE_BUTTONB1_H_
#define STM32F407_LEDTOGGLE_BUTTONB1_H_

#include "stm32F407.h"

#define STM32F407_LEDTOGGLE_BUTTONB1 STD_ON

#if(STM32F407_LEDTOGGLE_BUTTONB1 == STD_ON)
/* Function prototype for configuring the required GPIO pins and interrupt. */
extern void GPIO_ConfigAll(void);
/*Function prototype to toggle the LED when the input is HIGH */
extern void Button_CheckAndToggleLED(void);
#endif


#endif /* STM32F407_LEDTOGGLE_BUTTONB1_H_ */
