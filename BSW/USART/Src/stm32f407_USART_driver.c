/*
 * File: stm32f407_USART_driver.c
 *
 * Description:
 * This file contains the implementation of the USART driver for the
 * STM32F407 microcontroller.
 *
 * It provides functions to initialize and de-initialize USART peripherals,
 * configure peripheral clocks and the baud rate, read and clear status flags,
 * perform blocking (polling) and interrupt-driven data transfers, and handle
 * USART and NVIC interrupts.
 *
 * Supported frame formats: 8 or 9 bit words, parity (even/odd/none), 1, 0.5,
 * 2 or 1.5 stop bits, hardware flow control (CTS/RTS).
 *
 * @date    Nov 24, 2026
 * @author  Mohamed Ali, Lihiouel
 */

#include <stddef.h>
#include "stm32f407_USART_driver.h"
#include "stm32f407_RCC_driver.h"
#include "nvic_config.h"

/******************************************************************************
 * Private function prototypes
 ******************************************************************************/
static void USART_SetBaudRate(USART_Registers_t *pUSARTx, uint32_t BaudRate);
static void USART_SendNextFrameIT(USART_Hndl_t *pUSARTHandle);
static void USART_ReceiveNextFrameIT(USART_Hndl_t *pUSARTHandle);
static void USART_IRQ_TxComplete(USART_Hndl_t *pUSARTHandle);
static void USART_IRQ_TxEmpty(USART_Hndl_t *pUSARTHandle);
static void USART_IRQ_RxNotEmpty(USART_Hndl_t *pUSARTHandle);
static void USART_IRQ_Overrun(USART_Hndl_t *pUSARTHandle);
static void USART_IRQ_Error(USART_Hndl_t *pUSARTHandle);

/******************************************************************************
 * USART Initialisation and Deinitialisation
 ******************************************************************************/

/**
 * @brief  Sets the baud rate of the USART (register USART_BRR).
 *
 * @details
 * BRR holds USARTDIV as a fixed-point number (12 bit mantissa, 4 bit fraction):
 *
 *     Oversampling by 16 (OVER8 = 0): USARTDIV = PCLK / (16 * baud)
 *     Oversampling by 8  (OVER8 = 1): USARTDIV = PCLK / (8 * baud)
 *
 * To avoid floating point, USARTDIV is calculated multiplied by 100:
 *
 *     OVER8 = 0: usartDiv = 25 * PCLK / (4 * baud)   (= 100 * PCLK / (16 * baud))
 *     OVER8 = 1: usartDiv = 25 * PCLK / (2 * baud)   (= 100 * PCLK / (8 * baud))
 *
 * The mantissa is usartDiv / 100. The fraction is the remainder, scaled to
 * 16 steps (OVER8 = 0) or 8 steps (OVER8 = 1) and rounded to the nearest value.
 * If the rounding reaches 16 (or 8), the mantissa is incremented by 1.
 *
 * @param  pUSARTx:  Pointer to the USART peripheral registers.
 * @param  BaudRate: Baud rate in bit/s (must not be 0).
 * @retval None
 *
 * @note   Reads the OVER8 bit of CR1, so call this function AFTER CR1 has
 *         been configured.
 * @note   USART1 and USART6 are on APB2 (PCLK2), all other USART/UART
 *         peripherals on APB1 (PCLK1).
 */
static void USART_SetBaudRate(USART_Registers_t *pUSARTx, uint32_t BaudRate)
{
	// Variable to hold the APB clock
	uint32_t PCLKx;
	// Variables to hold Mantissa and Fraction values
	uint32_t M_part,F_part;

	uint32_t usartDiv;
	uint32_t tempReg=0;

	// Get the value of the APB bus clock into the variable PCLKx
	if(pUSARTx == USART1 || pUSARTx == USART6)
	{
	   // USART1 and USART6 are connected to the APB2 bus
	   PCLKx = RCC_GetPCLK2Value();
	}else
	{
	   PCLKx = RCC_GetPCLK1Value();
	}

	// Check the OVER8 configuration bit: oversampling by 8 or by 16
	if(pUSARTx->CR1 & (1 << USART_CR1_OVER8))
	{
	   // OVER8 = 1, oversampling by 8
	   usartDiv = ((25U * PCLKx) / (2U * BaudRate));
	}else
	{
	   // OVER8 = 0, oversampling by 16
	   usartDiv = ((25U * PCLKx) / (4U * BaudRate));
	}

	// Calculate the mantissa part
	M_part = usartDiv/100;

	// Extract the fraction part (remainder, still multiplied by 100)
	F_part = (usartDiv - (M_part * 100));

	// Calculate the final fraction, rounded to the nearest value.
	// If the rounding overflows (16 or 8), the mantissa is incremented.
	if(pUSARTx->CR1 & ( 1 << USART_CR1_OVER8))
	{
	  // OVER8 = 1, oversampling by 8: DIV_Fraction[2:0], bit 3 must be kept clear
	  F_part = ((( F_part * 8) + 50) / 100);
	  if(F_part > 0x07U)
	  {
		  M_part++;
	  }
	  F_part &= ((uint8_t)0x07);

	}else
	{
	  // OVER8 = 0, oversampling by 16: DIV_Fraction[3:0]
	  F_part = ((( F_part * 16) + 50) / 100);
	  if(F_part > 0x0FU)
	  {
		  M_part++;
	  }
	  F_part &= ((uint8_t)0x0F);
	}

	// Place the mantissa part in the appropriate bit position (see USART_BRR)
	tempReg |= M_part << USART_BRR_DIV_MANTISSA;

	// Place the fraction part in the appropriate bit position (see USART_BRR)
	tempReg |= F_part << USART_BRR_DIV_FRACTION;

	// Copy the value of tempReg into the BRR register
	pUSARTx->BRR = tempReg;
}

/**
 * @brief   Initializes the USART peripheral according to the settings
 *          provided in the USART_Hndl_t configuration structure.
 *
 * Supported configurations include:
 * - Transfer mode (TX only, RX only, TX and RX)
 * - Word length (8 or 9 bits)
 * - Parity control (disabled, even, odd)
 * - Number of stop bits
 * - Hardware flow control (none, CTS, RTS, CTS and RTS)
 * - Baud rate (oversampling by 16)
 *
 * @param   pUSARTHandle: Pointer to a USART handle structure (`USART_Hndl_t`)
 *                        containing the base address and user configuration settings.
 * @retval  None
 *
 * @note    The peripheral clock must be enabled before (USART_PeriClkCtrl()).
 * @note    CR1, CR2 and CR3 are overwritten. The USART is NOT enabled here
 *          (CR1.UE stays 0). Enable it afterwards with USART_PeripheralCtrl().
 */
void USART_Init(USART_Hndl_t *pUSARTHandle)
{
    uint32_t tempreg = 0U;


    // The peripheral clock must already be enabled (USART_PeriClkCtrl()).

    /********************************* Configuration of CR1*********************************/
    // 1) Transfer mode: enable the receiver (RE) and/or the transmitter (TE).
    if ( pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_RX)
    {
        // Receiver only
        tempreg|= (1 << USART_CR1_RE);
    }else if (pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_TX)
    {
        // Transmitter only
        tempreg |= ( 1 << USART_CR1_TE );

    }else if (pUSARTHandle->USART_Config.USART_Mode == USART_MODE_TXRX)
    {
        // Transmitter and receiver
        tempreg |= ( ( 1 << USART_CR1_RE) | ( 1 << USART_CR1_TE ) );
    }

    // 2) Word length (CR1.M): 0 = 8 bits, 1 = 9 bits (the parity bit is part of the word).
    tempreg |= pUSARTHandle->USART_Config.USART_WordLength << USART_CR1_M ;

    // 3) Parity control: PCE enables the parity, PS selects even (0) or odd (1).
    if ( pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_EVEN)
    {
        // Enable the parity control
        tempreg |= ( 1 << USART_CR1_PCE);

        // Even parity: nothing to do, PS = 0 selects even parity by default.

    }else if (pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_ODD )
    {
        // Enable the parity control
        tempreg |= ( 1 << USART_CR1_PCE);

        // Odd parity
        tempreg |= ( 1 << USART_CR1_PS);

    }

    // Program the CR1 register
    pUSARTHandle->pUSARTx_StartAddr->CR1 = tempreg;

    /********************************* Configuration of CR2*********************************/

    tempreg=0;

    // Number of stop bits (CR2.STOP[1:0]): 00 = 1, 01 = 0.5, 10 = 2, 11 = 1.5
    tempreg |= pUSARTHandle->USART_Config.USART_NoOfStopBits << USART_CR2_STOP;

    // Program the CR2 register
    pUSARTHandle->pUSARTx_StartAddr->CR2 = tempreg;

    /********************************* Configuration of CR3*********************************/

    tempreg=0;

    // Hardware flow control: CTSE (CTS) and RTSE (RTS) in CR3.
    if ( pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS)
    {
        // CTS flow control
        tempreg |= ( 1 << USART_CR3_CTSE);

    }else if (pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_RTS)
    {
        // RTS flow control
        tempreg |= ( 1 << USART_CR3_RTSE);

    }else if (pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS_RTS)
    {
        // CTS and RTS flow control
    	tempreg |= ( 1 << USART_CR3_RTSE) | ( 1 << USART_CR3_CTSE);
    }
    // Program the CR3 register
    pUSARTHandle->pUSARTx_StartAddr->CR3 = tempreg;

    /******************************** Configuration of BRR(Baudrate register)******************************************/

	// Baud rate: USART_SetBaudRate() reads CR1.OVER8, so it must be called
	// after CR1 has been written (see above).
	USART_SetBaudRate(pUSARTHandle->pUSARTx_StartAddr,pUSARTHandle->USART_Config.USART_BaudRate);
}


/**
 * @brief  De-initializes the specified USART peripheral, resetting all its registers to their default state.
 * @param  pUSARTx: Pointer to the base address of the target USART peripheral (e.g., USART1, USART2).
 * @retval None
 */
void USART_DeInit(USART_Registers_t *pUSARTx)
{
	switch ((uint32_t)pUSARTx)
	{
		case (uint32_t)USART1:
			RCC_USART1_RESET();
			break;
		case (uint32_t)USART2:
			RCC_USART2_RESET();
			break;
		case (uint32_t)USART3:
			RCC_USART3_RESET();
			break;
		case (uint32_t)USART6:
			RCC_USART6_RESET();
			break;
		default:
			break;
	}
}

/******************************************************************************
 * USART Peripheral Clock Control
 ******************************************************************************/

/**
 * @brief  Enables or disables the peripheral clock for the given USART peripheral.
 * @param  pUSARTx: Pointer to the base address of the target USART peripheral.
 * @param  EnOrDi: Macro for enabling or disabling the clock (e.g., ENABLE or DISABLE).
 * @retval None
 */
void USART_PeriClkCtrl(USART_Registers_t *pUSARTx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        switch ((uint32_t)pUSARTx)
        {
            case (uint32_t)USART1:
                RCC_USART1_CLK_ENABLE();
                break;
            case (uint32_t)USART2:
                RCC_USART2_CLK_ENABLE();
                break;
            case (uint32_t)USART3:
                RCC_USART3_CLK_ENABLE();
                break;
            case (uint32_t)USART6:
                RCC_USART6_CLK_ENABLE();
                break;
            default:
                break;
        }
    }
    else
    {
        switch ((uint32_t)pUSARTx)
        {
            case (uint32_t)USART1:
                RCC_USART1_CLK_DISABLE();
                break;
            case (uint32_t)USART2:
                RCC_USART2_CLK_DISABLE();
                break;
            case (uint32_t)USART3:
                RCC_USART3_CLK_DISABLE();
                break;
            case (uint32_t)USART6:
                RCC_USART6_CLK_DISABLE();
                break;
            default:
                break;
        }
    }
}

/******************************************************************************
 * USART Peripheral Control: Enable/Disable
 ******************************************************************************/

/**
 * @brief  Enables or disables the USART peripheral.
 *
 * @details
 * Controls the USART peripheral by setting or clearing the UE
 * (USART Enable) bit in the USART Control Register 1 (CR1).
 *
 * @param  pUSARTx: Pointer to the USART peripheral registers.
 * @param  EnOrDi:  Peripheral control command:
 *                  ENABLE  - Enable the USART peripheral.
 *                  DISABLE - Disable the USART peripheral.
 * @retval None
 *
 * @note   Nothing is transmitted or received while UE = 0. Call this function
 *         AFTER USART_Init(), because USART_Init() overwrites CR1.
 * @note   Disable the USART only after the last frame has been sent (TC = 1).
 */
void USART_PeripheralCtrl(USART_Registers_t *pUSARTx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        /* Set the UE bit to enable the USART peripheral. */
        pUSARTx->CR1 |= (1U << USART_CR1_UE);
    }
    else
    {
        /* Clear the UE bit to disable the USART peripheral. */
        pUSARTx->CR1 &= ~(1U << USART_CR1_UE);
    }
}

/******************************************************************************
 * USART Send and Receive
 ******************************************************************************/

/**
 * @brief  Returns the state of a flag in the status register USART_SR.
 * @param  pUSARTx:    Pointer to the USART peripheral registers.
 * @param  FlagMask: Flag mask (e.g. USART_FLAG_TXE), NOT the bit position.
 * @retval USART_FLAG_SET or USART_FLAG_RESET
 */
uint8_t USART_GetFlagStatus(const USART_Registers_t *pUSARTx, uint32_t FlagMask)
{
    if ((pUSARTx->SR & FlagMask) != 0U)
    {
        return USART_FLAG_SET;
    }
    return USART_FLAG_RESET;
}

/**
 * @brief  Sends data over the USART (blocking / polling).
 *
 * @details
 * For every frame the function waits until the transmit data register is empty
 * (TXE = 1) and then writes the frame to DR. After the last frame it waits for
 * TC = 1, so all frames have completely left the shift register when the
 * function returns.
 *
 * Frame handling depends on the configured word length and parity:
 * - 8 bit word:                    1 byte of the buffer per frame.
 * - 9 bit word with parity:        1 byte of the buffer per frame (8 data bits,
 *                                  the 9th bit is replaced by the parity bit).
 * - 9 bit word without parity:     2 bytes of the buffer per frame (9 data bits,
 *                                  bits 15:9 of the 16-bit value are ignored).
 *
 * @param  pUSARTHandle: Pointer to the USART handle (peripheral and configuration).
 * @param  pTxBuffer:    Pointer to the data to send.
 * @param  Length:       Number of FRAMES to send (not bytes). With 9 data bits
 *                       and no parity the buffer must hold 2 * Length bytes.
 * @retval None
 *
 * @note   The function waits without timeout. No frame is transmitted if the
 *         USART is not enabled (CR1.UE = 0) or the transmitter is off
 *         (CR1.TE = 0), and the function may then block forever.
 * @note   With 9 data bits and no parity pTxBuffer must be 2-byte aligned,
 *         because it is read as 16-bit values.
 */
void USART_SendData(USART_Hndl_t *pUSARTHandle, const uint8_t *pTxBuffer, uint32_t Length)
{
	const uint16_t *pData;   // pTxBuffer viewed as 16-bit values (9 bit frames)

	// Loop until "Length" frames are transferred
	for(uint32_t i = 0 ; i < Length; i++)
	{
		// Wait until the transmit data register is empty (TXE = 1)
		while(! USART_GetFlagStatus(pUSARTHandle->pUSARTx_StartAddr,USART_FLAG_TXE));

		// Check the word length: 9 bit or 8 bit per frame
		if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
		{
			// 9 bit: load DR with 2 bytes, masked to the first 9 bits (writing DR also clears TXE)
			pData = (const uint16_t*) pTxBuffer;
			pUSARTHandle->pUSARTx_StartAddr->DR = (*pData & (uint16_t)0x01FF);

			// Check the parity control
			if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
			{
				// No parity: all 9 bits are user data -> advance the buffer by 2 bytes
				pTxBuffer++;
				pTxBuffer++;
			}
			else
			{
				// Parity: 8 bits of user data, the 9th bit is replaced by the parity bit
				// by the hardware -> advance the buffer by 1 byte
				pTxBuffer++;
			}
		}
		else
		{
			// 8 bit: load DR with one byte (writing DR also clears TXE)
			pUSARTHandle->pUSARTx_StartAddr->DR = (*pTxBuffer  & (uint8_t)0xFF);

			// Advance the buffer by 1 byte
			pTxBuffer++;
		}
	}

	// Wait until the last frame has completely been sent (TC = 1)
	while( ! USART_GetFlagStatus(pUSARTHandle->pUSARTx_StartAddr,USART_FLAG_TC));
}

/**
 * @brief  Receives data over the USART (blocking / polling).
 *
 * @details
 * For every frame the function waits until the receive data register is not
 * empty (RXNE = 1) and then reads the frame from DR (reading DR clears RXNE).
 *
 * Frame handling depends on the configured word length and parity:
 * - 8 bit word, no parity:       1 byte of the buffer per frame (8 data bits).
 * - 8 bit word with parity:      1 byte of the buffer per frame (7 data bits,
 *                                the 8th bit is the parity bit and is masked out).
 * - 9 bit word with parity:      1 byte of the buffer per frame (8 data bits,
 *                                the 9th bit is the parity bit and is masked out).
 * - 9 bit word without parity:   2 bytes of the buffer per frame (9 data bits,
 *                                bits 15:9 of the 16-bit value are cleared).
 *
 * @param  pUSARTHandle: Pointer to the USART handle (peripheral and configuration).
 * @param  pRxBuffer:    Buffer for the received data.
 * @param  Length:       Number of FRAMES to receive (not bytes). With 9 data bits
 *                       and no parity the buffer must hold 2 * Length bytes.
 * @retval None
 *
 * @note   The function waits without timeout. It blocks forever if no data
 *         arrives or the receiver is off (CR1.RE = 0 or CR1.UE = 0).
 * @note   Error flags (PE, FE, NF, ORE) are not checked. In polling mode DR
 *         holds only one frame, so read each frame before the next one arrives
 *         (otherwise ORE = 1 and data is lost).
 * @note   With 9 data bits and no parity pRxBuffer must be 2-byte aligned,
 *         because it is written as 16-bit values.
 */
void USART_ReceiveData(USART_Hndl_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Length)
{
	for(uint32_t i = 0 ; i < Length; i++)
	{
		// Wait until a frame has been received (RXNE = 1)
		while(! USART_GetFlagStatus(pUSARTHandle->pUSARTx_StartAddr,USART_FLAG_RXNE));

		// Check the word length: 9 bit or 8 bit per frame
		if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
		{
			// Check the parity control
			if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
			{
				// 9 bit, no parity: all 9 bits are user data -> read DR as 16-bit value,
				// masked to the first 9 bits (reading DR clears RXNE)
				*((uint16_t*)pRxBuffer) = (pUSARTHandle->pUSARTx_StartAddr->DR & (uint16_t)0x01FF);

				// Advance the buffer by 2 bytes
				pRxBuffer++;
				pRxBuffer++;
			}
			else
			{
				// 9 bit with parity: 8 bits of user data, the 9th bit is the parity bit
				// -> mask it out with 0xFF and advance the buffer by 1 byte
				*pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx_StartAddr->DR & (uint8_t)0xFF);
				pRxBuffer++;
			}
		}
		else
		{
			// Check the parity control
			if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
			{
				// 8 bit, no parity: all 8 bits are user data
				// -> read one byte from DR and advance the buffer by 1 byte
				*pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx_StartAddr->DR & (uint8_t)0xFF);
				pRxBuffer++;
			}
			else
			{
				// 8 bit with parity: 7 bits of user data, the 8th bit is the parity bit
				// -> mask it out with 0x7F and advance the buffer by 1 byte
				*pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx_StartAddr->DR & (uint8_t)0x7F);
				pRxBuffer++;
			}
		}
	}
}

/**
 * @brief  Starts a transmission in interrupt mode (non-blocking).
 *
 * @details
 * Stores the transmit parameters in the handle and enables the TXE and TC
 * interrupts. The first TXE interrupt follows at once, because the data
 * register is empty. Everything else is done in USART_IRQHandling().
 * When all bytes are sent, USART_ApplicationEventCallback() is called with
 * USART_EVENT_TX_CMPLT.
 *
 * @param  pUSARTHandle: Pointer to the USART handle.
 * @param  pTxBuffer:    Pointer to the data to send. Must stay valid until the
 *                       transmission is finished.
 * @param  Length:       Number of bytes to send.
 * @retval Previous transmit state: USART_READY = transmission started,
 *         USART_BUSY_IN_TX = a transmission is already running, nothing was done.
 *
 * @note   With 9 data bits and no parity Length must be even (2 bytes per frame).
 */
uint8_t USART_SendDataIT(USART_Hndl_t *pUSARTHandle,uint8_t *pTxBuffer, uint32_t Length)
{
	uint8_t txstate = pUSARTHandle->TxBusyState;

	if(txstate != USART_BUSY_IN_TX)
	{
		// Store the transfer parameters in the handle (used by the ISR).
		pUSARTHandle->TxLen = Length;
		pUSARTHandle->pTxBuffer = pTxBuffer;
		pUSARTHandle->TxBusyState = USART_BUSY_IN_TX;

		// Enable the TXE interrupt: the first byte is sent in the ISR
		pUSARTHandle->pUSARTx_StartAddr->CR1 |= ( 1 << USART_CR1_TXEIE);


		// Enable the TC interrupt: it reports the end of the transmission
		pUSARTHandle->pUSARTx_StartAddr->CR1 |= ( 1 << USART_CR1_TCIE);
	}

	return txstate;
}


/**
 * @brief  Starts a reception in interrupt mode (non-blocking).
 *
 * @details
 * Stores the receive parameters in the handle, discards an old byte in the
 * data register and enables the RXNE interrupt. The bytes are stored in
 * USART_IRQHandling(). When all bytes are received,
 * USART_ApplicationEventCallback() is called with USART_EVENT_RX_CMPLT.
 *
 * @param  pUSARTHandle: Pointer to the USART handle.
 * @param  pRxBuffer:    Buffer for the received bytes.
 * @param  Length:       Number of bytes to receive.
 * @retval Previous receive state: USART_READY = reception started,
 *         USART_BUSY_IN_RX = a reception is already running, nothing was done.
 *
 * @note   Call this function BEFORE the sender starts, otherwise the first
 *         bytes can be lost.
 * @note   With 9 data bits and no parity Length must be even (2 bytes per frame).
 */
uint8_t USART_ReceiveDataIT(USART_Hndl_t *pUSARTHandle,uint8_t *pRxBuffer, uint32_t Length)
{
	uint8_t rxstate = pUSARTHandle->RxBusyState;

	if(rxstate != USART_BUSY_IN_RX)
	{
		// Store the transfer parameters in the handle (used by the ISR).
		pUSARTHandle->RxLen = Length;
		pUSARTHandle->pRxBuffer = pRxBuffer;
		pUSARTHandle->RxBusyState = USART_BUSY_IN_RX;

		// Discard an old byte (clears a pending RXNE flag).
		(void)pUSARTHandle->pUSARTx_StartAddr->DR;

		// Enable the RXNE interrupt
		pUSARTHandle->pUSARTx_StartAddr->CR1 |= ( 1 << USART_CR1_RXNEIE);

	}

	return rxstate;
}


/******************************************************************************
 * USART Interrupt Configuration and Handling
 ******************************************************************************/

/**
 * @brief  Enables or disables an USART interrupt line in the NVIC.
 *
 * @details
 * Writes 1 to the bit of the selected interrupt in the NVIC Interrupt
 * Set-Enable Registers (ISERx) to enable it, or in the Interrupt
 * Clear-Enable Registers (ICERx) to disable it.
 * Each register covers 32 interrupts:
 * - IRQ  0..31 : ISER0 / ICER0
 * - IRQ 32..63 : ISER1 / ICER1 (e.g. USART1 = 37, USART2 = 38, USART3 = 39)
 * - IRQ 64..95 : ISER2 / ICER2
 *
 * @param  IRQNumber: The Interrupt Request number in the NVIC
 *                    (e.g. IRQ_NO_USART2).
 * @param  EnOrDi:    Macro to enable or disable the interrupt (ENABLE or DISABLE).
 * @retval None
 *
 * @note   This only enables the line in the NVIC. The peripheral itself
 *         raises interrupts only if the interrupt enable bits in CR1 (TXEIE,
 *         TCIE, RXNEIE) or CR3 (EIE) are set.
 */
void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(IRQNumber<32)
		{
			// NVIC_ISER0
			*NVIC_ISER0 |= (1U << IRQNumber);
		}
		else if(IRQNumber>=32 && (IRQNumber<64))
		{
			// NVIC_ISER1
			*NVIC_ISER1 |= (1U << (IRQNumber%32));
		}
		else if(IRQNumber>=64 && (IRQNumber<96))
		{
			// NVIC_ISER2
			*NVIC_ISER2 |= (1U << (IRQNumber%32));
		}
	}
	else if(EnOrDi == DISABLE)
	{
		if(IRQNumber<32)
		{
			// NVIC_ICER0
			*NVIC_ICER0 |= (1U << IRQNumber);
		}
		else if(IRQNumber>=32 && (IRQNumber<64))
		{
			// NVIC_ICER1
			*NVIC_ICER1 |= (1U << (IRQNumber%32));
		}
		else if(IRQNumber>=64 && (IRQNumber<96))
		{
			// NVIC_ICER2
			*NVIC_ICER2 |= (1U << (IRQNumber%32));
		}
	}
}

/**
 * @brief  Configures the priority of an USART interrupt in the NVIC.
 *
 * @details
 * Each NVIC_IPRx register holds four 8-bit priority fields. The STM32F4
 * implements only the upper 4 bits of each field, so the priority value is
 * shifted left by NUM_PR_BITS_NOT_IMPLEMENTED (4) bits.
 *
 * @param  IRQNumber:   The Interrupt Request number in the NVIC.
 * @param  IRQPriority: Priority value (0..15, lower value = higher priority).
 * @retval None
 */
void USART_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority)
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


/******************************************************************************
 * Private helper functions
 ******************************************************************************/

/**
 * @brief  Writes the next frame of the transmit buffer to DR (interrupt mode).
 *
 * @details
 * - 8 bit word:                1 byte per frame.
 * - 9 bit word with parity:    1 byte per frame (the 9th bit is replaced by
 *                              the parity bit by the hardware).
 * - 9 bit word without parity: 2 bytes per frame (9 data bits).
 * The buffer pointer and the remaining length (TxLen, in bytes) are updated.
 *
 * @param  pUSARTHandle: Pointer to the USART handle.
 * @retval None
 *
 * @note   With 9 data bits and no parity TxLen must be even and pTxBuffer
 *         must be 2-byte aligned.
 */
static void USART_SendNextFrameIT(USART_Hndl_t *pUSARTHandle)
{
	uint16_t *pdata;

	// Check the USART_WordLength item for 9BIT or 8BIT in a frame
	if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
	{
		// 9BIT: load DR with 2 bytes, masked to the first 9 bits
		pdata = (uint16_t*) pUSARTHandle->pTxBuffer;
		pUSARTHandle->pUSARTx_StartAddr->DR = (*pdata & (uint16_t)0x01FF);

		// Check the parity control
		if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
		{
			// No parity: 9 bits of user data -> advance the buffer by 2 bytes
			pUSARTHandle->pTxBuffer++;
			pUSARTHandle->pTxBuffer++;
			pUSARTHandle->TxLen-=2;
		}
		else
		{
			// Parity: 8 bits of user data, the 9th bit is replaced by the parity bit
			pUSARTHandle->pTxBuffer++;
			pUSARTHandle->TxLen-=1;
		}
	}
	else
	{
		// 8BIT data transfer
		pUSARTHandle->pUSARTx_StartAddr->DR = (*pUSARTHandle->pTxBuffer  & (uint8_t)0xFF);

		// Advance the buffer by 1 byte
		pUSARTHandle->pTxBuffer++;
		pUSARTHandle->TxLen-=1;
	}
}

/**
 * @brief  Reads the next frame from DR into the receive buffer (interrupt mode).
 *
 * @details
 * - 9 bit word without parity: 9 data bits, stored as 16-bit value (2 bytes).
 * - 9 bit word with parity:    8 data bits (the 9th bit is the parity bit).
 * - 8 bit word without parity: 8 data bits.
 * - 8 bit word with parity:    7 data bits (bit 7 is the parity bit).
 * The buffer pointer and the remaining length (RxLen, in bytes) are updated.
 *
 * @param  pUSARTHandle: Pointer to the USART handle.
 * @retval None
 *
 * @note   With 9 data bits and no parity RxLen must be even and pRxBuffer
 *         must be 2-byte aligned.
 */
static void USART_ReceiveNextFrameIT(USART_Hndl_t *pUSARTHandle)
{
	// Check the USART_WordLength: 9 bit or 8 bit per frame
	if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
	{
		// 9BIT frame: check whether parity is used
		if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
		{
			// No parity: all 9 bits are user data, mask DR with 0x01FF
			*((uint16_t*) pUSARTHandle->pRxBuffer) = (pUSARTHandle->pUSARTx_StartAddr->DR  & (uint16_t)0x01FF);

			// Advance the buffer by 2 bytes
			pUSARTHandle->pRxBuffer++;
			pUSARTHandle->pRxBuffer++;
			pUSARTHandle->RxLen-=2;
		}
		else
		{
			// Parity: 8 bits of user data, 1 bit is parity
			 *pUSARTHandle->pRxBuffer = (pUSARTHandle->pUSARTx_StartAddr->DR  & (uint8_t)0xFF);
			 pUSARTHandle->pRxBuffer++;
			 pUSARTHandle->RxLen-=1;
		}
	}
	else
	{
		// 8BIT frame: check whether parity is used
		if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
		{
			// No parity: all 8 bits are user data
			 *pUSARTHandle->pRxBuffer = (uint8_t) (pUSARTHandle->pUSARTx_StartAddr->DR  & (uint8_t)0xFF);
		}
		else
		{
			// Parity: 7 bits of user data, mask DR with 0x7F
			 *pUSARTHandle->pRxBuffer = (uint8_t) (pUSARTHandle->pUSARTx_StartAddr->DR  & (uint8_t)0x7F);
		}

		// Advance the buffer by 1 byte
		pUSARTHandle->pRxBuffer++;
		 pUSARTHandle->RxLen-=1;
	}
}

/**
 * @brief  Handles the TC interrupt: closes the transmission.
 *
 * @details
 * If a transmission is running and all bytes are sent (TxLen = 0), TC and TCIE
 * are cleared, the handle is reset and USART_EVENT_TX_CMPLT is reported.
 *
 * @param  pUSARTHandle: Pointer to the USART handle.
 * @retval None
 */
static void USART_IRQ_TxComplete(USART_Hndl_t *pUSARTHandle)
{
	uint32_t temp1, temp2;

	// State of the TC bit in SR and of the TCIE bit in CR1
	temp1 = pUSARTHandle->pUSARTx_StartAddr->SR & ( 1 << USART_SR_TC);
	temp2 = pUSARTHandle->pUSARTx_StartAddr->CR1 & ( 1 << USART_CR1_TCIE);

	if(temp1 && temp2 )
	{
		// This interrupt is because of TC.
		// Close the transmission and call the application callback if TxLen is zero.
		if ( pUSARTHandle->TxBusyState == USART_BUSY_IN_TX)
		{
			// Check the TxLen. If it is zero then close the data transmission
			if(! pUSARTHandle->TxLen )
			{
				// Clear the TC flag by writing 0 to it. Writing 1 to the other bits has no effect,
				// so RXNE cannot be cleared by accident (a read-modify-write could do that).
				pUSARTHandle->pUSARTx_StartAddr->SR = ~( 1U << USART_SR_TC);

				// Clear the TCIE control bit
				pUSARTHandle->pUSARTx_StartAddr->CR1 &= ~( 1 << USART_CR1_TCIE);

				// Reset the application state
				pUSARTHandle->TxBusyState = USART_READY;

				// Reset the buffer address to NULL
				pUSARTHandle->pTxBuffer = NULL;

				// Reset the length to zero
				pUSARTHandle->TxLen = 0;

				// Call the application callback with event USART_EVENT_TX_CMPLT
				USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_TX_CMPLT);
			}
		}
	}
}

/**
 * @brief  Handles the TXE interrupt: sends the next frame.
 *
 * @details
 * Sends frames until TxLen reaches zero, then disables the TXE interrupt.
 * The end of the transmission is detected by the TC interrupt.
 *
 * @param  pUSARTHandle: Pointer to the USART handle.
 * @retval None
 */
static void USART_IRQ_TxEmpty(USART_Hndl_t *pUSARTHandle)
{
	uint32_t temp1, temp2;

	// State of the TXE bit in SR and of the TXEIE bit in CR1
	temp1 = pUSARTHandle->pUSARTx_StartAddr->SR & ( 1 << USART_SR_TXE);
	temp2 = pUSARTHandle->pUSARTx_StartAddr->CR1 & ( 1 << USART_CR1_TXEIE);

	if(temp1 && temp2 )
	{
		// This interrupt is because of TXE
		if(pUSARTHandle->TxBusyState == USART_BUSY_IN_TX)
		{
			// Keep sending data until TxLen reaches zero
			if(pUSARTHandle->TxLen > 0)
			{
				USART_SendNextFrameIT(pUSARTHandle);
			}
			if (pUSARTHandle->TxLen == 0 )
			{
				// TxLen is zero: disable the interrupt for the TXE flag
				pUSARTHandle->pUSARTx_StartAddr->CR1 &= ~( 1 << USART_CR1_TXEIE);
			}
		}
	}
}

/**
 * @brief  Handles the RXNE interrupt: stores the received frame.
 *
 * @details
 * Reads frames until RxLen reaches zero, then disables the RXNE interrupt,
 * resets the state and reports USART_EVENT_RX_CMPLT.
 *
 * @param  pUSARTHandle: Pointer to the USART handle.
 * @retval None
 */
static void USART_IRQ_RxNotEmpty(USART_Hndl_t *pUSARTHandle)
{
	uint32_t temp1, temp2;

	// State of the RXNE bit in SR and of the RXNEIE bit in CR1
	temp1 = pUSARTHandle->pUSARTx_StartAddr->SR & ( 1 << USART_SR_RXNE);
	temp2 = pUSARTHandle->pUSARTx_StartAddr->CR1 & ( 1 << USART_CR1_RXNEIE);

	if(temp1 && temp2 )
	{
		// This interrupt is because of RXNE
		if(pUSARTHandle->RxBusyState == USART_BUSY_IN_RX)
		{
			if(pUSARTHandle->RxLen > 0)
			{
				USART_ReceiveNextFrameIT(pUSARTHandle);
			}

			if(! pUSARTHandle->RxLen)
			{
				// All data received: disable RXNE interrupt, reset the state, report
				pUSARTHandle->pUSARTx_StartAddr->CR1 &= ~( 1 << USART_CR1_RXNEIE );
				pUSARTHandle->RxBusyState = USART_READY;
				USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_RX_CMPLT);
			}
		}
	}
}

/**
 * @brief  Handles the overrun error (ORE) in combination with the RXNE interrupt.
 *
 * @details
 * ORE is set when a byte arrives while the previous byte has not been read
 * from DR. The new byte is lost and USART_ERR_ORE is reported.
 *
 * ORE is cleared by the sequence "read SR, then read DR", which
 * USART_IRQ_RxNotEmpty() performs when it reads the byte. Therefore this
 * handler must be called BEFORE USART_IRQ_RxNotEmpty(), otherwise the flag is
 * already cleared and the error is not reported.
 *
 * @param  pUSARTHandle: Pointer to the USART handle.
 * @retval None
 */
static void USART_IRQ_Overrun(USART_Hndl_t *pUSARTHandle)
{
	uint32_t temp1, temp2;

	// State of the ORE bit in SR and of the RXNEIE bit in CR1
	temp1 = pUSARTHandle->pUSARTx_StartAddr->SR & ( 1 << USART_SR_ORE);
	temp2 = pUSARTHandle->pUSARTx_StartAddr->CR1 & ( 1 << USART_CR1_RXNEIE);

	if(temp1  && temp2 )
	{
		// Do not clear ORE here: USART_IRQ_RxNotEmpty() reads SR and DR afterwards, which clears it.
		// This interrupt is because of an overrun error.
		USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_ORE);
	}
}

/**
 * @brief  Handles the error interrupt (CR3.EIE): FE, NF and ORE.
 *
 * @details
 * Reports the error flags to the application. The flags are NOT cleared here.
 * Clear them with USART_ClearFlag() (sequence: read SR, then read DR).
 * The EIE interrupt only occurs in multibuffer communication (DMA). Without
 * DMA the same errors are reported together with RXNE.
 *
 * @param  pUSARTHandle: Pointer to the USART handle.
 * @retval None
 */
static void USART_IRQ_Error(USART_Hndl_t *pUSARTHandle)
{
	uint32_t temp1, temp2;

	// State of the EIE bit in CR3
	temp2 =  pUSARTHandle->pUSARTx_StartAddr->CR3 & ( 1 << USART_CR3_EIE) ;

	if(temp2 )
	{
		temp1 = pUSARTHandle->pUSARTx_StartAddr->SR;
		if(temp1 & ( 1 << USART_SR_FE))
		{
			// Framing error: de-synchronization, excessive noise or a break character
			USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_FE);
		}

		if(temp1 & ( 1 << USART_SR_NF) )
		{
			// Noise detected on a received frame
			USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_NE);
		}

		if(temp1 & ( 1 << USART_SR_ORE) )
		{
			// Overrun error
			USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_ORE);
		}
	}
}

/******************************************************************************
 * USART Interrupt Handling
 ******************************************************************************/

/**
 * @brief  Handles the USART interrupt (TC, TXE, ORE, RXNE and error flags).
 *
 * @details
 * Calls the handler of each interrupt source in this order:
 * TC, TXE, ORE, RXNE and the error flags (CR3.EIE).
 * ORE is checked before RXNE, because reading the received byte clears ORE.
 *
 * @param  pUSARTHandle: Pointer to the USART handle of the peripheral that
 *                       raised the interrupt.
 * @retval None
 *
 * @note   Call this function from the USARTx_IRQHandler() of the application.
 */
void USART_IRQHandling(USART_Hndl_t *pUSARTHandle)
{
	USART_IRQ_TxComplete(pUSARTHandle);   // TC flag
	USART_IRQ_TxEmpty(pUSARTHandle);      // TXE flag
	USART_IRQ_Overrun(pUSARTHandle);      // ORE flag
	USART_IRQ_RxNotEmpty(pUSARTHandle);   // RXNE flag
	USART_IRQ_Error(pUSARTHandle);        // FE, NF, ORE (EIE)
}

/**
 * @brief  Application event callback (weak default implementation).
 *
 * @details
 * Called by USART_IRQHandling() when a transfer is complete or an error
 * occurred. The application overrides this function by defining a function
 * with the same name (see the USART test applications).
 *
 * @param  pUSARTHandle: Pointer to the USART handle that reported the event.
 * @param  Event:        USART_EVENT_xxx or USART_ERR_xxx code.
 * @retval None
 */
__attribute__((weak)) void USART_ApplicationEventCallback(USART_Hndl_t *pUSARTHandle,uint8_t Event)
{
	(void)pUSARTHandle;
	(void)Event;
}
