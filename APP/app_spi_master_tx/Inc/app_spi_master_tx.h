/**
 * @file    app_spi_master_tx.h
 * @brief   Interface for the SPI master transmission test application.
 *
 * @details Provides the public function declarations required to configure
 *          the SPI GPIO pins, initialize the SPI peripheral, and transmit
 *          a test message using SPI2 in master mode.
 *
 *          The application can be enabled or disabled through the
 *          APP_SPI_MASTER_TX configuration switch.
 *
 * @date    Oct 20, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
 */

#ifndef APP_SPI_MASTER_TX_H_
#define APP_SPI_MASTER_TX_H_

#include "stm32f407_SPI_driver.h"

#define APP_SPI_MASTER_TX STD_ON

#if (APP_SPI_MASTER_TX == STD_ON)
extern void SPI_ConfigGPIOPins(void);
extern void SPI_ConfigAll(void);
extern void SPI_MasterSendData(void);
#endif /* APP_SPI_MASTER_TX == STD_ON */

#endif /* APP_SPI_MASTER_TX_H_ */
