/*
 * File: stm32f407_LedToggle_ButtonB1_Interupt.c
 *
 * Description:
 * This file is associated with the GPIO configuration and interrupt
 * handling for toggling an LED connected to pin PD12 of the STM32F407
 * microcontroller when the user button B1, connected to pin PA0,
 * triggers an EXTI0 interrupt.
 *
 * The GPIO pins are configured using the STM32F407 GPIO driver.
 * The EXTI0 interrupt handler clears the pending interrupt and
 * toggles the LED state.
 *
 * Hardware:
 * - LED:        PD12
 * - User Button B1: PA0 (EXTI0)
 *
 * @date    Oct 16, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
 */

#include "stm32f407_gpio_driver.h"
#include <stm32f407_LedToggle_ButtonB1_Interupt.h>

#if(STM32F407_LEDTOGGLE_BUTTONB1_INTERUPT == STD_ON)
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

/*
 * Central GPIO configuration table.
 * Each entry describes the configuration of one GPIO pin.
 */
static GPIO_PinHndl_t gpioConfig[] =
{
    /* LED: PD12 */
    {
        .pGPIO_StartAddr = GPIOD,
        .GPIO_PinConfig =
        {
            .GPIO_PinNumber    = GPIO_PIN_12,
            .GPIO_PinMode      = GPIO_MODE_OUTPUT,
            .GPIO_PinSpeed     = GPIO_SPEED_MEDIUM,
            .GPIO_PinOPType    = GPIO_OTYPE_PP,
            .GPIO_PinPuPdRCtrl = GPIO_NO_PUPD
        }
    },

    /* User button B1: PA0, configured for falling-edge interrupt */
    {
        .pGPIO_StartAddr = GPIOA,
        .GPIO_PinConfig =
        {
            .GPIO_PinNumber    = GPIO_PIN_0,
            .GPIO_PinMode      = GPIO_MODE_IT_FT,
            .GPIO_PinSpeed     = GPIO_SPEED_MEDIUM,
            .GPIO_PinOPType    = GPIO_OTYPE_PP,
            .GPIO_PinPuPdRCtrl = GPIO_NO_PUPD
        }
    }
};


/**
 * @brief  Configure all GPIO pins defined in the configuration table.
 * @param  None
 * @retval None
 */
void GPIO_ConfigAll(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(gpioConfig); i++)
    {
        /* Enable the peripheral clock for the selected GPIO port. */
        GPIO_PeriClkCtrl(gpioConfig[i].pGPIO_StartAddr, ENABLE);

        /* Initialize the GPIO pin with its configured settings. */
        GPIO_Init(&gpioConfig[i]);
    }

    /* Configure the EXTI0 interrupt priority and enable the interrupt. */
    GPIO_IRQPriorityConfig(IRQ_NO_EXTI0, 10);
    GPIO_IRQInterruptConfig(IRQ_NO_EXTI0, ENABLE);
}

/**
 * @brief  EXTI0 interrupt handler: Clear the pending interrupt and toggle the LED on PD12.
 * @param  None
 * @retval None
 */
void EXTI0_IRQHandler(void)
{
    /* Handle and clear the pending EXTI0 interrupt. */
    GPIO_IRQHandling(GPIO_PIN_0);

    /* Toggle the LED connected to PD12. */
    GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_12);
}
#endif
