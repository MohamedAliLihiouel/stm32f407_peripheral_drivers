/*
- File: stm32f407_SPI_driver.c
 *
- Description:
- This file contains the implementation of the SPI driver for the
- STM32F407 microcontroller.
 *
- It provides functions to initialize and de-initialize SPI peripherals,
- configure peripheral clocks, control peripheral and slave-select states,
- perform blocking (polling) and non-blocking (interrupt-driven) data
- transmission and reception, and manage NVIC and SPI interrupt events.
 *
- @date    Oct 17, 2026
- @author  Mohamed Ali, Lihiouel
 */

#include <stddef.h>
#include "stm32f407_SPI_driver.h"
#include "nvic_config.h"
#include "app_spi_interrupt.h"

/* Static helper function prototypes for internal interrupt and state handling. */
static void SPI_TxeInterruptHandle(SPI_Hndl_t *pSPIHandle);
static void SPI_RxneInterruptHandle(SPI_Hndl_t *pSPIHandle);
static void SPI_OvrErrInterruptHandle(SPI_Hndl_t *pSPIHandle);
static void SPI_CloseTransmission(SPI_Hndl_t *pSPIHandle);
static void SPI_CloseReception(SPI_Hndl_t *pSPIHandle);
static void SPI_ClearOVRFlag(SPI_Registers_t *pSPIx);

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

    pSPIHandle->pSPIx_StartAddr->CR2 |= (1U << SPI_CR2_ERRIE);
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
    if (EnOrDi == ENABLE)
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
 * @param   Length  Number of bytes to transmit (`uint32_t`)
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
	while (!(pSPIx->SR & (1U << SPI_SR_TXE)));   // TX-Puffer leer
	while (  pSPIx->SR & (1U << SPI_SR_BSY));    // Übertragung fertig
}

/**
 * @brief   Receives data over the SPI bus using polling.
 *
 * @details Receives data by polling the RXNE (Receive Buffer Not Empty)
 *          flag. Supports both 8-bit and 16-bit data frames according
 *          to the DFF bit in SPI_CR1.
 *
 *          In 8-bit mode, each received frame is stored as one byte.
 *          In 16-bit mode, each received frame is stored as two bytes,
 *          with the least significant byte stored first.
 *
 * @param   pSPIx   Pointer to the SPI peripheral register structure.
 * @param   pRxBuff Pointer to the receive buffer.
 * @param   Length  Number of bytes to receive.
 *
 * @retval  None
 *
 * @note    This function uses blocking polling and waits until RXNE is set.
 *          In SPI master mode, the master must generate clock pulses
 *          to receive data from the slave.
 *
 */
void SPI_ReceiveData(SPI_Registers_t *pSPIx, uint8_t *pRxBuff, uint32_t Length)
{
	/* Continue until all bytes have been read into the SPI peripheral. */
	while (Length > 0)
	{

		/* Wait until a received data frame is available. */
		while ((pSPIx->SR & (1U << SPI_SR_RXNE)) == 0U)
		{
			/* Blocking wait for RXNE. */
		}
		/* Check the configured SPI data frame size. */
		if((pSPIx->CR1 & (1U << SPI_CR1_DFF)) == SPI_DFF_8BIT)
		{
			/* Read one byte from the SPI data register. */
			*pRxBuff = pSPIx->DR;
			/* One byte has been processed. */
			Length--;
			pRxBuff++;
		}
		else
		{
	        /* A 16-bit frame requires exactly two bytes in the buffer. */
	        if ((Length % 2U) != 0U)
	        {
	            return;
	        }

            /* Read the data register once to obtain the complete frame. */
            uint16_t data = (uint16_t)pSPIx->DR;

            /* Store the least significant byte first. */
            *pRxBuff++ = (uint8_t)(data & 0xFFU);
            *pRxBuff++ = (uint8_t)((data >> 8U) & 0xFFU);

            /* Two bytes have been received. */
            Length -= 2U;
		}
	}
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
        /* Set the SPE bit to enable the SPI peripheral. */
        pSPIx->CR1 |= (1U << SPI_CR1_SPE);
    }
    else
    {
        /* Clear the SPE bit to disable the SPI peripheral. */
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

/**
 * @brief   Enables or disables the SPI slave-select output.
 *
 * @details Controls the SSOE (SS Output Enable) bit in SPI_CR2.
 *          When enabled, the SPI peripheral controls the NSS output
 *          in master mode using hardware slave management.
 *
 * @param   pSPIx   Pointer to the SPI peripheral register structure.
 * @param   EnOrDi  Configuration command:
 *                  ENABLE  - Enable the hardware NSS output.
 *                  DISABLE - Disable the hardware NSS output.
 *
 * @retval  None
 *
 * @note    This setting is relevant when the SPI peripheral operates
 *          in master mode. The NSS pin must also be configured
 *          appropriately in the GPIO alternate-function mode.
 */
void SPI_SSOEConfig(SPI_Registers_t *pSPIx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        /* Enable hardware control of the NSS output. */
        pSPIx->CR2 |= (1U << SPI_CR2_SSOE);
    }
    else
    {
        /* Disable hardware control of the NSS output. */
        pSPIx->CR2 &= ~(1U << SPI_CR2_SSOE);
    }
}

/**
- @brief   Initiates non-blocking (interrupt-driven) SPI data transmission.
 *
 * @details Stores the transmit buffer pointer and length in the SPI handle,
 *         sets the transmission state to busy, and enables the TXEIE interrupt
 *         so data frames are loaded asynchronously inside the ISR.
 *
 * @param   pSPIHandle: Pointer to the SPI handle structure (`SPI_Hndl_t`).
 * @param   pTxBuff:    Pointer to the transmit data buffer.
 * @param   Length:     Number of bytes to transmit.
 * @retval  Previous transmission state (`SPI_READY` or `SPI_BUSY_IN_TX`).
 */
uint8_t SPI_SendDataIT(SPI_Hndl_t *pSPIHandle, uint8_t *pTxBuff, uint32_t Length)
{
    uint8_t state = pSPIHandle->TxState;

    if (state != SPI_BUSY_IN_TX)
    {
        // 1) Save the TX buffer address and length information in the SPI handle.
        pSPIHandle->pTxBuffer = pTxBuff;
        pSPIHandle->TxLen = Length;

        // 2) Mark the SPI state as busy in transmission so no other call
        //    can take over the same SPI peripheral until transmission completes.
        pSPIHandle->TxState = SPI_BUSY_IN_TX;

        // 3) Enable the TXEIE control bit to trigger an interrupt whenever TXE is set in SR.
        pSPIHandle->pSPIx_StartAddr->CR2 |= (1U << SPI_CR2_TXEIE);
    }

    return state;
}

/**
 * @brief   Initiates non-blocking (interrupt-driven) SPI data reception.
 *
 * @details Stores the receive buffer pointer and length in the SPI handle,
 *          sets the reception state to busy, and enables the RXNEIE interrupt
 *          so incoming data frames are read asynchronously inside the ISR.
 *
 * @param   pSPIHandle: Pointer to the SPI handle structure (`SPI_Hndl_t`).
 * @param   pRxBuff:    Pointer to the receive data buffer.
 * @param   Length:     Number of bytes to receive.
 * @retval  Previous reception state (`SPI_READY` or `SPI_BUSY_IN_RX`).
 */
uint8_t SPI_ReceiveDataIT(SPI_Hndl_t *pSPIHandle, uint8_t *pRxBuff, uint32_t Length)
{
    uint8_t state = pSPIHandle->RxState;

    if (state != SPI_BUSY_IN_RX)
    {
        // 1) Save the RX buffer address and length information in the SPI handle.
        pSPIHandle->pRxBuffer = pRxBuff;
        pSPIHandle->RxLen = Length;

        // 2) Mark the SPI state as busy in reception so no other call
        //    can take over the same SPI peripheral until reception completes.
        pSPIHandle->RxState = SPI_BUSY_IN_RX;

        // 3) Enable the RXNEIE control bit to trigger an interrupt whenever RXNE is set in SR.
        pSPIHandle->pSPIx_StartAddr->CR2 |= (1U << SPI_CR2_RXNEIE);
    }

    return state;
}

/**
 * @brief  Configures the interrupt parameters (IRQ number and activation) for SPI interrupts in the NVIC.
 * @param  IRQNumber: The Interrupt Request number in the NVIC.
 * @param  EnOrDi: Macro to enable or disable the interrupt configuration.
 * @retval None
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        if (IRQNumber < 32U)
        {
            // NVIC_ISER0
            *NVIC_ISER0 |= (1U << IRQNumber);
        }
        else if ((IRQNumber >= 32U) && (IRQNumber < 64U))
        {
            // NVIC_ISER1
            *NVIC_ISER1 |= (1U << (IRQNumber % 32U));
        }
        else if ((IRQNumber >= 64U) && (IRQNumber < 96U))
        {
            // NVIC_ISER2
            *NVIC_ISER2 |= (1U << (IRQNumber % 32U));
        }
    }
    else if (EnOrDi == DISABLE)
    {
        if (IRQNumber < 32U)
        {
            // NVIC_ICER0
            *NVIC_ICER0 |= (1U << IRQNumber);
        }
        else if ((IRQNumber >= 32U) && (IRQNumber < 64U))
        {
            // NVIC_ICER1
            *NVIC_ICER1 |= (1U << (IRQNumber % 32U));
        }
        else if ((IRQNumber >= 64U) && (IRQNumber < 96U))
        {
            // NVIC_ICER2
            *NVIC_ICER2 |= (1U << (IRQNumber % 32U));
        }
    }
}

/**
 * @brief  Configures the priority of an SPI interrupt in the NVIC.
 * @param  IRQNumber: The Interrupt Request number in the NVIC.
 * @param  IRQPriority: Priority configuration for the interrupt.
 * @retval None
 */
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority)
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
    pIpr[tempIprx] &= ~(0xFU << tempShiftImplDefinedPrio);
    /* Write the priority value into the corresponding field. */
    pIpr[tempIprx] |= ((uint32_t)IRQPriority << tempShiftImplDefinedPrio);
}

/**
 * @brief   Handles incoming SPI interrupts by evaluating status and control flags.
 *
 * @details Checks TXE (Transmit Buffer Empty), RXNE (Receive Buffer Not Empty),
 *          and OVR (Overrun Error) flags in SPI_SR together with their corresponding
 *          interrupt enable bits in SPI_CR2, delegating each active event to its
 *          internal handler.
 *
 * @param   pSPIHandle: Pointer to the SPI handle structure (`SPI_Hndl_t`).
 * @retval  None
 */
void SPI_IRQHandling(SPI_Hndl_t *pSPIHandle)
{
    uint8_t temp1, temp2;

    // 1) Check for TXE (Transmit buffer empty) interrupt.
    temp1 = (uint8_t)(pSPIHandle->pSPIx_StartAddr->SR & (1U << SPI_SR_TXE));
    temp2 = (uint8_t)(pSPIHandle->pSPIx_StartAddr->CR2 & (1U << SPI_CR2_TXEIE));

    if (temp1 && temp2)
    {
        // Handle TXE event.
        SPI_TxeInterruptHandle(pSPIHandle);
    }

    // 2) Check for RXNE (Receive buffer not empty) interrupt.
    temp1 = (uint8_t)(pSPIHandle->pSPIx_StartAddr->SR & (1U << SPI_SR_RXNE));
    temp2 = (uint8_t)(pSPIHandle->pSPIx_StartAddr->CR2 & (1U << SPI_CR2_RXNEIE));

    if (temp1 && temp2)
    {
        // Handle RXNE event.
        SPI_RxneInterruptHandle(pSPIHandle);
    }

    // 3) Check for OVR (Overrun error) interrupt.
    temp1 = (uint8_t)(pSPIHandle->pSPIx_StartAddr->SR & (1U << SPI_SR_OVR));
    temp2 = (uint8_t)(pSPIHandle->pSPIx_StartAddr->CR2 & (1U << SPI_CR2_ERRIE));

    if (temp1 && temp2)
    {
        // Handle OVR error event.
        SPI_OvrErrInterruptHandle(pSPIHandle);
    }
}

/**
- @brief   Clears the SPI Overrun (OVR) error flag.
 *
- @details Executes the hardware-required read sequence from SPI_DR followed by
-          SPI_SR to clear the OVR status bit.
 *
- @param   pSPIx: Pointer to the SPI peripheral registers.
- @retval  None
 */
static void SPI_ClearOVRFlag(SPI_Registers_t *pSPIx)
{
    uint32_t tempRead;
    tempRead = pSPIx->DR;
    tempRead = pSPIx->SR;
    (void)tempRead;
}

/**
- @brief   Internal helper to handle TXE (Transmit Buffer Empty) interrupts.
 *
- @details Loads the next 8-bit or 16-bit data frame from the transmit buffer into
-          SPI_DR, updates the remaining byte count, closes transmission once all
-          bytes are sent, and notifies the application via `SPI_ApplEventCallback`.
 *
- @param   pSPIHandle: Pointer to the SPI handle structure (`SPI_Hndl_t`).
- @retval  None
 */
static void SPI_TxeInterruptHandle(SPI_Hndl_t *pSPIHandle)
{
    // Check the configured SPI data frame size.
    if ((pSPIHandle->pSPIx_StartAddr->CR1 & (1U << SPI_CR1_DFF)) == (SPI_DFF_8BIT << SPI_CR1_DFF))
    {
        // Write one byte to the SPI data register.
        pSPIHandle->pSPIx_StartAddr->DR = *(pSPIHandle->pTxBuffer);
        // One byte has been processed.
        pSPIHandle->TxLen--;
        pSPIHandle->pTxBuffer++;
    }
    else
    {
        // A 16-bit frame requires two bytes from the buffer.
        if (pSPIHandle->TxLen < 2U)
        {
            SPI_CloseTransmission(pSPIHandle);
            // Invalid remaining length for a 16-bit frame.
            return;
        }

        // Combine two bytes into one 16-bit data frame.
        uint16_t data = (uint16_t)pSPIHandle->pTxBuffer[0]
                      | ((uint16_t)pSPIHandle->pTxBuffer[1] << 8U);

        // Write one 16-bit frame to the SPI data register.
        pSPIHandle->pSPIx_StartAddr->DR = data;

        // Advance the buffer by two bytes.
        pSPIHandle->TxLen -= 2U;
        pSPIHandle->pTxBuffer += 2;
    }

    if (pSPIHandle->TxLen == 0U)
    {
        // TxLen is zero: close SPI transmission and inform the application that TX is complete.
        // Disabling TXEIE prevents further interrupts from the TXE flag.
        SPI_CloseTransmission(pSPIHandle);
        SPI_ApplEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
    }
}

/**
- @brief   Internal helper to handle RXNE (Receive Buffer Not Empty) interrupts.
 *
- @details Reads the incoming 8-bit or 16-bit data frame from SPI_DR into the
-          receive buffer, updates the remaining byte count, closes reception once
-          all expected bytes are received, and notifies the application via
-          `SPI_ApplEventCallback`.
 *
- @param   pSPIHandle: Pointer to the SPI handle structure (`SPI_Hndl_t`).
- @retval  None
 */
static void SPI_RxneInterruptHandle(SPI_Hndl_t *pSPIHandle)
{
    /* Check the configured SPI data frame size. */
    if ((pSPIHandle->pSPIx_StartAddr->CR1 & (1U << SPI_CR1_DFF)) == (SPI_DFF_8BIT << SPI_CR1_DFF))
    {
        /* Read one byte from the SPI data register. */
        *(pSPIHandle->pRxBuffer) = (uint8_t)pSPIHandle->pSPIx_StartAddr->DR;
        /* One byte has been processed. */
        pSPIHandle->RxLen--;
        pSPIHandle->pRxBuffer++;
    }
    else
    {
        /* A 16-bit frame requires exactly two bytes in the buffer. */
        if ((pSPIHandle->RxLen % 2U) != 0U)
        {
            SPI_CloseReception(pSPIHandle);
            return;
        }

        /* Read the data register once to obtain the complete frame. */
        uint16_t data = (uint16_t)pSPIHandle->pSPIx_StartAddr->DR;

        /* Store the least significant byte first. */
        *(pSPIHandle->pRxBuffer)++ = (uint8_t)(data & 0xFFU);
        *(pSPIHandle->pRxBuffer)++ = (uint8_t)((data >> 8U) & 0xFFU);

        /* Two bytes have been received. */
        pSPIHandle->RxLen -= 2U;
    }

    if (pSPIHandle->RxLen == 0U)
    {
        // RxLen is zero: close SPI reception and inform the application that RX is complete.
        // Disabling RXNEIE prevents further interrupts from the RXNE flag.
        SPI_CloseReception(pSPIHandle);
        SPI_ApplEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
    }
}

/**
 * @brief   Internal helper to handle OVR (Overrun Error) interrupts.
 *
 * @details Clears the OVR status flag via a read sequence on SPI_DR and SPI_SR
 *          (unless a transmission is currently active, in which case the application
 *          must clear it via `SPI_ClearOVRFlag`) and notifies the application via
 *          `SPI_ApplEventCallback`.
 *
 * @param   pSPIHandle: Pointer to the SPI handle structure (`SPI_Hndl_t`).
 * @retval  None
 */
static void SPI_OvrErrInterruptHandle(SPI_Hndl_t *pSPIHandle)
{
    // 1) Clear the OVR flag if SPI is not currently busy transmitting.
    if (pSPIHandle->TxState != SPI_BUSY_IN_TX)
    {
        SPI_ClearOVRFlag(pSPIHandle->pSPIx_StartAddr);
    }

    // 2) Inform the application about the overrun error.
    SPI_ApplEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}

/**
 * @brief   Closes an active interrupt-driven SPI transmission.
 *
 * @details Disables the TXEIE interrupt and resets the transmit buffer pointer,
 *          length counter, and transmission state in the SPI handle.
 *
 * @param   pSPIHandle: Pointer to the SPI handle structure (`SPI_Hndl_t`).
 * @retval  None
 */
static void SPI_CloseTransmission(SPI_Hndl_t *pSPIHandle)
{
	pSPIHandle->pSPIx_StartAddr->CR2 &= ~(1U << SPI_CR2_TXEIE);
	pSPIHandle->pTxBuffer = NULL;
	pSPIHandle->TxLen = 0U;
	pSPIHandle->TxState = SPI_READY;

}

/**
 * @brief   Closes an active interrupt-driven SPI reception.
 *
 * @details Disables the RXNEIE interrupt and resets the receive buffer pointer,
 *          length counter, and reception state in the SPI handle.
 *
- @param   pSPIHandle: Pointer to the SPI handle structure (`SPI_Hndl_t`).
- @retval  None
 */
static void SPI_CloseReception(SPI_Hndl_t *pSPIHandle)
{
    pSPIHandle->pSPIx_StartAddr->CR2 &= ~(1U << SPI_CR2_RXNEIE);
    pSPIHandle->pRxBuffer = NULL;
    pSPIHandle->RxLen = 0U;
    pSPIHandle->RxState = SPI_READY;

}
