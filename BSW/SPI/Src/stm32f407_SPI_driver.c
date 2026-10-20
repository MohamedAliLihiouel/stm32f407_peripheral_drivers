/*
 * stm32f407_SPI_driver.c
 *
 *  Created on: Oct 17, 2026
 *      Author: Moham
 */

#include "stm32f407_SPI_driver.h"

/**
 * @brief   Initializes the SPI peripheral according to the specified
 *          parameters in the SPI_Hndl_t structure.
 *
 * Supported configurations include:
 * - Device mode (Master/Slave)
 * - Bus configuration (Full-duplex, Half-duplex, Simplex RX)
 * - Data frame format (8-bit or 16-bit)
 * - Clock phase (CPHA) and clock polarity (CPOL)
 * - Software slave management (SSM)
 * - Baud rate prescaler (SPI speed)
 *
 * @param   pSPIHandle: Pointer to an SPI handle structure (`SPI_Hndl_t`)
 *                      containing the base address and user configuration settings.
 * @retval  None
 */
void SPI_Init(SPI_Hndl_t *pSPIHandle)
{
    uint32_t tempReg = 0U;

    /* 1. Configure Master/Slave Selection */
    tempReg |= (pSPIHandle->SPI_Config.SPI_DeviceMode << SPI_CR1_MSTR);

    /* 2. Configure SPI Bus Directionality (Full-Duplex, Half-Duplex, or Simplex) */
    if (pSPIHandle->SPI_Config.SPI_BusConfig == SPI_BUS_CONFIG_FULL_DUPLEX)
    {
        // 2-line unidirectional data mode
        tempReg &= ~(1U << SPI_CR1_BIDIMODE);
    }
    else if (pSPIHandle->SPI_Config.SPI_BusConfig == SPI_BUS_CONFIG_HALF_DUPLEX)
    {
        // 1-line bidirectional data mode
        tempReg |= (1U << SPI_CR1_BIDIMODE);
    }
    else if (pSPIHandle->SPI_Config.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RX)
    {
        // 2-line unidirectional data mode with output disabled (Receive-only)
        tempReg &= ~(1U << SPI_CR1_BIDIMODE);
        tempReg |=  (1U << SPI_CR1_RXONLY);
    }

    /* 3. Configure Data Frame Format (8-bit or 16-bit) */
    tempReg |= (pSPIHandle->SPI_Config.SPI_DFF << SPI_CR1_DFF);

    /* 4. Configure Clock Phase (CPHA) */
    tempReg |= (pSPIHandle->SPI_Config.SPI_CPHA << SPI_CR1_CPHA);

    /* 5. Configure Clock Polarity (CPOL) */
    tempReg |= (pSPIHandle->SPI_Config.SPI_CPOL << SPI_CR1_CPOL);

    /* 6. Configure Software Slave Management (SSM) */
    tempReg |= (pSPIHandle->SPI_Config.SPI_SSM << SPI_CR1_SSM);

    /* 7. Configure Baud Rate Prescaler (SPI Speed) */
    tempReg |= (pSPIHandle->SPI_Config.SPI_Speed << SPI_CR1_BR);

    /* 8. Write the finalized configuration value into the SPI_CR1 register */
    pSPIHandle->pSPIx_StartAddr->CR1 = tempReg;
}


/**
 * @brief  De-initializes the specified SPI peripheral, resetting all its registers to their default state.
 * @param  pSPIx: Pointer to the base address of the target SPI peripheral (e.g., SPI1, SPI2).
 * @retval None
 */
void SPI_DeInit(SPI_Registers_t *pSPIx)
{
	switch ((uint32_t)pSPIx)
	{
		case (uint32_t)SPI1:
			RCC_SPI1_RESET();
			break;
		case (uint32_t)SPI2:
			RCC_SPI2_RESET();
			break;
		case (uint32_t)SPI3:
			RCC_SPI3_RESET();
			break;
		case (uint32_t)SPI4:
			RCC_SPI4_RESET();
			break;
		default:
			break;
	}
}

/**
 * @brief  Enables or disables the peripheral clock for the given SPI peripheral.
 * @param  pSPIx: Pointer to the base address of the target SPI peripheral.
 * @param  EnOrDi: Macro for enabling or disabling the clock (e.g., ENABLE or DISABLE).
 * @retval None
 */
void SPI_PeriClkCtrl(SPI_Registers_t *pSPIx, uint8_t EnOrDi)
{
	if (EnOrDi == 1) // Enable
	{
		switch ((uint32_t)pSPIx)
		{
			case (uint32_t)SPI1:
				RCC_SPI1_CLK_ENABLE();
				break;
			case (uint32_t)SPI2:
				RCC_SPI2_CLK_ENABLE();
				break;
			case (uint32_t)SPI3:
				RCC_SPI3_CLK_ENABLE();
				break;
			case (uint32_t)SPI4:
				RCC_SPI4_CLK_ENABLE();
				break;
			default:
				break;
		}
	}
	else
	{
		switch ((uint32_t)pSPIx)
		{
			case (uint32_t)SPI1:
				RCC_SPI1_CLK_DISABLE();
				break;
			case (uint32_t)SPI2:
				RCC_SPI2_CLK_DISABLE();
				break;
			case (uint32_t)SPI3:
				RCC_SPI3_CLK_DISABLE();
				break;
			case (uint32_t)SPI4:
				RCC_SPI4_CLK_DISABLE();
				break;
			default:
				break;
		}
	}
}

/**
 * @brief   Transmits data over the SPI bus in Master or Slave mode.
 *
 * Supports both 8-bit and 16-bit data frame formats (DFF) based on
 * the SPI configuration registers. Handles buffer transmission iteratively
 * until the specified length is fully sent.
 *
 * @param   pSPIx   Pointer to the SPI peripheral registers base address (`SPI_Registers_t`)
 * @param   pTxBuff Pointer to the data buffer to be transmitted (`uint8_t*`)
 * @param   Length  Number of bytes (or data frames) to transmit (`uint32_t`)
 * @retval  None
 */
void SPI_SendData(SPI_Registers_t *pSPIx, uint8_t *pTxBuff, uint32_t Length)
{
	// Continue until all bytes have been loaded into the SPI peripheral.
	while (Length > 0)
	{

		// Wait until the transmit data register can accept new data.
		// This is a blocking call
		while ((pSPIx->SR & (1U << SPI_SR_TXE)) == 0U)
		{
			// Poll TXE until the register is ready.
		}
		// Check the configured SPI data frame size.
		if((pSPIx->CR1 & (1U << SPI_CR1_DFF)) == SPI_DFF_8BIT)
		{
			// Write one byte to the SPI data register.
			pSPIx->DR =   *pTxBuff;
			// One byte has been processed.
			Length--;
			pTxBuff++;
		}
		else
		{
			// A 16-bit frame requires two bytes from the buffer.
			if (Length < 2U)
			{
				// Invalid remaining length for a 16-bit frame.
				return;
			}
			// Combine two bytes into one 16-bit data frame.
			uint16_t data = (uint16_t)pTxBuff[0]
						  | ((uint16_t)pTxBuff[1] << 8U);

			// Write one 16-bit frame to the SPI data register.
			pSPIx->DR = data;

			// Advance the buffer by two bytes.
			pTxBuff += 2;
			Length -= 2U;
		}
	}
}

void SPI_ReceiveData(SPI_Registers_t *pSPIx, uint8_t *pRxBuff, uint32_t Length)
{

}


/**
 * @brief  Enables or disables the SPI peripheral.
 *
 * Controls the SPI peripheral by setting or clearing the SPE
 * (SPI Enable) bit in the SPI Control Register 1 (CR1).
 *
 * @param  pSPIx   Pointer to the SPI peripheral registers.
 * @param  EnOrDi  Peripheral control command:
 *                 ENABLE  - Enable the SPI peripheral.
 *                 DISABLE - Disable the SPI peripheral.
 *
 * @retval None
 */
void SPI_PeripheralCtrl(SPI_Registers_t *pSPIx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        // Set the SPE bit to enable the SPI peripheral.
        pSPIx->CR1 |= (1U << SPI_CR1_SPE);
    }
    else
    {
        // Clear the SPE bit to disable the SPI peripheral.
        pSPIx->CR1 &= ~(1U << SPI_CR1_SPE);
    }
}

/**
 * @brief   Enables or disables Software Slave Management Internal Slave Select (SSI).
 *
 * Controls the internal slave select level by setting or clearing the SSI
 * (Internal Slave Select) bit in the SPI Control Register 1 (CR1) when Software
 * Slave Management (SSM) is enabled.
 *
 * @param   pSPIx   Pointer to the SPI peripheral registers.
 * @param   EnOrDi  Peripheral control command:
 *                  ENABLE  - Set the SSI bit (high level / internal select).
 *                  DISABLE - Clear the SSI bit (low level).
 * @retval  None
 */
void SPI_SSIConfig(SPI_Registers_t *pSPIx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        /* Set the SSI bit to drive the internal NSS high */
        pSPIx->CR1 |= (1U << SPI_CR1_SSI);
    }
    else
    {
        /* Clear the SSI bit to drive the internal NSS low */
        pSPIx->CR1 &= ~(1U << SPI_CR1_SSI);
    }
}

void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{

}

void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority)
{

}

void SPI_IRQHandling(SPI_Hndl_t *pSPIHandle)
{

}

