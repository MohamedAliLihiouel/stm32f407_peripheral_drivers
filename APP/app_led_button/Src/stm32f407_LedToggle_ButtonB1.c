/**
 * File:    stm32f407_LedToggle_ButtonB1.c
 * @brief   GPIO configuration and LED toggle control using button B1.
 *
 * @details Configures the user button B1 on PA0 as an input and the LED
 *          on PD12 as an output. The GPIO configuration is stored in a
 *          table and applied by GPIO_ConfigAll().
 *
 *          Button_CheckAndToggleLED() reads the button input. When the
 *          input is HIGH, a software delay is executed and the LED output
 *          is toggled.
 *
 * @date    Oct 16, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
 */

#include "stm32f407_gpio_driver.h"
#include <stm32f407_LedToggle_ButtonB1.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#if(STM32F407_LEDTOGGLE_BUTTONB1 == STD_ON)
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

    /* Button: PA0 */
    {
        .pGPIO_StartAddr = GPIOA,
        .GPIO_PinConfig =
        {
            .GPIO_PinNumber    = GPIO_PIN_0,
            .GPIO_PinMode      = GPIO_MODE_INPUT,
            .GPIO_PinSpeed     = GPIO_SPEED_MEDIUM,
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
        GPIO_PeriClkCtrl(gpioConfig[i].pGPIO_StartAddr, ENABLE);
        GPIO_Init(&gpioConfig[i]);
    }
}

/*
 * @brief Check button B1 and toggle the LED when the input is HIGH.
 * @param  None
 * @return None
 * @warning If called repeatedly while the button remains HIGH, the LED
 *          may toggle repeatedly. A software delay alone does not provide
 *          reliable button debouncing or one-toggle-per-press behavior.
 */
void Button_CheckAndToggleLED(void)
{
    if (GPIO_ReadInputPin(GPIOA, GPIO_PIN_0) == HIGH)
    {
        for (uint32_t i = 0; i < 500000 / 2; i++);

        GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_12);
    }
}
#endif
