/*
 * File: app_usart_it_tx_rx.c
 *
 * Description:
 * This file contains the USART test application for the STM32F407 Discovery
 * board. USART2 and USART3 are used on the same board in interrupt mode.
 * When the user button is pressed, USART2 sends a text to USART3. USART3
 * reverses the received text and sends it back to USART2.
 *
 * @date    Nov 24, 2026
 * @author  Mohamed Ali, Lihiouel
 */

#include <stdint.h>
#include <string.h>
#include "app_usart_it_tx_rx.h"
#include "stm32f407_GPIO_driver.h"
#include "stm32f407_USART_driver.h"

#if(APP_USART_IT_TX_RX == STD_ON)

/******************************************************************************
 * Private macros and variables
 ******************************************************************************/

/* Number of elements of a C array. */
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

/* Index of the peripherals in usartConfig[]. */
#define USART2_IDX   (0U)
#define USART3_IDX   (1U)

/* Buffers (the text must be shorter than the buffer size). */
static uint8_t usartRxBuffer[32];        /* USART3: text received from USART2          */
static uint8_t usartReverseBuffer[32];   /* USART3: reversed text, sent to USART2      */
static uint8_t usartEchoBuffer[32];      /* USART2: reversed text received from USART3 */

/* Flags set in USART_ApplicationEventCallback() (interrupt context). */
static volatile uint8_t usartTx2Done = 0U;   /* 1 = USART2 has sent all bytes     */
static volatile uint8_t usartRx3Done = 0U;   /* 1 = USART3 has received all bytes */
static volatile uint8_t usartTx3Done = 0U;   /* 1 = USART3 has sent all bytes     */
static volatile uint8_t usartRx2Done = 0U;   /* 1 = USART2 has received all bytes */
static volatile uint8_t usartError   = 0U;   /* 0 = no error, otherwise USART_ERR_xxx */

/* Result (check in the debugger): 1 = USART2 received the text in reverse order. */
static volatile uint8_t usartOk = 0U;

/******************************************************************************
 * USART test application: USART2 <-> USART3 on the same board (interrupt mode)
 * Wiring: PA2 (USART2_TX) -> PD9 (USART3_RX)
 *         PD8 (USART3_TX) -> PA3 (USART2_RX)
 ******************************************************************************/

/**
 * @brief GPIO configuration table of the USART test application.
 *
 * @details
 * Pins used by USART2 and USART3 (STM32F407 Discovery):
 * - PA0 : User push-button B1 (input, starts the transfer)
 * - PA2 : AF7 USART2_TX
 * - PA3 : AF7 USART2_RX
 * - PD8 : AF7 USART3_TX
 * - PD9 : AF7 USART3_RX
 *
 * @note The idle level of a UART line is HIGH. TX is driven push-pull (PP),
 *       so no pull resistor is needed. RX gets the internal pull-up
 *       (about 40 kOhm), so the line does not float while nothing is connected.
 */
static GPIO_PinHndl_t gpioConfig[] =
{
	/* PA0: User Push-Button B1 (Trigger for the USART transmission) */
	{
		.pGPIO_StartAddr = GPIOA,
		.GPIO_PinConfig =
		{
			.GPIO_PinNumber    = GPIO_PIN_0,
			.GPIO_PinMode      = GPIO_MODE_INPUT,
			.GPIO_PinSpeed     = GPIO_SPEED_HIGH,
			.GPIO_PinPuPdRCtrl = GPIO_NO_PUPD
		}
	},
	/* PA2 : USART2 Tx (AF7) */
	{
		.pGPIO_StartAddr = GPIOA,
		.GPIO_PinConfig = {
			.GPIO_PinNumber      = GPIO_PIN_2,
			.GPIO_PinMode        = GPIO_MODE_ALTFUNC,   /* alternate function     */
			.GPIO_PinSpeed       = GPIO_SPEED_MEDIUM,   /* enough for 115200 baud */
			.GPIO_PinOPType      = GPIO_OTYPE_PP,       /* push-pull              */
			.GPIO_PinPuPdRCtrl   = GPIO_NO_PUPD,        /* no pull resistor       */
			.GPIO_PinAltFuncMode = 7                    /* AF7 = USART1..USART3   */
		}
	},

	/* PA3 : USART2 Rx (AF7) */
	{
		.pGPIO_StartAddr = GPIOA,
		.GPIO_PinConfig = {
			.GPIO_PinNumber      = GPIO_PIN_3,
			.GPIO_PinMode        = GPIO_MODE_ALTFUNC,
			.GPIO_PinSpeed       = GPIO_SPEED_MEDIUM,
			.GPIO_PinOPType      = GPIO_OTYPE_PP,
			.GPIO_PinPuPdRCtrl   = GPIO_PIN_PU,         /* idle level HIGH        */
			.GPIO_PinAltFuncMode = 7
		}
	},

	/* PD8 : USART3 Tx (AF7) */
	{
		.pGPIO_StartAddr = GPIOD,
		.GPIO_PinConfig = {
			.GPIO_PinNumber      = GPIO_PIN_8,
			.GPIO_PinMode        = GPIO_MODE_ALTFUNC,
			.GPIO_PinSpeed       = GPIO_SPEED_MEDIUM,
			.GPIO_PinOPType      = GPIO_OTYPE_PP,
			.GPIO_PinPuPdRCtrl   = GPIO_NO_PUPD,
			.GPIO_PinAltFuncMode = 7
		}
	},

	/* PD9 : USART3 Rx (AF7) */
	{
		.pGPIO_StartAddr = GPIOD,
		.GPIO_PinConfig = {
			.GPIO_PinNumber      = GPIO_PIN_9,
			.GPIO_PinMode        = GPIO_MODE_ALTFUNC,
			.GPIO_PinSpeed       = GPIO_SPEED_MEDIUM,
			.GPIO_PinOPType      = GPIO_OTYPE_PP,
			.GPIO_PinPuPdRCtrl   = GPIO_PIN_PU,         /* idle level HIGH        */
			.GPIO_PinAltFuncMode = 7
		}
	}
};

/**
 * @brief USART peripheral configuration table of the test application.
 *
 * @details
 * One array element per peripheral (both 115200 baud, 8 data bits, no parity,
 * 1 stop bit, transmitter and receiver enabled):
 * - [0] = USART2 (USART2_IDX)
 * - [1] = USART3 (USART3_IDX)
 */
static USART_Hndl_t usartConfig[] =
{
	/* [0] USART2: TX = PA2, RX = PA3 */
	{
		.pUSARTx_StartAddr = USART2,
		.USART_Config =
		{
			.USART_Mode          = USART_MODE_TXRX,          /* transmitter and receiver  */
			.USART_BaudRate      = USART_STD_BAUD_115200,    /* 115200 bit/s              */
			.USART_NoOfStopBits  = USART_STOPBITS_1,         /* 1 stop bit                */
			.USART_WordLength    = USART_WORDLEN_8BITS,      /* 8 data bits               */
			.USART_ParityControl = USART_PARITY_DISABLE,     /* no parity (8N1)           */
			.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE   /* no CTS / RTS              */
		}
	},
	/* [1] USART3: TX = PD8, RX = PD9 */
	{
		.pUSARTx_StartAddr = USART3,
		.USART_Config =
		{
			.USART_Mode          = USART_MODE_TXRX,
			.USART_BaudRate      = USART_STD_BAUD_115200,
			.USART_NoOfStopBits  = USART_STOPBITS_1,
			.USART_WordLength    = USART_WORDLEN_8BITS,
			.USART_ParityControl = USART_PARITY_DISABLE,
			.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE
		}
	}
};

/**
 * @brief  Generates a blocking software delay for button debouncing.
 * @param  None
 * @retval None
 */
static void delay(void)
{
	for(uint32_t i = 0 ; i < 500000 ; i ++);
}

/**
 * @brief  Configure all GPIO pins defined in the configuration table.
 *
 * @details Enables the clock for each GPIO port referenced by the
 *          configuration table and initializes the corresponding pins.
 *
 * @param  None
 * @retval None
 */
void USART_ConfigGPIOPins(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(gpioConfig); i++)
    {
        GPIO_PeriClkCtrl(gpioConfig[i].pGPIO_StartAddr, ENABLE);
        GPIO_Init(&gpioConfig[i]);
    }
}

/**
 * @brief  Configure and initialize all USART peripherals in the configuration table.
 *
 * @details Enables the peripheral clock for each configured USART instance
 *          (USART2, USART3) and applies its settings using USART_Init().
 *          Then the interrupt lines of USART2 and USART3 are enabled in the NVIC.
 *
 * @param  None
 * @retval None
 *
 * @note   The USARTs are not enabled here (CR1.UE = 0). They are enabled in
 *         USART_InterruptTx_Rx() with USART_PeripheralCtrl().
 */
void USART_ConfigAll(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(usartConfig); i++)
    {
    	// 1) Enable the peripheral clock, then 2) write the configuration registers.
    	USART_PeriClkCtrl(usartConfig[i].pUSARTx_StartAddr, ENABLE);
    	USART_Init(&usartConfig[i]);
    }
    // 3) Enable the USART2 and USART3 interrupt request line in the NVIC.
    USART_IRQInterruptConfig(IRQ_NO_USART2, ENABLE);
    USART_IRQInterruptConfig(IRQ_NO_USART3, ENABLE);
}

/**
 * @brief  Send a text from USART2 to USART3 and echo it reversed to USART2 (interrupt mode).
 *
 * @details Sequence:
 *          1) Wait for the user button (PA0) and debounce it.
 *          2) Enable both peripherals (UE = 1) and reset the flags.
 *          3) Phase 1: USART3 receives, USART2 sends the text.
 *          4) USART3 reverses the received text.
 *          5) Phase 2: USART2 receives, USART3 sends the reversed text.
 *          6) Compare the echo with the original text in reverse order (usartOk).
 *          7) Disable both peripherals (UE = 0), then their clocks.
 *
 * @param  None
 * @retval None
 *
 * @note   The receiver is always started BEFORE the transmitter, otherwise the
 *         first bytes can be lost.
 * @note   Check in the debugger: usartOk == 1, usartError == 0,
 *         usartRxBuffer == "hello I am USART2",
 *         usartEchoBuffer == "2TRASU ma I olleh".
 */
void USART_InterruptTx_Rx(void)
{
	char user_data[] = "hello I am USART2 IT";
	uint32_t len = (uint32_t)strlen(user_data);

	USART_Registers_t *pUsart2 = usartConfig[USART2_IDX].pUSARTx_StartAddr;
	USART_Registers_t *pUsart3 = usartConfig[USART3_IDX].pUSARTx_StartAddr;

	/* 1. Wait until the user push-button (PA0) is pressed. */
	while (! GPIO_ReadInputPin(GPIOA, GPIO_PIN_0) )
	{
		/* wait */
	}
	// Apply software delay (~200 ms) to avoid mechanical button bouncing.
	delay();

	/* 2. Enable the peripherals (UE must be set AFTER USART_Init()) and reset the flags. */
	USART_PeriClkCtrl(pUsart2, ENABLE);
	USART_PeripheralCtrl(pUsart2, ENABLE);
	USART_PeriClkCtrl(pUsart3, ENABLE);
	USART_PeripheralCtrl(pUsart3, ENABLE);

	usartTx2Done = 0U;
	usartRx3Done = 0U;
	usartTx3Done = 0U;
	usartRx2Done = 0U;
	usartError   = 0U;
	usartOk      = 0U;

	/* 3. Phase 1: USART2 -> USART3. Wait until both are done (or an error occurs). */
	USART_ReceiveDataIT(&usartConfig[USART3_IDX], usartRxBuffer, len);
	USART_SendDataIT(&usartConfig[USART2_IDX], (uint8_t *)user_data, len);
	while ((!usartTx2Done || !usartRx3Done) && !usartError) { }
	usartRxBuffer[len] = '\0';

	/* 4. USART3 reverses the received text. */
	for (uint32_t i = 0U; i < len; i++)
	{
		usartReverseBuffer[i] = usartRxBuffer[len - 1U - i];
	}

	/* 5. Phase 2: USART3 -> USART2. Wait until both are done (or an error occurs). */
	USART_ReceiveDataIT(&usartConfig[USART2_IDX], usartEchoBuffer, len);
	USART_SendDataIT(&usartConfig[USART3_IDX], usartReverseBuffer, len);
	while ((!usartTx3Done || !usartRx2Done) && !usartError) { }
	usartEchoBuffer[len] = '\0';

	/* 6. The echo must be the ORIGINAL text in reverse order. */
	usartOk = (usartError == 0U);
	for (uint32_t i = 0U; i < len; i++)
	{
		if (usartEchoBuffer[i] != (uint8_t)user_data[len - 1U - i])
		{
			usartOk = 0U;
		}
	}

	/* 7. Disable the USARTs first (UE = 0), then their clocks. */
	USART_PeripheralCtrl(pUsart2, DISABLE);
	USART_PeriClkCtrl(pUsart2, DISABLE);
	USART_PeripheralCtrl(pUsart3, DISABLE);
	USART_PeriClkCtrl(pUsart3, DISABLE);
}

/**
 * @brief  Application event callback (overrides the weak driver function).
 *
 * @details Called from USART_IRQHandling(). The handle tells which USART
 *          reported the event.
 *          - USART_EVENT_TX_CMPLT: all bytes were sent (TC = 1).
 *          - USART_EVENT_RX_CMPLT: all requested bytes were received.
 *          - Any other event is an error code and is stored in usartError.
 *
 * @param  pUSARTHandle: Pointer to the USART handle that reported the event.
 * @param  Event:        USART_EVENT_xxx or USART_ERR_xxx code.
 * @retval None
 */
void USART_ApplicationEventCallback(USART_Hndl_t *pUSARTHandle, uint8_t Event)
{
	if (Event == USART_EVENT_TX_CMPLT)
	{
		if (pUSARTHandle == &usartConfig[USART2_IDX]) { usartTx2Done = 1U; }
		else                                          { usartTx3Done = 1U; }
	}
	else if (Event == USART_EVENT_RX_CMPLT)
	{
		if (pUSARTHandle == &usartConfig[USART2_IDX]) { usartRx2Done = 1U; }
		else                                          { usartRx3Done = 1U; }
	}
	else
	{
		usartError = Event;
	}
}

/**
 * @brief  Interrupt Service Routine (ISR) for USART2 global interrupts.
 * @param  None
 * @retval None
 */
void USART2_IRQHandler(void)
{
	// Delegate interrupt flag evaluation and handling to the USART driver.
	USART_IRQHandling(&usartConfig[USART2_IDX]);
}

/**
 * @brief  Interrupt Service Routine (ISR) for USART3 global interrupts.
 * @param  None
 * @retval None
 */
void USART3_IRQHandler(void)
{
	// Delegate interrupt flag evaluation and handling to the USART driver.
	USART_IRQHandling(&usartConfig[USART3_IDX]);
}

#endif
