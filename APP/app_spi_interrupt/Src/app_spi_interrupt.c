/*
- File: app_spi_interrupt.c
 *
- Description:
- This file contains the application implementation for interrupt-driven
- SPI communication on the STM32F407 microcontroller.
 *
- It configures the required GPIO pins and SPI2 peripheral in master mode,
- triggers byte-by-byte interrupt-based transmission and reception upon
- button press, and handles SPI application callbacks as well as the
- SPI2 interrupt service routine.
 *
- @date    Nov 19, 2026
- @author  Mohamed Ali, Lihiouel
 */

#include <stdint.h>
#include <string.h>
#include "app_spi_interrupt.h"
#include "stm32f407_LedToggle_ButtonB1.h"
#include "stm32f407_LedToggle_ButtonB1_Interupt.h"
#include "stm32f407_GPIO_driver.h"
#include "stm32f407_SPI_driver.h"


#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#if(APP_SPI_INTERRUPT == STD_ON)

#define MAX_LENGTH 1000

volatile static uint8_t SPI_AppRcvStop = 0;
volatile static char SPI_AppRcvBuff[MAX_LENGTH] = "FFFFFFFFFFFFFFFFFFFFFF";
volatile static char SPI_AppReadByte;


// GPIO -> SPI2 Pin Mapping:
// PB12 : AF5 SPI2_NSS
// PB13 : AF5 SPI2_SCK  (Logic Analyzer CH0)
// PB14 : AF5 SPI2_MISO
// PB15 : AF5 SPI2_MOSI (Logic Analyzer CH1)
static GPIO_PinHndl_t gpioConfig[] =
{
    // PB12: SPI2_NSS (Hardware Slave Select output)
    {
        .pGPIO_StartAddr = GPIOB,
        .GPIO_PinConfig = {
            .GPIO_PinNumber      = GPIO_PIN_12,
            .GPIO_PinMode        = GPIO_MODE_ALTFUNC,
            .GPIO_PinSpeed       = GPIO_SPEED_HIGH,
            .GPIO_PinOPType      = GPIO_OTYPE_PP,
            .GPIO_PinPuPdRCtrl   = GPIO_NO_PUPD,
            .GPIO_PinAltFuncMode = 5
        }
    },

    // PB13: SPI2_SCK (Serial Clock)
    {
        .pGPIO_StartAddr = GPIOB,
        .GPIO_PinConfig = {
            .GPIO_PinNumber      = GPIO_PIN_13,
            .GPIO_PinMode        = GPIO_MODE_ALTFUNC,
            .GPIO_PinSpeed       = GPIO_SPEED_HIGH,
            .GPIO_PinOPType      = GPIO_OTYPE_PP,
            .GPIO_PinPuPdRCtrl   = GPIO_NO_PUPD,
            .GPIO_PinAltFuncMode = 5
        }
    },

    // PB14: SPI2_MISO (Master In Slave Out)
    {
        .pGPIO_StartAddr = GPIOB,
        .GPIO_PinConfig = {
            .GPIO_PinNumber      = GPIO_PIN_14,
            .GPIO_PinMode        = GPIO_MODE_ALTFUNC,
            .GPIO_PinSpeed       = GPIO_SPEED_HIGH,
            .GPIO_PinOPType      = GPIO_OTYPE_PP,
            .GPIO_PinPuPdRCtrl   = GPIO_NO_PUPD,
            .GPIO_PinAltFuncMode = 5
        }
    },

    // PB15: SPI2_MOSI (Master Out Slave In)
    {
        .pGPIO_StartAddr = GPIOB,
        .GPIO_PinConfig = {
            .GPIO_PinNumber      = GPIO_PIN_15,
            .GPIO_PinMode        = GPIO_MODE_ALTFUNC,
            .GPIO_PinSpeed       = GPIO_SPEED_HIGH,
            .GPIO_PinOPType      = GPIO_OTYPE_PP,
            .GPIO_PinPuPdRCtrl   = GPIO_NO_PUPD,
            .GPIO_PinAltFuncMode = 5
        }
    },

    // PA0: User Push-Button B1 (Trigger for SPI transmission)
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

    // PD12: On-board Green LED (Error / status indicator)
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
    }
};

static SPI_Hndl_t spiConfig[] =
{
    // SPI2: Configured in Master mode, Full-Duplex, 8-bit frame, Hardware NSS
    {
        .pSPIx_StartAddr = SPI2,
        .SPI_Config =
        {
            .SPI_DeviceMode   = SPI_DEVICE_MODE_MASTER,
            .SPI_BusConfig    = SPI_BUS_CONFIG_FULL_DUPLEX,
            .SPI_DFF          = SPI_DFF_8BIT,
            .SPI_CPHA         = SPI_CPHA_FIRST_EDGE,
            .SPI_CPOL         = SPI_CPOL_LOW,
            .SPI_SSM          = SPI_SSM_HARDWARE,
            .SPI_Speed        = SPI_SPEED_DIV_8 // SCLK = 2 MHz (assuming 16 MHz APB1 clock)
        }
    }
};

/**
- @brief  Generates a blocking software delay for button debouncing.
- @param  None
- @retval None
 */
void delay(void)
{
	for(uint32_t i = 0 ; i < 500000 ; i ++);
}

/**
- @brief  Configures and initializes all GPIO pins defined in the configuration table.
- @param  None
- @retval None
 */
void SPI_Int_ConfigGPIOPins(void)
{
    // 1) Enable peripheral clocks and initialize each configured GPIO pin.
    for (uint32_t i = 0; i < ARRAY_SIZE(gpioConfig); i++)
    {
        GPIO_PeriClkCtrl(gpioConfig[i].pGPIO_StartAddr, ENABLE);
        GPIO_Init(&gpioConfig[i]);
    }
}

/**
- @brief  Configures and initializes the SPI peripheral and its NVIC interrupt line.
- @param  None
- @retval None
 */
void SPI_Int_ConfigAll(void)
{
    // 1) Enable peripheral clocks and initialize all configured SPI instances.
    for (uint32_t i = 0; i < ARRAY_SIZE(spiConfig); i++)
    {
    	SPI_PeriClkCtrl(spiConfig[i].pSPIx_StartAddr, ENABLE);
    	SPI_Init(&spiConfig[i]);
    }

    // 2) Enable Slave Select Output (SSOE) for hardware NSS management in master mode.
    SPI_SSOEConfig(SPI2, ENABLE);

    // 3) Enable the SPI2 interrupt request line in the NVIC.
    SPI_IRQInterruptConfig(IRQ_NO_SPI2,ENABLE);
}

/**
- @brief  Transmits a test string over SPI2 in interrupt mode upon button press
-         and simultaneously receives incoming bytes until null termination.
- @param  None
- @retval None
 */
void SPI_Int_MasterSendData()
{
	uint32_t i = 0;
	// Message to be transmitted over SPI2.
	char user_data[] = "hello World";

	// Reset reception termination flag before starting a new transfer.
	SPI_AppRcvStop = 0;

	// 1) Wait until the user push-button (PA0) is pressed.
	while (! GPIO_ReadInputPin(GPIOA, GPIO_PIN_0) );

	// Apply software delay (~200 ms) to avoid mechanical button bouncing.
	delay();

	// 2) Enable the SPI2 peripheral before initiating data transfer.
	SPI_PeripheralCtrl(SPI2, ENABLE);

	// 3) Transmit and receive data byte-by-byte using non-blocking interrupt APIs
	//    until the reception callback signals completion ('\0' or buffer limit).
	while(!SPI_AppRcvStop)
	{
		// Queue 1 byte for interrupt-driven transmission (wait if TX is busy).
		while ( SPI_SendDataIT(&spiConfig[0], (uint8_t *)&user_data[i], 1) == SPI_BUSY_IN_TX);

		// Queue 1 byte for interrupt-driven reception (wait if RX is busy).
		while ( SPI_ReceiveDataIT(&spiConfig[0], (uint8_t *)&SPI_AppReadByte, 1) == SPI_BUSY_IN_RX );
		i++;
	}

	// 4) Disable the SPI2 peripheral after the transfer sequence is complete.
	SPI_PeripheralCtrl(SPI2, DISABLE);
}

/**
- @brief  Application callback invoked by the SPI driver upon asynchronous interrupt events.
- @param  pSPIHandle: Pointer to the SPI handle structure that triggered the event.
- @param  AppEv:      SPI application event identifier (e.g., SPI_EVENT_RX_CMPLT, SPI_EVENT_OVR_ERR).
- @retval None
 */
void SPI_ApplEventCallback(SPI_Hndl_t *pSPIHandle, uint8_t AppEv)
{
	static uint32_t i = 0;

	// 1) Handle reception complete event: store received byte and check for end of string.
	if(AppEv == SPI_EVENT_RX_CMPLT)
	{
		SPI_AppRcvBuff[i++] = SPI_AppReadByte;

		// Stop reception when null terminator is received or buffer capacity is reached.
		if(SPI_AppReadByte == '\0' || ( i == MAX_LENGTH)){
			SPI_AppRcvStop = 1;
			SPI_AppRcvBuff[i-1] = '\0';
			i = 0;
		}
	}
	// 2) Handle overrun error event: toggle on-board LED (PD12) as error indicator.
	else if (AppEv == SPI_EVENT_OVR_ERR)
	{
		GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_12);
	}
}

/**
- @brief  Interrupt Service Routine (ISR) for SPI2 global interrupts.
- @param  None
- @retval None
 */
void SPI2_IRQHandler(void)
{
	// Delegate interrupt flag evaluation and handling to the SPI driver.
	SPI_IRQHandling(&spiConfig[0]);
}

#endif