/*
 * File: stm32f407_I2C_driver.c
 *
 * Description:
 * This file contains the implementation of the I2C driver for the
 * STM32F407 microcontroller.
 *
 * It provides functions to initialize and de-initialize I2C peripherals,
 * configure peripheral clocks, control ACK and peripheral states,
 * perform blocking (polling) and non-blocking (interrupt-driven) master
 * and slave data transfers, and handle I2C event, error, and NVIC interrupts.
 *
 * Supported transfer directions:
 * - Master transmitter (polling and interrupt)
 * - Slave receiver (interrupt)
 *
 * @date    Nov 21, 2026
 * @author  Mohamed Ali, Lihiouel
 */

#include "stm32f407_I2C_driver.h"
#include "nvic_config.h"

/******************************************************************************
 * Private macros and constant tables
 ******************************************************************************/

#define I2C_HSI_CLK_HZ   (16000000U)   /* HSI: 16 MHz                          */
#define I2C_HSE_CLK_HZ   (8000000U)    /* HSE: 8 MHz auf dem Discovery Board   */

/* AHB prescaler table for HPRE values 8..15 (4 = /2 ... 15 = /512). */
static const uint16_t arrayAhbPresc[8] = {2U, 4U, 8U, 16U, 64U, 128U, 256U, 512U};
/* APB prescaler table for PPRE values 4..7 (/2, /4, /8, /16). */
static const uint8_t  arrayApbPresc[4] = {2U, 4U, 8U, 16U};

/******************************************************************************
 * Private function prototypes
 ******************************************************************************/
static uint32_t I2C_GetPCLK1Value(void);
static void I2C_CloseSendData(I2C_Hndl_t *pI2CHandle);

/******************************************************************************
 * Private helper functions
 ******************************************************************************/

/**
 * @brief  Calculates the APB1 peripheral clock frequency (PCLK1) in Hz.
 *
 * @details
 * PCLK1 feeds the I2C peripherals. It is derived from the system clock
 * source (RCC_CFGR.SWS), the AHB prescaler (HPRE) and the APB1 prescaler
 * (PPRE1):
 *
 *     PCLK1 = SYSCLK / AHB prescaler / APB1 prescaler
 *
 * The result is used by I2C_Init() to program CR2.FREQ, CCR and TRISE.
 *
 * @param  None
 * @retval PCLK1 frequency in Hz.
 *
 * @note   Only HSI (16 MHz) and HSE (8 MHz) are supported. If the PLL is the
 *         system clock source (SWS = 2) the system clock stays 0 (still tbd).
 */
static uint32_t I2C_GetPCLK1Value(void)
{
	uint32_t tempPCKL1 = 0, sysClk = 0;
	uint8_t tempAhbPresc = 0, tempApbPresc = 0;
	uint8_t ahbPresc = 0, apbPresc = 0;
	// Bits 3:2 SWS: system clock switch status (0 = HSI, 1 = HSE, 2 = PLL)
	uint8_t clkSrc = ((RCC->CFGR >> 2U) & 0x3);

	if(clkSrc == 0U) // HSI oscillator used as the system clock
	{
		sysClk = I2C_HSI_CLK_HZ; // 16 MHZ
	}
	else if(clkSrc == 1U) // HSE oscillator used as the system clock
	{
		sysClk = I2C_HSE_CLK_HZ; // 8 MHZ in STM32 Discovery Board
	}
	else if(clkSrc == 2U) // PLL used as the system clock
	{
		// tbd
	}

	// Bits 7:4 HPRE: AHB prescaler
	tempAhbPresc = ((RCC->CFGR >> 4U) & 0xF);
	if(tempAhbPresc<8)
	{
		ahbPresc = 1;
	}
	else
	{
		ahbPresc = arrayAhbPresc[tempAhbPresc-8U];
	}

	// Bits 12:10 PPRE1: APB Low speed prescaler (APB1)
	tempApbPresc = ((RCC->CFGR >> 10U) & 0x7);
	if(tempApbPresc<4)
	{
		apbPresc = 1;
	}
	else
	{
		apbPresc = arrayApbPresc[tempApbPresc-4U];
	}

	// PCLK1 = SYSCLK / AHB prescaler / APB1 prescaler
	tempPCKL1 = (uint32_t)((sysClk/ahbPresc) / apbPresc);

	return tempPCKL1;
}

/******************************************************************************
 * I2C Initialisation and Deinitialisation
 ******************************************************************************/

/**
 * @brief   Initializes the I2C peripheral according to the settings
 *          provided in the I2C_Hndl_t configuration structure.
 *
 * Supported configurations include:
 * - Peripheral clock frequency (CR2.FREQ, derived from PCLK1)
 * - Own 7-bit device address (OAR1)
 * - SCL speed: standard mode (up to 100 kHz) or fast mode (up to 400 kHz)
 * - Fast-mode duty cycle (Tlow/Thigh = 2 or 16/9)
 * - Maximum rise time (TRISE)
 * - Acknowledge control (CR1.ACK)
 *
 * @param   pI2CHandle: Pointer to an I2C handle structure (`I2C_Hndl_t`)
 *                      containing the base address and user configuration settings.
 * @retval  None
 *
 * @note    I2C_PeriClkCtrl() must be called before I2C_Init().
 * @note    The peripheral must be disabled (PE = 0) while CCR and TRISE are
 *          written. I2C_Init() therefore has to run BEFORE I2C_PeripheralCtrl().
 * @note    Hardware clears CR1.ACK while PE = 0. The ACK setting written here
 *          is lost when the peripheral is enabled later. Call
 *          I2C_ManageAcking(pI2Cx, ENABLE) after I2C_PeripheralCtrl().
 * @note    CCR formulas (Thigh = Tlow in standard mode):
 *          - Standard mode:  CCR = PCLK1 / (2 * SCL speed)
 *          - Fast, duty 2:   CCR = PCLK1 / (3 * SCL speed)
 *          - Fast, duty 16/9: CCR = PCLK1 / (25 * SCL speed)
 * @note    TRISE = (maximum rise time / Tpclk1) + 1
 *          (standard: 1000 ns -> FREQ + 1, fast: 300 ns -> FREQ * 300 / 1000 + 1).
 */
void I2C_Init(I2C_Hndl_t *pI2CHandle)
{
	uint32_t tempReg = 0U;
	uint32_t pclk1   = I2C_GetPCLK1Value();
	uint32_t freqMHz = pclk1 / 1000000U;
	/* Local copies: PCLK1 in Hz, PCLK1 in MHz (CR2.FREQ) and the requested SCL speed. */
	uint32_t speed   = pI2CHandle->I2C_Config.I2C_SCLSpeed;   /* see note below */
	uint16_t ccr     = 0U;

	/* 1. Peripheral clock frequency in MHz (CR2.FREQ[5:0], valid 2..42). */
	pI2CHandle->pI2Cx_StartAddr->CR2 = (uint16_t)(freqMHz & 0x3FU);

	/* 2. Own address (7 bit) in ADD[7:1]; bit 14 must be kept at 1. */
	tempReg  = ((uint32_t)pI2CHandle->I2C_Config.I2C_DeviceAddress << I2C_OAR1_ADD71);
	tempReg |= (1U << I2C_OAR1_BIT14);
	pI2CHandle->pI2Cx_StartAddr->OAR1 = (uint16_t)tempReg;

	/* 3. CCR: standard mode (FS = 0) or fast mode (FS = 1) + duty cycle. */
	tempReg = 0U;
	if (speed <= I2C_SCL_SPEED_STANDARD_MODE)
	{
		ccr = (uint16_t)(pclk1 / (2U * speed));          /* Thigh = Tlow = CCR*Tpclk */
	}
	else
	{
		tempReg |= (1U << I2C_CCR_FS);
		tempReg |= ((uint32_t)pI2CHandle->I2C_Config.I2C_DMDutyCycle << I2C_CCR_DUTY);
		if (pI2CHandle->I2C_Config.I2C_DMDutyCycle == I2C_DUTY_CYCLE_2)
		{
			ccr = (uint16_t)(pclk1 / (3U * speed));      /* Tlow = 2 * Thigh  */
		}
		else
		{
			ccr = (uint16_t)(pclk1 / (25U * speed));     /* Tlow/Thigh = 16/9 */
		}
	}
	tempReg |= (ccr & 0x0FFFU);
	pI2CHandle->pI2Cx_StartAddr->CCR = (uint16_t)tempReg;

	/* 4. TRISE: max. rise time 1000 ns (standard) / 300 ns (fast), in Tpclk + 1. */
	if (speed <= I2C_SCL_SPEED_STANDARD_MODE)
	{
		tempReg = freqMHz + 1U;
	}
	else
	{
		tempReg = ((freqMHz * 300U) / 1000U) + 1U;
	}
	pI2CHandle->pI2Cx_StartAddr->TRISE = (uint16_t)(tempReg & 0x3FU);

	/* 5. ACK control (CR1.ACK). */
	pI2CHandle->pI2Cx_StartAddr->CR1 &= (uint16_t)~(1U << I2C_CR1_ACK);
	pI2CHandle->pI2Cx_StartAddr->CR1 |= (uint16_t)((uint32_t)pI2CHandle->I2C_Config.I2C_ACKControl << I2C_CR1_ACK);
}

/**
 * @brief  De-initializes the specified I2C peripheral.
 *
 * @details
 * Pulses the reset line of the selected I2C peripheral in the RCC
 * (APB1RSTR), which returns all its registers to their reset values.
 *
 * @param  pI2Cx: Pointer to the base address of the target I2C peripheral
 *                (I2C1, I2C2 or I2C3).
 * @retval None
 */
void I2C_DeInit(I2C_Registers_t *pI2Cx)
{
	// Select the reset macro that belongs to this peripheral.
	switch ((uint32_t)pI2Cx)
	{
		case (uint32_t)I2C1:
			RCC_I2C1_RESET();
			break;
		case (uint32_t)I2C2:
			RCC_I2C2_RESET();
			break;
		case (uint32_t)I2C3:
			RCC_I2C3_RESET();
			break;
		default:
			break;
	}
}

/******************************************************************************
 * I2C Peripheral Clock Control
 ******************************************************************************/

/**
 * @brief  Enables or disables the peripheral clock for the given I2C peripheral.
 * @param  pI2Cx:  Pointer to the base address of the target I2C peripheral.
 * @param  EnOrDi: Macro for enabling or disabling the clock (ENABLE or DISABLE).
 * @retval None
 *
 * @note   The clock must be enabled before any register of the peripheral
 *         is accessed (e.g. before I2C_Init()).
 */
void I2C_PeriClkCtrl(I2C_Registers_t *pI2Cx, uint8_t EnOrDi)
{
	// Enable or disable the clock of the selected I2C in RCC_APB1ENR.
	if (EnOrDi == 1) // Enable
	{
		switch ((uint32_t)pI2Cx)
		{
			case (uint32_t)I2C1:
				RCC_I2C1_CLK_ENABLE();
				break;
			case (uint32_t)I2C2:
				RCC_I2C2_CLK_ENABLE();
				break;
			case (uint32_t)I2C3:
				RCC_I2C3_CLK_ENABLE();
				break;
			default:
				break;
		}
	}
	else
	{
		switch ((uint32_t)pI2Cx)
		{
			case (uint32_t)I2C1:
				RCC_I2C1_CLK_DISABLE();
				break;
			case (uint32_t)I2C2:
				RCC_I2C2_CLK_DISABLE();
				break;
			case (uint32_t)I2C3:
				RCC_I2C3_CLK_DISABLE();
				break;
			default:
				break;
		}
	}
}

/******************************************************************************
 * I2C Peripheral Control: Enable/Disable
 ******************************************************************************/

/**
 * @brief  Enables or disables the I2C peripheral.
 *
 * @details
 * Controls the I2C peripheral by setting or clearing the PE
 * (Peripheral Enable) bit in the I2C Control Register 1 (CR1).
 *
 * @param  pI2Cx:  Pointer to the I2C peripheral registers.
 * @param  EnOrDi: Peripheral control command:
 *                 ENABLE  - Enable the I2C peripheral.
 *                 DISABLE - Disable the I2C peripheral.
 * @retval None
 *
 * @note   Hardware clears CR1.ACK while PE = 0. Enable ACK with
 *         I2C_ManageAcking() after the peripheral has been enabled.
 */
void I2C_PeripheralCtrl(I2C_Registers_t *pI2Cx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        /* Set the PE bit to enable the I2C peripheral. */
        pI2Cx->CR1 |= (1U << I2C_CR1_PE);
    }
    else
    {
        /* Clear the PE bit to disable the I2C peripheral. */
        pI2Cx->CR1 &= ~(1U << I2C_CR1_PE);
    }
}

/******************************************************************************
 * I2C Interrupt Configuration and Handling
 ******************************************************************************/

/**
 * @brief  Enables or disables an I2C interrupt line in the NVIC.
 *
 * @details
 * Writes 1 to the bit of the selected interrupt in the NVIC Interrupt
 * Set-Enable Registers (ISERx) to enable it, or in the Interrupt
 * Clear-Enable Registers (ICERx) to disable it.
 * Each register covers 32 interrupts:
 * - IRQ  0..31 : ISER0 / ICER0
 * - IRQ 32..63 : ISER1 / ICER1
 * - IRQ 64..95 : ISER2 / ICER2 (e.g. I2C3_EV = 72, I2C3_ER = 73)
 *
 * @param  IRQNumber: The Interrupt Request number in the NVIC
 *                    (e.g. IRQ_NO_I2C1_EV, IRQ_NO_I2C1_ER).
 * @param  EnOrDi:    Macro to enable or disable the interrupt (ENABLE or DISABLE).
 * @retval None
 *
 * @note   This only enables the line in the NVIC. The peripheral itself
 *         raises interrupts only if CR2.ITEVTEN / ITBUFEN / ITERREN are set.
 */
void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
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
 * @brief  Configures the priority of an I2C interrupt in the NVIC.
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
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority)
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

/******************************************************************************
 * I2C Send and Receive
 ******************************************************************************/

/**
 * @brief  Returns the state of a flag in the status register I2C_SR1.
 * @param  pI2Cx:    Pointer to the I2C peripheral registers.
 * @param  FlagMask: Flag mask (e.g. I2C_FLAG_TXE), NOT the bit position.
 * @retval I2C_FLAG_SET or I2C_FLAG_RESET
 */
uint8_t I2C_GetFlagStatus(const I2C_Registers_t *pI2Cx, uint32_t FlagMask)
{
    if ((pI2Cx->SR1 & FlagMask) != 0U)
    {
        return I2C_FLAG_SET;
    }
    return I2C_FLAG_RESET;
}

/**
 * @brief  Generates a START (or repeated START) condition.
 * @param  pI2Cx: Pointer to the I2C peripheral registers.
 * @retval None
 *
 * @note   Sets CR1.START. In master mode the hardware generates the START
 *         as soon as the bus is free and clears the bit afterwards.
 *         Completion is signalled by SR1.SB (EV5).
 */
static void I2C_GenerateStartCondition(I2C_Registers_t *pI2Cx)
{
    pI2Cx->CR1 |= (uint16_t)(1U << I2C_CR1_START);
}

/**
 * @brief  Generates a STOP condition (CR1.STOP).
 * @param  pI2Cx: Pointer to the I2C peripheral registers.
 * @retval None
 *
 * @note   Hardware clears TXE and BTF together with the STOP request and
 *         clears CR1.STOP once the STOP has been sent.
 */
static void I2C_GenerateStopCondition(I2C_Registers_t *pI2Cx)
{
    pI2Cx->CR1 |= (uint16_t)(1U << I2C_CR1_STOP);
}

/**
 * @brief  Sends the 7-bit slave address with the R/nW bit = 0 (write).
 *
 * @details
 * Address byte = (SlaveAddr << 1) | 0. Writing DR also clears SB
 * (SR1 was read while waiting for SB, so the EV5 clear sequence is complete).
 *
 * @param  pI2Cx:     Pointer to the I2C peripheral registers.
 * @param  SlaveAddr: 7-bit slave address (NOT shifted).
 * @retval None
 */
static void I2C_ExecuteAddressPhaseWrite(I2C_Registers_t *pI2Cx, uint8_t SlaveAddr)
{
    uint8_t addrByte = (uint8_t)(SlaveAddr << 1U);   /* bit 0 = 0 -> write */
    pI2Cx->DR = addrByte;
}

/**
 * @brief  Clears the ADDR flag (EV6 in master mode, EV1 in slave mode).
 *
 * @details
 * Software sequence: read SR1, then read SR2. SCL is stretched (held low)
 * until this sequence is done. SR1 was already read in the wait loop, but
 * it is read again here to make the sequence independent of that.
 *
 * @param  pI2Cx: Pointer to the I2C peripheral registers.
 * @retval None
 */
static void I2C_ClearADDRFlag(I2C_Registers_t *pI2Cx)
{
    volatile uint32_t dummy;
    dummy = pI2Cx->SR1;
    dummy = pI2Cx->SR2;
    (void)dummy;   /* avoid "unused variable" warning */
}

/**
 * @brief  Waits until a flag in SR1 is set; aborts on error or timeout.
 *
 * @details
 * Also watches AF (NACK), BERR and ARLO. Otherwise a missing slave would
 * block the loop forever (ADDR is never set on a NACK).
 * The loop is limited by I2C_WAIT_TIMEOUT.
 *
 * @param  pI2Cx:    Pointer to the I2C peripheral registers.
 * @param  FlagMask: Flag mask in SR1 (e.g. I2C_FLAG_SB), NOT the bit position.
 * @retval I2C_STATUS_OK, I2C_STATUS_NACK, I2C_STATUS_BUS_ERROR or I2C_STATUS_TIMEOUT
 */
static I2C_Status_t I2C_WaitForFlag(I2C_Registers_t *pI2Cx, uint32_t FlagMask)
{
    uint32_t timeout = I2C_WAIT_TIMEOUT;

    while (I2C_GetFlagStatus(pI2Cx, FlagMask) == I2C_FLAG_RESET)
    {
        if (I2C_GetFlagStatus(pI2Cx, I2C_FLAG_AF) == I2C_FLAG_SET)
        {
            return I2C_STATUS_NACK;
        }
        if ((pI2Cx->SR1 & (I2C_FLAG_BERR | I2C_FLAG_ARLO)) != 0U)
        {
            return I2C_STATUS_BUS_ERROR;
        }
        if (timeout-- == 0U)
        {
            return I2C_STATUS_TIMEOUT;
        }
    }
    return I2C_STATUS_OK;
}

/**
 * @brief  Enables or disables the ACK generation (CR1.ACK).
 * @param  pI2Cx:  Pointer to the I2C peripheral registers.
 * @param  EnOrDi: ENABLE or DISABLE.
 * @retval None
 *
 * @note   Hardware clears ACK while PE = 0. Call this AFTER
 *         I2C_PeripheralCtrl(pI2Cx, ENABLE), otherwise a slave does not
 *         acknowledge its own address and the master sees a NAK.
 */
void I2C_ManageAcking(I2C_Registers_t *pI2Cx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        pI2Cx->CR1 |= (uint16_t)(1U << I2C_CR1_ACK);
    }
    else
    {
        pI2Cx->CR1 &= (uint16_t)~(1U << I2C_CR1_ACK);
    }
}

/**
 * @brief  Ends an interrupt-driven master transmission.
 *
 * @details
 * Disables the event and buffer interrupts, clears the transmit
 * parameters in the handle and sets the transmit state back to I2C_READY.
 * The error interrupt (ITERREN) stays enabled.
 *
 * @param  pI2CHandle: Pointer to the I2C handle.
 * @retval None
 */
static void I2C_CloseSendData(I2C_Hndl_t *pI2CHandle)
{
    pI2CHandle->pI2Cx_StartAddr->CR2 &= (uint16_t)~((1U << I2C_CR2_ITEVTEN) |
                                                    (1U << I2C_CR2_ITBUFEN));
    pI2CHandle->pTxBuffer = 0;
    pI2CHandle->TxLen     = 0U;
    pI2CHandle->TxState   = I2C_READY;
}

/**
 * @brief  Sends data as I2C master (blocking / polling).
 *
 * @details
 * Transfer sequence (master transmitter, 7-bit address, RM0090,
 * events EV5 / EV6 / EV8_1 / EV8 / EV8_2):
 *
 *     S | Address | A | Data1 | A | Data2 | A | ... | DataN | A | P
 *       EV5       EV6 EV8_1   EV8                       EV8_2
 *
 * - EV5:   SB = 1.   Cleared by reading SR1, then writing DR (address).
 * - EV6:   ADDR = 1. Cleared by reading SR1, then reading SR2.
 * - EV8_1: TXE = 1.  Shift register and DR empty -> write Data1 to DR.
 * - EV8:   TXE = 1.  Write the next byte to DR (while the previous one
 *                    is still being shifted out).
 * - EV8_2: TXE = 1 and BTF = 1 after the last byte -> program STOP.
 *                    TXE and BTF are cleared by hardware with the STOP.
 *
 * The ACK ("A") of the slave is checked via the AF flag in I2C_WaitForFlag().
 * On error a STOP is generated and the error flags are cleared.
 *
 * @param  pI2CHandle:    Pointer to the I2C handle (master peripheral, PE = 1).
 * @param  pTxBuffer:     Pointer to the data to send.
 * @param  Length:        Number of bytes to send.
 * @param  SlaveAddr:     7-bit slave address (NOT shifted).
 * @param  RepeatedStart: I2C_REPEATED_START_ON  = no STOP at the end,
 *                        I2C_REPEATED_START_OFF = generate STOP.
 * @retval I2C_STATUS_OK, I2C_STATUS_NACK, I2C_STATUS_BUS_ERROR or I2C_STATUS_TIMEOUT
 *
 * @note   The function returns after the STOP was requested, not after it
 *         has finished on the bus. Wait for SR2.BUSY = 0 before disabling
 *         the peripheral.
 */
I2C_Status_t I2C_MasterSendData(I2C_Hndl_t *pI2CHandle, const uint8_t *pTxBuffer,
                                uint32_t Length, uint8_t SlaveAddr, uint8_t RepeatedStart)
{
    I2C_Registers_t *pI2Cx = pI2CHandle->pI2Cx_StartAddr;
    I2C_Status_t status;

    /* 1. "S": generate the START condition (CR1.START = 1). */
    I2C_GenerateStartCondition(pI2Cx);

    /* 2. EV5: wait until SB = 1 (START sent). Until SB is cleared
     *    (read SR1, then write DR), SCL is stretched (held low). */
    status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_SB);
    if (status != I2C_STATUS_OK) { goto error; }

    /* 3. "Address": send 7-bit address + R/nW = 0 (write) to DR.
     *    Writing DR completes the EV5 clear sequence (SB = 0). */
    I2C_ExecuteAddressPhaseWrite(pI2Cx, SlaveAddr);

    /* 4. "A" / EV6: wait until ADDR = 1 (slave has acknowledged its address).
     *    No slave -> AF is set instead -> I2C_STATUS_NACK. */
    status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_ADDR);
    if (status != I2C_STATUS_OK) { goto error; }

    /* 5. Clear ADDR (EV6): read SR1, then read SR2. SCL is stretched until then. */
    I2C_ClearADDRFlag(pI2Cx);

    /* 6. EV8_1 (first byte) and EV8 (all following bytes):
     *    wait for TXE = 1, write the next byte to DR, repeat until Length = 0.
     *    The slave's "A" after each byte shows up as AF = 1 if it is missing. */
    while (Length > 0U)
    {
        status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_TXE);   /* DR empty? */
        if (status != I2C_STATUS_OK) { goto error; }

        pI2Cx->DR = *pTxBuffer;
        pTxBuffer++;
        Length--;
    }

    /* 7. EV8_2: after the last byte wait for TXE = 1 and BTF = 1 (DR and
     *    shift register are empty, last byte fully sent and acknowledged).
     *    With BTF = 1 SCL is stretched until STOP / next write. */
    status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_TXE);
    if (status != I2C_STATUS_OK) { goto error; }
    status = I2C_WaitForFlag(pI2Cx, I2C_FLAG_BTF);
    if (status != I2C_STATUS_OK) { goto error; }

    /* 8. "P": program the STOP condition (clears TXE and BTF in hardware).
     *    Not waiting for its completion. With a
     *    repeated start the caller keeps the bus and starts again. */
    if (RepeatedStart == I2C_REPEATED_START_OFF)
    {
        I2C_GenerateStopCondition(pI2Cx);
    }
    return I2C_STATUS_OK;

error:
    /* Release the bus and clear the error flags (write 0 to the bits). */
    I2C_GenerateStopCondition(pI2Cx);
    pI2Cx->SR1 &= (uint16_t)~(I2C_FLAG_AF | I2C_FLAG_BERR | I2C_FLAG_ARLO);
    return status;
}

/**
 * @brief  Starts a master transmission in interrupt mode (non-blocking).
 *
 * @details
 * Stores the transmit parameters in the handle, enables the event and error
 * interrupts and generates the START condition. Everything else (address,
 * data, STOP) is done in I2C_EV_IRQHandling().
 * When the transfer is finished, I2C_ApplicationEventCallback() is called
 * with I2C_EVENT_TX_COMPLETE (or with an I2C_ERROR_xxx code).
 *
 * @param  pI2CHandle:    Pointer to the I2C handle (master peripheral, PE = 1).
 * @param  pTxBuffer:     Pointer to the data to send. Must stay valid until
 *                        the transfer is done.
 * @param  Length:        Number of bytes to send.
 * @param  SlaveAddr:     7-bit slave address (NOT shifted).
 * @param  RepeatedStart: I2C_REPEATED_START_ON  = no STOP at the end,
 *                        I2C_REPEATED_START_OFF = generate STOP.
 * @retval I2C_STATUS_OK or I2C_STATUS_BUSY (a transmission is already running)
 */
I2C_Status_t I2C_MasterSendDataIT(I2C_Hndl_t *pI2CHandle, const uint8_t *pTxBuffer,
                                  uint32_t Length, uint8_t SlaveAddr, uint8_t RepeatedStart)
{
    if (pI2CHandle->TxState != I2C_READY)
    {
        return I2C_STATUS_BUSY;
    }

    /* Store the transfer parameters in the handle (used by the ISR). */
    pI2CHandle->pTxBuffer     = pTxBuffer;
    pI2CHandle->TxLen         = Length;
    pI2CHandle->DevAddr       = SlaveAddr;
    pI2CHandle->RepeatedStart = RepeatedStart;
    pI2CHandle->TxState       = I2C_BUSY_IN_TX;

    /* Enable event and error interrupts BEFORE the START, so SB is not missed.
     * ITBUFEN (TXE interrupt) is enabled later, after the address phase. */
    pI2CHandle->pI2Cx_StartAddr->CR2 |= (uint16_t)((1U << I2C_CR2_ITERREN) |
                                                   (1U << I2C_CR2_ITEVTEN));

    /* "S": the rest of the transfer continues in I2C_EV_IRQHandling() (EV5). */
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx_StartAddr);

    return I2C_STATUS_OK;
}

/**
 * @brief  Prepares the slave to receive data in interrupt mode.
 *
 * @details
 * Stores the user buffer in the handle and enables the event (ITEVTEN),
 * buffer (ITBUFEN) and error (ITERREN) interrupts. The data are received
 * in I2C_EV_IRQHandling(); a STOP condition from the master ends the
 * message (I2C_EVENT_STOP_DETECTED). The interrupts stay enabled, so the
 * next message is received as well.
 *
 * @param  pI2CHandle: Pointer to the I2C handle (slave peripheral).
 * @param  pRxBuffer:  Buffer for the received bytes.
 * @param  Length:     Size of the buffer in bytes.
 * @retval I2C_STATUS_OK or I2C_STATUS_BUSY (a reception is already running)
 *
 * @note   Call I2C_PeripheralCtrl(ENABLE) and then I2C_ManageAcking(ENABLE)
 *         for the slave afterwards, so that it acknowledges its own address.
 */
I2C_Status_t I2C_SlaveReceiveDataIT(I2C_Hndl_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Length)
{
    if (pI2CHandle->RxState != I2C_READY)
    {
        return I2C_STATUS_BUSY;
    }

    /* Store the user buffer and reset the receive counter. */
    pI2CHandle->pRxBuffer = pRxBuffer;
    pI2CHandle->RxSize    = Length;
    pI2CHandle->RxLen     = 0U;
    pI2CHandle->RxState   = I2C_BUSY_IN_RX;

    /* Enable error, event and buffer (RXNE/TXE) interrupts. */
    pI2CHandle->pI2Cx_StartAddr->CR2 |= (uint16_t)((1U << I2C_CR2_ITERREN) |
                                                   (1U << I2C_CR2_ITEVTEN) |
                                                   (1U << I2C_CR2_ITBUFEN));
    return I2C_STATUS_OK;
}


/**
 * @brief  Handles the I2C event interrupt (master transmitter AND slave receiver).
 *
 * @details
 * Master transmitter (RM0090), 7-bit address:
 *
 *     S | Address | A | Data1 | A | ... | DataN | A | P
 *       EV5       EV6 EV8_1   EV8                   EV8_2
 *
 * - EV5  (SB):   write the address (+W) to DR (clears SB).
 * - EV6  (ADDR): clear ADDR (read SR1, SR2), then enable ITBUFEN.
 * - EV8_1 / EV8 (TXE): write the next byte to DR. After the last byte
 *                ITBUFEN is disabled (else TXE would keep firing).
 * - EV8_2 (BTF + TXE, no bytes left): generate STOP, close the transfer
 *                and call I2C_ApplicationEventCallback(I2C_EVENT_TX_COMPLETE).
 *
 * Slave receiver:
 *
 *     S | Address | A | Data1 | A | ... | DataN | A | P
 *       EV1 (ADDR)      EV2 (RXNE)                  EV4 (STOPF)
 *
 * - EV1  (ADDR):  clear ADDR (read SR1, then SR2).
 * - EV2  (RXNE):  read DR and store the byte in the user buffer.
 * - EV4  (STOPF): read SR1, then write CR1 (clears STOPF). The message is
 *                 complete: I2C_EVENT_STOP_DETECTED is reported.
 *
 * @param  pI2CHandle: Pointer to the I2C handle of the peripheral that raised
 *                     the interrupt.
 * @retval None
 *
 * @note   Call this function from the I2Cx_EV_IRQHandler() of the application.
 * @note   The handle states (TxState / RxState) decide whether the master or
 *         the slave part of the handler is active.
 */
void I2C_EV_IRQHandling(I2C_Hndl_t *pI2CHandle)
{
    I2C_Registers_t *pI2Cx = pI2CHandle->pI2Cx_StartAddr;
    volatile uint32_t dummy;
    uint32_t cr2 = pI2Cx->CR2;
    uint32_t sr1 = pI2Cx->SR1;

    /* Do nothing if the event interrupt is not enabled for this peripheral. */
    if ((cr2 & (1U << I2C_CR2_ITEVTEN)) == 0U)
    {
        return;
    }

    /* EV5: START sent (master only) -> send the address, R/nW = 0. */
    if (((sr1 & I2C_FLAG_SB) != 0U) && (pI2CHandle->TxState == I2C_BUSY_IN_TX))
    {
        I2C_ExecuteAddressPhaseWrite(pI2Cx, pI2CHandle->DevAddr);
    }

    /* EV6 (master) / EV1 (slave): address phase done -> clear ADDR. */
    if ((sr1 & I2C_FLAG_ADDR) != 0U)
    {
        dummy = pI2Cx->SR1;     /* clear sequence: read SR1 ... */
        dummy = pI2Cx->SR2;     /* ... then read SR2            */
        (void)dummy;

        if (pI2CHandle->TxState == I2C_BUSY_IN_TX)
        {
            /* Master: from now on TXE interrupts are wanted. */
            pI2Cx->CR2 |= (uint16_t)(1U << I2C_CR2_ITBUFEN);
        }
    }

    /* EV8_1 / EV8 (master): DR is empty -> next byte. */
    if (((sr1 & I2C_FLAG_TXE) != 0U) &&
        ((cr2 & (1U << I2C_CR2_ITBUFEN)) != 0U) &&
        (pI2CHandle->TxState == I2C_BUSY_IN_TX))
    {
        if (pI2CHandle->TxLen > 0U)
        {
            pI2Cx->DR = *pI2CHandle->pTxBuffer;
            pI2CHandle->pTxBuffer++;
            pI2CHandle->TxLen--;
        }
        else
        {
            /* Everything is in DR/shift register: wait for BTF instead. */
            pI2Cx->CR2 &= (uint16_t)~(1U << I2C_CR2_ITBUFEN);
        }
    }

    /* EV8_2 (master): last byte completely sent (BTF = 1 and TXE = 1). */
    if (((sr1 & I2C_FLAG_BTF) != 0U) && (pI2CHandle->TxState == I2C_BUSY_IN_TX))
    {
        /* Re-read SR1: a DR write above clears TXE/BTF. */
        if (((pI2Cx->SR1 & I2C_FLAG_TXE) != 0U) && (pI2CHandle->TxLen == 0U))
        {
            if (pI2CHandle->RepeatedStart == I2C_REPEATED_START_OFF)
            {
                I2C_GenerateStopCondition(pI2Cx);    /* clears BTF/TXE */
            }
            I2C_CloseSendData(pI2CHandle);
            I2C_ApplicationEventCallback(pI2CHandle, I2C_EVENT_TX_COMPLETE);
        }
    }

    /* EV2 (slave): byte received. Reading DR clears RXNE. */
    if (((sr1 & I2C_FLAG_RXNE) != 0U) &&
        ((cr2 & (1U << I2C_CR2_ITBUFEN)) != 0U) &&
        (pI2CHandle->RxState == I2C_BUSY_IN_RX))
    {
        uint8_t data = (uint8_t)pI2Cx->DR;

        if (pI2CHandle->RxLen < pI2CHandle->RxSize)
        {
            pI2CHandle->pRxBuffer[pI2CHandle->RxLen] = data;
            pI2CHandle->RxLen++;
        }
        else
        {
            I2C_ApplicationEventCallback(pI2CHandle, I2C_EVENT_RX_BUFFER_FULL);
        }
    }

    /* EV4 (slave): STOP detected = end of the message. */
    if ((sr1 & (1U << I2C_SR1_STOPF)) != 0U)
    {
        dummy = pI2Cx->SR1;         /* read SR1 ...                      */
        pI2Cx->CR1 |= 0x0000U;      /* ... then write CR1 (clears STOPF) */
        (void)dummy;

        pI2CHandle->RxState = I2C_READY;
        I2C_ApplicationEventCallback(pI2CHandle, I2C_EVENT_STOP_DETECTED);
    }
}

/**
 * @brief  Handles the I2C error interrupt (BERR, ARLO, AF, OVR).
 *
 * @details
 * Each error flag is cleared by writing 0 to it and is reported to the
 * application through I2C_ApplicationEventCallback(). If a master
 * transmission is running it is aborted: STOP is generated and the handle
 * is closed (AF = the slave sent a NAK).
 *
 * @param  pI2CHandle: Pointer to the I2C handle of the peripheral that raised
 *                     the interrupt.
 * @retval None
 *
 * @note   Call this function from the I2Cx_ER_IRQHandler() of the application.
 */
void I2C_ER_IRQHandling(I2C_Hndl_t *pI2CHandle)
{
    I2C_Registers_t *pI2Cx = pI2CHandle->pI2Cx_StartAddr;
    uint32_t sr1 = pI2Cx->SR1;
    uint8_t  event = 0xFFU;        /* 0xFF = no error found */

    /* Do nothing if the error interrupt is not enabled for this peripheral. */
    if ((pI2Cx->CR2 & (1U << I2C_CR2_ITERREN)) == 0U)
    {
        return;
    }

    if ((sr1 & I2C_FLAG_BERR) != 0U)
    {
        pI2Cx->SR1 &= (uint16_t)~I2C_FLAG_BERR;
        event = I2C_ERROR_BERR;
    }
    if ((sr1 & I2C_FLAG_ARLO) != 0U)
    {
        pI2Cx->SR1 &= (uint16_t)~I2C_FLAG_ARLO;
        event = I2C_ERROR_ARLO;
    }
    if ((sr1 & I2C_FLAG_AF) != 0U)
    {
        pI2Cx->SR1 &= (uint16_t)~I2C_FLAG_AF;
        event = I2C_ERROR_AF;
    }
    if ((sr1 & (1U << I2C_SR1_OVR)) != 0U)
    {
        pI2Cx->SR1 &= (uint16_t)~(1U << I2C_SR1_OVR);
        event = I2C_ERROR_OVR;
    }

    /* An error was found: abort a running transmission, then tell the application. */
    if (event != 0xFFU)
    {
        /* Abort a running master transmission. */
        if (pI2CHandle->TxState == I2C_BUSY_IN_TX)
        {
            I2C_GenerateStopCondition(pI2Cx);
            I2C_CloseSendData(pI2CHandle);
        }
        I2C_ApplicationEventCallback(pI2CHandle, event);
    }
}

/**
 * @brief  Application event callback (weak default implementation).
 *
 * @details
 * Called by I2C_EV_IRQHandling() and I2C_ER_IRQHandling() when a transfer
 * is complete or an error occurred. The application overrides this function
 * by defining a function with the same name (see the I2C test applications).
 *
 * @param  pI2CHandle: Pointer to the I2C handle that reported the event.
 * @param  Event:      I2C_EVENT_xxx or I2C_ERROR_xxx code.
 * @retval None
 */
__attribute__((weak)) void I2C_ApplicationEventCallback(I2C_Hndl_t *pI2CHandle, uint8_t Event)
{
    (void)pI2CHandle;
    (void)Event;
}
