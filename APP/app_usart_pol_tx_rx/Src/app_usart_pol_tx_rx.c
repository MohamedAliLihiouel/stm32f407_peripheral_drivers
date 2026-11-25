/*
 * File: app_usart_pol_tx_rx.c
 *
 * Description:
 * USART test application for the STM32F407 Discovery board (polling mode).
 * After a press of the user button (PA0), USART2 sends the text
 * "hello I am USART2" byte by byte and USART3 receives every byte.
 * Both blocking driver functions are used, no interrupts.
 *
 * Wiring (jumper wires):
 *   PA2 (USART2_TX) -> PD9 (USART3_RX)
 *   PD8 (USART3_TX) -> PA3 (USART2_RX)   (not used in this test)
 *   115200 baud, 8 data bits, no parity, 1 stop bit.
 *   The signal can be checked with a logic analyser on PA2.
 *
 * Result: usartRxOk = 1 and usartRxBuffer holds the text (check in the debugger).
 *
 * @date    Nov 24, 2026
 * @author  Mohamed Ali, Lihiouel
 */

#include <stdint.h>
#include <string.h>
#include "app_usart_pol_tx_rx.h"
#include "stm32f407_GPIO_driver.h"
#include "stm32f407_USART_driver.h"

#if(APP_USART_POL_TX_RX == STD_ON)

/******************************************************************************
 * Private macros and variables
 ******************************************************************************/

/* Number of elements of a C array. */
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

/******************************************************************************
 * USART test application: USART2 transmits a text (polling mode)
 * Measurement with a logic analyser on PA2 (USART2_TX) + common GND.
 ******************************************************************************/

/* Receive buffer and result flag of the loopback test (check in the debugger). */
static uint8_t          usartRxBuffer[32];     /* text received by USART3     */
static volatile uint8_t usartRxOk = 0U;   /* 1 = all frames received correctly */

/**
 * @brief GPIO configuration table of the USART test application.
 *
 * @details
 * Pins used by USART2 (STM32F407 Discovery):
 * - PA0 : User push-button B1 (input, starts the transfer)
 * - PA2 : AF7 USART2_TX
 * - PA3 : AF7 USART2_RX
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
            .GPIO_PinMode        = GPIO_MODE_ALTFUNC,   /* alternate function   */
            .GPIO_PinSpeed       = GPIO_SPEED_MEDIUM,   /* enough for 115200 baud */
            .GPIO_PinOPType      = GPIO_OTYPE_PP,       /* push-pull            */
            .GPIO_PinPuPdRCtrl   = GPIO_NO_PUPD,        /* no pull resistor     */
            .GPIO_PinAltFuncMode = 7                    /* AF7 = USART1..USART3 */
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
            .GPIO_PinPuPdRCtrl   = GPIO_PIN_PU,         /* idle level HIGH      */
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
			.GPIO_PinPuPdRCtrl   = GPIO_PIN_PU,         /* idle level HIGH */
			.GPIO_PinAltFuncMode = 7
		}
	}
};

/**
 * @brief USART peripheral configuration table of the test application.
 *
 * @details
 * One array element per peripheral:
 * - [0] = USART2 (TX and RX, 115200 baud, 8N1)
 * - [1] = USART3 (TX and RX, 115200 baud, 8N1)
 *
 * Wiring: PA2 (USART2_TX) -> PD9 (USART3_RX),
 * both boards pins are on the same Discovery board, so no extra GND is needed.
 */
static USART_Hndl_t usartConfig[] =
{
	/* [0] USART2: TX = PA2, RX = PA3 */
	{
		.pUSARTx_StartAddr = USART2,
		.USART_Config =
		{
			.USART_Mode          = USART_MODE_TXRX,          /* TE = 1, RE = 1            */
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
			.USART_Mode          = USART_MODE_TXRX,          /* TE = 1, RE = 1            */
			.USART_BaudRate      = USART_STD_BAUD_115200,    /* 115200 bit/s              */
			.USART_NoOfStopBits  = USART_STOPBITS_1,         /* 1 stop bit                */
			.USART_WordLength    = USART_WORDLEN_8BITS,      /* 8 data bits               */
			.USART_ParityControl = USART_PARITY_DISABLE,     /* no parity (8N1)           */
			.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE   /* no CTS / RTS              */
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
 *          (USART TX) and applies its settings using USART_Init().
 *
 * @param  None
 * @retval None
 *
 * @note   The USART is not enabled here (CR1.UE = 0). It is enabled in
 *         USART_PollingTx_Rx() with USART_PeripheralCtrl().
 */
void USART_ConfigAll(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(usartConfig); i++)
    {
    	// 1) Enable the peripheral clock, then 2) write the configuration registers.
    	USART_PeriClkCtrl(usartConfig[i].pUSARTx_StartAddr, ENABLE);
    	USART_Init(&usartConfig[i]);
    }
}

/**
 * @brief  Sends a test text from USART2 to USART3 (polling mode).
 *
 * @details Sequence:
 *          1) Wait for the user button (PA0) and debounce it.
 *          2) Enable the clocks and both peripherals (UE = 1).
 *          3) For each byte: USART_SendData() on USART2 (blocking, returns
 *             after TC = 1), then USART_ReceiveData() on USART3 (blocking).
 *             The received byte is compared with the sent byte.
 *          4) Disable the peripherals (UE = 0), then their clocks.
 *
 * @param  None
 * @retval None
 *
 * @note   UE must be set AFTER USART_Init(), because USART_Init() overwrites CR1.
 * @note   Sending and receiving byte by byte works, because the USART3 data
 *         register holds the byte until it is read.
 */
void USART_PollingTx_Rx(void)
{
    const char user_data[] = "hello I am USART2";
    uint32_t len = (uint32_t)strlen(user_data);

    /* Use the peripheral of the configuration table, so USART_Init(),
     * the clock, UE and USART_SendData() always address the same USART. */
    USART_Registers_t *pUsartA = usartConfig[0].pUSARTx_StartAddr;   /* USART2 */
    USART_Registers_t *pUsartB = usartConfig[1].pUSARTx_StartAddr;   /* USART3 */

    /* Wait until the user push-button (PA0) is pressed. */
    while (! GPIO_ReadInputPin(GPIOA, GPIO_PIN_0) )
    {
        /* wait */
    }
	// Apply software delay (~200 ms) to avoid mechanical button bouncing.
	delay();

	/* Enable clocks and both peripherals BEFORE sending. */
	USART_PeriClkCtrl(pUsartA, ENABLE);
	USART_PeripheralCtrl(pUsartA, ENABLE);

	USART_PeriClkCtrl(pUsartB, ENABLE);
	USART_PeripheralCtrl(pUsartB, ENABLE);

	usartRxOk = 1U;
	for (uint32_t i = 0; i < len; i++)
	{
	    /* USART2 -> USART3: send one frame and read it on the other USART. */
	    USART_SendData(&usartConfig[0], (const uint8_t *)&user_data[i], 1U);
	    USART_ReceiveData(&usartConfig[1], &usartRxBuffer[i], 1U);
	    if (usartRxBuffer[i] != (uint8_t)user_data[i]) { usartRxOk = 0U; }
	}
	usartRxBuffer[len] = '\0';

	// All frames are sent and received: disable the peripheral first (UE = 0),
	// then its clock.
	USART_PeripheralCtrl(pUsartA, DISABLE);
	USART_PeriClkCtrl(pUsartA, DISABLE);

	USART_PeripheralCtrl(pUsartB, DISABLE);
	USART_PeriClkCtrl(pUsartB, DISABLE);

#endif
