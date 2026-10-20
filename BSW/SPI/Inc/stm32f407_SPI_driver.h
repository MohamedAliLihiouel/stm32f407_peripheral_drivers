/*
 * stm32f407_SPI_driver.h
 *
 *  Created on: Oct 17, 2026
 *      Author: Moham
 */

#ifndef STM32F407_SPI_DRIVER_H_
#define STM32F407_SPI_DRIVER_H_


#include "stm32f407.h"


/******************************************************************************
 * Structures
 ******************************************************************************/

/**
 * @brief SPI pin configuration structure.
 *
 * @details
 * Contains the configuration parameters required to initialise a SPI peripheral,
 * including its tbd.
 *
 * The selected parameters are applied by SPI_Init().
 */
typedef struct
{
    uint8_t SPI_DeviceMode;       /*!< SPI device mode: Master or Slave .
                                       @ref SPI_device_mode */
    uint8_t SPI_BusConfig;         /*!< SPI bus configuration: Full-Duplex, Half-Duplex, Simplex (TX) and Simplex (RX)
                                       @ref SPI_bus_config */
    uint8_t SPI_DFF;               /*!< SPI Data format
                                       @ref SPI_data_format: 8 or 16 bits */
    uint8_t SPI_CPHA;              /*!< SPI clock phase: 0 Data is sampled on the first edge, 1 Data is sampled on the second edge
                                       @ref SPI_clock_phase*/
    uint8_t SPI_CPOL;              /*!< Clock Polarity: Defines the idle state of the clock line(SCLK) when no data is being transmitted
                                       @ref SPI_clock_polarity */
    uint8_t SPI_SSM;               /*!< Software Slave Management: Hardware or Software
                                       @ref GPIO_alt_functions */
    uint8_t SPI_Speed;              /*!< SPI Speed & Clock Prescaler
                                       @ref SPI_speed */
} SPI_Config_t;


/**
 * @brief SPI handle structure.
 *
 * @details
 * Associates a SPI peripheral with its configuration.
 *
 * The peripheral pointer identifies the SPI peripheral whose registers are
 * accessed by the driver, while SPI_Config contains the settings
 * used to configure the selected pin.
 */
typedef struct
{
	SPI_Registers_t *pSPIx_StartAddr; /*!< Base address of the SPI peripheral (e.g., SPI1, SPI2). */
    SPI_Config_t SPI_Config;          /*!< Configuration parameters for the selected SPI peripheral. */
} SPI_Hndl_t;

/******************************************************************************
 * Macros
 ******************************************************************************/

/* SPI_CR1 Register Bit Position Definitions */
#define SPI_CR1_CPHA        (0U)   /*!< Clock Phase */
#define SPI_CR1_CPOL        (1U)   /*!< Clock Polarity */
#define SPI_CR1_MSTR        (2U)   /*!< Master Selection */
#define SPI_CR1_BR          (3U)   /*!< Baud Rate Control (Bits 3 to 5) */
#define SPI_CR1_SPE         (6U)   /*!< SPI Enable */
#define SPI_CR1_LSBFIRST    (7U)   /*!< Frame Format */
#define SPI_CR1_SSI         (8U)   /*!< Internal Slave Select */
#define SPI_CR1_SSM         (9U)   /*!< Software Slave Management */
#define SPI_CR1_RXONLY      (10U)  /*!< Receive-Only Mode */
#define SPI_CR1_DFF         (11U)  /*!< Data Frame Format */
#define SPI_CR1_CRCNEXT     (12U)  /*!< Transmit CRC Next */
#define SPI_CR1_CRCEN       (13U)  /*!< Hardware CRC Calculation Enable */
#define SPI_CR1_BIDIOE      (14U)  /*!< Output Enable in Bidirectional Mode */
#define SPI_CR1_BIDIMODE    (15U)  /*!< Bidirectional Data Mode Enable */

/*------------------------------------------------------------------------------
 * SPI_CR2 Register Bit Position Definitions
 *----------------------------------------------------------------------------*/
#define SPI_CR2_RXDMAEN     (0U)   /**< Rx Buffer DMA Enable */
#define SPI_CR2_TXDMAEN     (1U)   /**< Tx Buffer DMA Enable */
#define SPI_CR2_SSOE        (2U)   /**< SS Output Enable */
#define SPI_CR2_RESERVED    (3U)   /**< Reserved */
#define SPI_CR2_FRF         (4U)   /**< Frame Format */
#define SPI_CR2_ERRIE       (5U)   /**< Error Interrupt Enable */
#define SPI_CR2_RXNEIE      (6U)   /**< RX Buffer Not Empty Interrupt Enable */
#define SPI_CR2_TXEIE       (7U)   /**< Tx Buffer Empty Interrupt Enable */

/*------------------------------------------------------------------------------
 * SPI_SR Register Bit Position Definitions
 *----------------------------------------------------------------------------*/
#define SPI_SR_RXNE         (0U)   /**< Receive Buffer Not Empty */
#define SPI_SR_TXE          (1U)   /**< Transmit Buffer Empty */
#define SPI_SR_CHSIDE       (2U)   /**< Channel Side (Left/Right) */
#define SPI_SR_UDR          (3U)   /**< Underrun Flag */
#define SPI_SR_CRCERR       (4U)   /**< CRC Error Flag */
#define SPI_SR_MODF         (5U)   /**< Mode Fault */
#define SPI_SR_OVR          (6U)   /**< Overrun Flag */
#define SPI_SR_BSY          (7U)   /**< Busy Flag */
#define SPI_SR_FRE          (8U)   /**< Frame Format Error */

/**
 * @defgroup SPI_device_mode SPI Device Mode
 * @brief Defines whether the SPI module operates as Master or Slave.
 * @{
 */
#define SPI_DEVICE_MODE_SLAVE       0U  /*!< Configure SPI as Slave device.  */
#define SPI_DEVICE_MODE_MASTER      1U  /*!< Configure SPI as Master device. */
/** @} */


/**
 * @defgroup SPI_bus_config SPI Bus Configuration
 * @brief Communication modes defining line usage and data flow direction.
 * @{
 */
#define SPI_BUS_CONFIG_FULL_DUPLEX  0U  /*!< 4-wire simultaneous bi-directional TX/RX.     */
#define SPI_BUS_CONFIG_HALF_DUPLEX  1U  /*!< 3-wire shared data line for sequential TX/RX. */
#define SPI_BUS_CONFIG_SIMPLEX_RX   2U  /*!< Receive-only mode (MOSI omitted).             */
/** @} */


/**
 * @defgroup SPI_data_format SPI Data Frame Format
 * @brief Specifies frame size for transmission and reception.
 * @{
 */
#define SPI_DFF_8BIT                0U  /*!< 8-bit data frame format.  */
#define SPI_DFF_16BIT               1U  /*!< 16-bit data frame format. */
/** @} */


/**
 * @defgroup SPI_clock_phase SPI Clock Phase
 * @brief Defines which clock edge is used for data sampling.
 * @{
 */
#define SPI_CPHA_FIRST_EDGE         0U  /*!< Data sampled on the first (leading) clock edge.   */
#define SPI_CPHA_SECOND_EDGE        1U  /*!< Data sampled on the second (trailing) clock edge. */
/** @} */


/**
 * @defgroup SPI_clock_polarity SPI Clock Polarity
 * @brief Defines the steady state of the SCLK line when idle.
 * @{
 */
#define SPI_CPOL_LOW                0U  /*!< SCLK idles at logic low (0V).   */
#define SPI_CPOL_HIGH               1U  /*!< SCLK idles at logic high (VDD). */
/** @} */


/**
 * @defgroup SPI_slave_management SPI Software Slave Management
 * @brief Controls whether Chip Select (NSS) is driven by hardware or software.
 * @{
 */
#define SPI_SSM_HARDWARE            0U  /*!< Hardware NSS management enabled. */
#define SPI_SSM_SOFTWARE            1U  /*!< Software NSS management enabled. */
/** @} */


/**
 * @defgroup SPI_speed SPI Clock Prescaler
 * @brief Baud rate prescaler values derived from the peripheral bus clock (PCLK).
 * @{
 */
#define SPI_SPEED_DIV_2             0U  /*!< SCLK = PCLK / 2.   */
#define SPI_SPEED_DIV_4             1U  /*!< SCLK = PCLK / 4.   */
#define SPI_SPEED_DIV_8             2U  /*!< SCLK = PCLK / 8.   */
#define SPI_SPEED_DIV_16            3U  /*!< SCLK = PCLK / 16.  */
#define SPI_SPEED_DIV_32            4U  /*!< SCLK = PCLK / 32.  */
#define SPI_SPEED_DIV_64            5U  /*!< SCLK = PCLK / 64.  */
#define SPI_SPEED_DIV_128           6U  /*!< SCLK = PCLK / 128. */
#define SPI_SPEED_DIV_256           7U  /*!< SCLK = PCLK / 256. */
/** @} */

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/** @defgroup SPI_Driver_APIs SPI Driver APIs
 * @brief Public functions provided by the SPI driver.
 *
 * @details
 * The API provides SPI initialisation, peripheral clock control,
 * Send/receive SPI Data, and interrupt configuration and handling.
 * @{
 */
/******************************************************************************
 * SPI Initialisation and Deinitialisation
 ******************************************************************************/
extern void SPI_Init(SPI_Hndl_t *pSPIHandle);
extern void SPI_DeInit(SPI_Registers_t *pSPIx);


/******************************************************************************
 * SPI Peripheral Clock Control
 ******************************************************************************/
extern void SPI_PeriClkCtrl(SPI_Registers_t *pSPIx, uint8_t EnOrDi);


/******************************************************************************
 * SPI Send and Receive
 ******************************************************************************/
extern void SPI_SendData(SPI_Registers_t *pSPIx, uint8_t *pTxBuff, uint32_t Length);
extern void SPI_ReceiveData(SPI_Registers_t *pSPIx, uint8_t *pRxBuff, uint32_t Length);

/******************************************************************************
 * SPI Peripheral Control: Enable/Disable
 ******************************************************************************/
extern void SPI_PeripheralCtrl(SPI_Registers_t *pSPIx, uint8_t EnOrDi);
extern void SPI_SSIConfig(SPI_Registers_t *pSPIx, uint8_t EnOrDi);

/******************************************************************************
 * SPI Interrupt Configuration and Handling
 ******************************************************************************/
extern void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
extern void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
extern void SPI_IRQHandling(SPI_Hndl_t *pSPIHandle);


/** @} */ /* End of GPIO_Driver_APIs */


#endif /* STM32F407_SPI_DRIVER_H_ */
