/*
 * File: stm32f407_gpio_driver.c
 *
 * Description:
 * This file contains the implementation of the GPIO driver for the
 * STM32F407 microcontroller.
 *
 * It provides functions to initialize and de-initialize GPIO pins,
 * configure GPIO peripheral clocks, read input pins and ports,
 * write output pins and ports, and toggle output pins.
 *
 * The driver also supports external interrupt configuration through
 * the EXTI peripheral and interrupt management through the NVIC,
 * including interrupt enabling, disabling, priority configuration,
 * and pending interrupt flag handling.
 *
 * @date    Oct 16, 2026
 * @author  Mohamed Ali, Lihiouel
 */

#include "stm32f407_gpio_driver.h"
#include "nvic_config.h"

/**
 * @brief   * Configures the selected GPIO pin according to the settings
 * provided in the GPIO_PinHndl_t configuration structure.
 *
 * Supported configurations include:
 * - GPIO operating mode
 * - External interrupt trigger selection
 * - GPIO port-to-EXTI line mapping
 * - Output speed
 * - Pull-up and pull-down resistors
 * - Output type
 * - Alternate-function selection
 * @param  pGPIOHandle: Pointer to a GPIO pin handle structure (`GPIO_PinHndl_t`)
 *                      containing the base address and user configuration settings.
 * @retval None
 */
void GPIO_Init(GPIO_PinHndl_t *pGPIOHandle)
{
	uint32_t tempReg=0;
    uint32_t pin = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber;

	// 1) configure the mode of GPIO pin
	if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG)
	{
		// non interrupt mode
		// Reset: Clear the 2 bits corresponding to this pin.
		pGPIOHandle->pGPIO_StartAddr->MODER &= ~( 0x3 << (2 * pin));
		tempReg = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode << (2 * pin));
		// Set: Write the requested mode into the cleared field.
		pGPIOHandle->pGPIO_StartAddr->MODER |= tempReg;
	}
	else
	{
		// interrupt mode
        // Configure the GPIO pin as an input and configure the EXTI
        // peripheral to detect the selected signal transition.
		if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT)
		{
			// 1) Configure the EXTI Falling Trigger Selection Register (FTSR).
			//    Generate an interrupt when the input signal changes
			//    from HIGH to LOW (falling edge).
			EXTI->FTSR |= (1 << pin);
			// clear the corresponding RTSR bit
			EXTI->RTSR &= ~(1 << pin);

		}
		else if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT)
		{
			// 1) Configure the EXTI Rising Trigger Selection Register (RTSR).
			//    Generate an interrupt when the input signal changes
			//    from LOW to HIGH (rising edge).
			EXTI->RTSR |= (1 << pin);
			// clear the corresponding FTSR bit
			EXTI->FTSR &= ~(1 << pin);
		}
		else if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RFT)
		{
			// 1) Configure both FTSR and RTSR.
			//    Generate an interrupt on both falling and rising edges.
			EXTI->FTSR |= (1 << pin);
			EXTI->RTSR |= (1 << pin);
		}

		// 2) Configure the GPIO port selection in SYSCFG_EXTICR.
		//    Connect the selected GPIO port and pin to the corresponding
		//    EXTI line (for example, PA0 -> EXTI0).
		uint8_t tempReg = (uint8_t)(pin / 4);
		uint8_t tempBit = (uint8_t)(pin % 4);
		uint8_t portNumber = GPIO_STR_ADDR_TO_PORT(pGPIOHandle->pGPIO_StartAddr);

		RCC_SYSCFG_CLK_ENABLE();
		SYSCFG->EXTICR[tempReg] = (uint32_t)(portNumber << (4 * tempBit));

        // 3) Enable interrupt delivery for the selected EXTI line using IMR.
        //    The corresponding NVIC interrupt must also be enabled separately.
		EXTI->IMR |= (1 << pin);
	}
	tempReg=0;

	// 2) configure the speed
	// Reset: Clear the selected pin's 2-bit field.
	pGPIOHandle->pGPIO_StartAddr->OSPEEDR &= ~(0x3 << (2 * pin));
	tempReg = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed << (2 * pin));
	// Set: Write the requested output speed.
	pGPIOHandle->pGPIO_StartAddr->OSPEEDR |= tempReg;
	tempReg = 0;

	// 3) configure the PuPd(Pull up/Pull down) settings
	// Reset: Clear the selected pin's 2-bit field.
	pGPIOHandle->pGPIO_StartAddr->PUPDR &= ~(0x3 << (2 * pin));
	tempReg =(pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdRCtrl << (2 * pin));
	// Set: Write the requested pull-up/pull-down configuration.
	pGPIOHandle->pGPIO_StartAddr->PUPDR |= tempReg;
	tempReg = 0;

	// 4) configure the optyp(output type)
	// Reset: Clear the selected pin's bit.
	pGPIOHandle->pGPIO_StartAddr->OTYPER &= ~(0x1 << pin);
	tempReg = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType << pin);
	// Set: Write the requested output type.
	pGPIOHandle->pGPIO_StartAddr->OTYPER |= tempReg;
	tempReg = 0;

	// 5) configure the alternate functionality
	if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFUNC)
	{
		if(pin < 8)
		{
			// AFRL: Pins 0-7.
			// Reset: Clear this pin's 4-bit alternate-function field.
			pGPIOHandle->pGPIO_StartAddr->AFRL &= ~(0xF << (4 * pin));
			tempReg = (pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFuncMode << (4 * pin));
			// Set: Write the requested alternate-function value.
			pGPIOHandle->pGPIO_StartAddr->AFRL |= tempReg;
		}
		else
		{
			// AFRH: Pins 8-15.
			// Reset: Clear this pin's 4-bit alternate-function field.
			pGPIOHandle->pGPIO_StartAddr->AFRH &= ~(0xF << (4 * (pin % 8)));
			tempReg = (pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFuncMode << (4 * (pin % 8)));
			// Set: Write the requested alternate-function value.
			pGPIOHandle->pGPIO_StartAddr->AFRH |= tempReg;
		}
	}
}

/**
 * @brief  De-initializes the specified GPIO port, resetting all its registers to their default state.
 * @param  pGPIOx: Pointer to the base address of the target GPIO port (e.g., GPIOA, GPIOB).
 * @retval None
 */
void GPIO_DeInit(GPIO_Registers_t *pGPIOx)
{
	switch ((uint32_t)pGPIOx)
	{
		case (uint32_t)GPIOA:
			RCC_GPIOA_PORT_RESET();
			break;
		case (uint32_t)GPIOB:
			RCC_GPIOB_PORT_RESET();
			break;
		case (uint32_t)GPIOC:
			RCC_GPIOC_PORT_RESET();
			break;
		case (uint32_t)GPIOD:
			RCC_GPIOD_PORT_RESET();
			break;
		case (uint32_t)GPIOE:
			RCC_GPIOE_PORT_RESET();
			break;
		case (uint32_t)GPIOF:
			RCC_GPIOF_PORT_RESET();
			break;
		case (uint32_t)GPIOG:
			RCC_GPIOG_PORT_RESET();
			break;
		case (uint32_t)GPIOH:
			RCC_GPIOH_PORT_RESET();
			break;
		case (uint32_t)GPIOI:
			RCC_GPIOI_PORT_RESET();
			break;
		case (uint32_t)GPIOJ:
			RCC_GPIOJ_PORT_RESET();
			break;
		case (uint32_t)GPIOK:
			RCC_GPIOK_PORT_RESET();
			break;
		default:
			break;
	}
}

/**
 * @brief  Enables or disables the peripheral clock for the given GPIO port.
 * @param  pGPIOx: Pointer to the base address of the target GPIO port.
 * @param  EnOrDi: Macro for enabling or disabling the clock (e.g., ENABLE or DISABLE).
 * @retval None
 */
void GPIO_PeriClkCtrl(GPIO_Registers_t *pGPIOx, uint8_t EnOrDi)
{
	if (EnOrDi == 1) // Enable
	{
		switch ((uint32_t)pGPIOx)
		{
			case (uint32_t)GPIOA:
				RCC_GPIOA_CLK_ENABLE();
				break;
			case (uint32_t)GPIOB:
				RCC_GPIOB_CLK_ENABLE();
				break;
			case (uint32_t)GPIOC:
				RCC_GPIOC_CLK_ENABLE();
				break;
			case (uint32_t)GPIOD:
				RCC_GPIOD_CLK_ENABLE();
				break;
			case (uint32_t)GPIOE:
				RCC_GPIOE_CLK_ENABLE();
				break;
			case (uint32_t)GPIOF:
				RCC_GPIOF_CLK_ENABLE();
				break;
			case (uint32_t)GPIOG:
				RCC_GPIOG_CLK_ENABLE();
				break;
			case (uint32_t)GPIOH:
				RCC_GPIOH_CLK_ENABLE();
				break;
			case (uint32_t)GPIOI:
				RCC_GPIOI_CLK_ENABLE();
				break;
			case (uint32_t)GPIOJ:
				RCC_GPIOJ_CLK_ENABLE();
				break;
			case (uint32_t)GPIOK:
				RCC_GPIOK_CLK_ENABLE();
				break;
			default:
				break;
		}
	}
	else
	{
		switch ((uint32_t)pGPIOx)
		{
			case (uint32_t)GPIOA:
				RCC_GPIOA_CLK_DISABLE();
				break;
			case (uint32_t)GPIOB:
				RCC_GPIOB_CLK_DISABLE();
				break;
			case (uint32_t)GPIOC:
				RCC_GPIOC_CLK_DISABLE();
				break;
			case (uint32_t)GPIOD:
				RCC_GPIOD_CLK_DISABLE();
				break;
			case (uint32_t)GPIOE:
				RCC_GPIOE_CLK_DISABLE();
				break;
			case (uint32_t)GPIOF:
				RCC_GPIOF_CLK_DISABLE();
				break;
			case (uint32_t)GPIOG:
				RCC_GPIOG_CLK_DISABLE();
				break;
			case (uint32_t)GPIOH:
				RCC_GPIOH_CLK_DISABLE();
				break;
			case (uint32_t)GPIOI:
				RCC_GPIOI_CLK_DISABLE();
				break;
			case (uint32_t)GPIOJ:
				RCC_GPIOJ_CLK_DISABLE();
				break;
			case (uint32_t)GPIOK:
				RCC_GPIOK_CLK_DISABLE();
				break;
			default:
				break;
		}
	}
}

/**
 * @brief  Reads the digital input value of a specific GPIO pin.
 * @param  pGPIOx: Pointer to the base address of the target GPIO port.
 * @param  PinNumber: The specific pin number to read from.
 * @retval The pin input value (0 or 1).
 */
uint8_t GPIO_ReadInputPin(GPIO_Registers_t *pGPIOx, uint16_t PinNumber)
{
	return (uint8_t)((pGPIOx->IDR >> PinNumber) & 0x1);
}

/**
 * @brief  Reads the entire 16-bit input data register value of a GPIO port.
 * @param  pGPIOx: Pointer to the base address of the target GPIO port.
 * @retval 16-bit value representing the state of all port input pins.
 */
uint16_t GPIO_ReadInputPort(GPIO_Registers_t *pGPIOx)
{
	return (uint16_t)(pGPIOx->IDR & 0xFFFFU);
}

/**
 * @brief  Sets or resets a specific output pin on a GPIO port.
 * @param  pGPIOx: Pointer to the base address of the target GPIO port.
 * @param  PinNumber: The specific pin number to write to.
 * @param  Value: Output value to write (e.g., GPIO_PIN_SET or GPIO_PIN_RESET).
 * @retval None
 */
void GPIO_WriteOutputPin(GPIO_Registers_t *pGPIOx, uint16_t PinNumber, uint8_t Value)
{
	if (Value == GPIO_PIN_SET)
	{
	    pGPIOx->ODR |= (1U << PinNumber);
	}
	else
	{
	    pGPIOx->ODR &= ~(1U << PinNumber);
	}
}

/**
 * @brief  Writes a 16-bit value to the entire output data register of a GPIO port.
 * @param  pGPIOx: Pointer to the base address of the target GPIO port.
 * @param  Value: 16-bit value to write to the output port.
 * @retval None
 */
void GPIO_WriteOutputPort(GPIO_Registers_t *pGPIOx, uint16_t Value)
{
	pGPIOx->ODR = (uint32_t)Value;
}

/**
 * @brief  Toggles the current output state (high to low, or low to high) of a specific GPIO pin.
 * @param  pGPIOx: Pointer to the base address of the target GPIO port.
 * @param  PinNumber: The specific pin number to toggle.
 * @retval None
 */
void GPIO_ToggleOutputPin(GPIO_Registers_t *pGPIOx, uint16_t PinNumber)
{
	pGPIOx->ODR ^= (1 << PinNumber);
}

/**
 * @brief  Configures the interrupt parameters (IRQ number and activation) for external interrupts in the NVIC.
 * @param  IRQNumber: The Interrupt Request number in the NVIC.
 * @param  EnOrDi: Macro to enable or disable the interrupt configuration.
 * @retval None
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(IRQNumber<32)
		{
			// NVIC_ISER0
			*NVIC_ISER0 |= (1 << IRQNumber);
		}
		else if(IRQNumber>=32 && (IRQNumber<64))
		{
			// NVIC_ISER1
			*NVIC_ISER1 |= (1 << (IRQNumber%32));
		}
		else if(IRQNumber>=64 && (IRQNumber<96))
		{
			// NVIC_ISER2
			*NVIC_ISER2 |= (1 << (IRQNumber%64));
		}
	}
	else if(EnOrDi == DISABLE)
	{
		if(IRQNumber<32)
		{
			// NVIC_ICER0
			*NVIC_ICER0 |= (1 << IRQNumber);
		}
		else if(IRQNumber>=32 && (IRQNumber<64))
		{
			// NVIC_ICER1
			*NVIC_ICER1 |= (1 << (IRQNumber%32));
		}
		else if(IRQNumber>=65 && (IRQNumber<96))
		{
			// NVIC_ICER2
			*NVIC_ICER2 |= (1 << (IRQNumber%64));
		}
	}
}

/**
 * @brief  Configures the priority of an external interrupt in the NVIC.
 * @param  IRQNumber: The Interrupt Request number in the NVIC.
 * @param  IRQPriority: Priority configuration for the interrupt.
 * @retval None
 */
void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority)
{
    /* Calculate the index of the 32-bit IPR register.
     * Each register contains four 8-bit interrupt priority fields. */
    uint8_t tempIprx = IRQNumber / 4U;

    /* Identify the byte field corresponding to this interrupt (0-3). */
    uint8_t tempIprxField = IRQNumber % 4U;

    /* Calculate the bit position of the implemented priority bits.
     * Each field occupies 8 bits; the implemented bits are left-aligned. */
    uint8_t tempShiftImplDefinedPrio = (uint8_t)((8U * tempIprxField) + NUM_PR_BITS_NOT_IMPLEMENTED);

    /* Access the corresponding 32-bit NVIC IPR register. */
    volatile uint32_t *pIpr = (volatile uint32_t *)CORTEX_M_NVIC_IPR_START_ADDRESS;


    /* Clear this interrupt's 4-bit priority field. */
    pIpr[tempIprx] &= ~(0xF << tempShiftImplDefinedPrio);
    /* Write the priority value into the corresponding field. */
    pIpr[tempIprx] |= ((uint32_t)IRQPriority << tempShiftImplDefinedPrio);
}

/**
 * @brief  Handles incoming interrupts for a specific pin by clearing the corresponding pending flag.
 * @param  PinNumber: The pin number that triggered the interrupt.
 * @retval None
 */
void GPIO_IRQHandling(uint8_t PinNumber)
{
    /* Check whether the pin's EXTI pending flag is set. */
    if(EXTI->PR & (1 << PinNumber))
    {
        /* Clear the pending flag by writing 1 to the corresponding bit. */
        EXTI->PR = (1 << PinNumber);
    }
}
