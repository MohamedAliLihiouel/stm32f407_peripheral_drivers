/**
 * @file    app_spi_master_tx.c
 * @brief   SPI2 master configuration and data transmission test.
 *
 * @details Configures the GPIO pins PB12-PB15 for the SPI2 alternate
 *          function (AF5) and initializes SPI2 in master mode with
 *          full-duplex communication and 8-bit data frames.
 *
 *          The GPIO and SPI configurations are stored in configuration
 *          tables and applied by SPI_ConfigGPIOPins() and SPI_ConfigAll().
 *
 *          SPI_MasterSendData() enables the SPI peripheral, transmits
 *          the test message "hello World", and disables the peripheral
 *          after the transmission function returns.
 *
 * @note    SPI2 is configured for software slave management (SSM).
 *          SPI_SSIConfig() sets the internal slave-select level.
 *          The external NSS pin configuration depends on the intended
 *          hardware setup.
 *
 * @date    Oct 20, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
 */

#include <stdint.h>
#include "app_spi_master_tx.h"
#include "stm32f407_LedToggle_ButtonB1.h"
#include "stm32f407_LedToggle_ButtonB1_Interupt.h"
#include "stm32f407_GPIO_driver.h"
#include "stm32f407_SPI_driver.h"


#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#if(APP_SPI_MASTER_TX == STD_ON)

/* tbd1 -> to be deleted */
// GPIO -> SPI2
// PB12 : AF5 SPI2_NSS
// PB13 : AF5 SPI2_SCK x CH0
// PB14 : AF5 SPI2_MISO
// PB15 : AF5 SPI2_MOSI x CH1
static GPIO_PinHndl_t gpioConfig[] =
{
    // PB12: SPI2_NSS (optional, if needed)
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

    // PB13: SPI2_SCK
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

    // PB14: SPI2_MISO
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

    // PB15: SPI2_MOSI
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
    }
};

static SPI_Hndl_t spiConfig[] =
{
    /* SPI2: Master */
    {
        .pSPIx_StartAddr = SPI2,
        .SPI_Config =
        {
            .SPI_DeviceMode   = SPI_DEVICE_MODE_MASTER,
            .SPI_BusConfig    = SPI_BUS_CONFIG_FULL_DUPLEX,
            .SPI_DFF          = SPI_DFF_8BIT,
            .SPI_CPHA         = SPI_CPHA_FIRST_EDGE,
            .SPI_CPOL         = SPI_CPOL_LOW,
            .SPI_SSM          = SPI_SSM_SOFTWARE,
            .SPI_Speed        = SPI_SPEED_DIV_2
        }
    }
};

/**
 * @brief  Configure all GPIO pins defined in the configuration table.
 *
 * @details Enables the clock for each GPIO port referenced by the
 *          configuration table and initializes the corresponding pins.
 *
 * @param  None
 * @retval None
 */
void SPI_ConfigGPIOPins(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(gpioConfig); i++)
    {
        GPIO_PeriClkCtrl(gpioConfig[i].pGPIO_StartAddr, ENABLE);
        GPIO_Init(&gpioConfig[i]);
    }
}

/**
 * @brief  Configure and initialize all SPI peripherals in the configuration table.
 *
 * @details Enables the peripheral clock for each configured SPI instance
 *          and applies its settings using SPI_Init().
 *
 *          Configures the SPI2 internal slave-select level when software
 *          slave management is enabled.
 *
 * @param  None
 * @retval None
 */
void SPI_ConfigAll(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(spiConfig); i++)
    {
    	SPI_PeriClkCtrl(spiConfig[i].pSPIx_StartAddr, ENABLE);
    	SPI_Init(&spiConfig[i]);
    }
    SPI_SSIConfig(SPI2, ENABLE);
}

/**
 * @brief  Transmit a test message using SPI2 in master mode.
 *
 * @details Enables the SPI2 peripheral, sends the test message
 *          "hello World" through SPI_SendData(), and disables SPI2
 *          after SPI_SendData() returns.
 *
 * @param  None
 * @retval None
 *
 * @note   SPI_SendData() must return only after the transmit operation
 *         has reached the point required by the driver's contract.
 *         The function's return does not necessarily mean that the
 *         final frame has completely left the SPI peripheral.
 */
void SPI_MasterSendData()
{
	char user_data[] = "hello World";

	SPI_PeripheralCtrl(SPI2, ENABLE);
	SPI_SendData(SPI2, (uint8_t*)user_data, strlen(user_data));
	SPI_PeripheralCtrl(SPI2, DISABLE);
}

#endif
/* End tbd1 */
