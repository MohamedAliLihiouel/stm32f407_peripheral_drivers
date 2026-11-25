/**
 * @file    stm32f407_RCC_driver.c
 * @brief   RCC driver: calculation of the peripheral bus clocks.
 *
 * @details
 * Reads the clock configuration register RCC_CFGR and calculates the
 * frequency of the APB1 and APB2 bus (PCLK1, PCLK2).
 *
 * @date    Nov 23, 2026
 * @author  Mohamed Ali, Lihiouel
 */

#include "stm32f407_RCC_driver.h"

/******************************************************************************
 * Private macros and constant tables
 ******************************************************************************/

#define RCC_HSI_CLK_HZ   (16000000U)   /* HSI: 16 MHz                          */
#define RCC_HSE_CLK_HZ   (8000000U)    /* HSE: 8 MHz on the Discovery board    */

/* AHB prescaler table for HPRE values 8..15 (8 = /2 ... 15 = /512). */
static const uint16_t arrayAhbPresc[8] = {2U, 4U, 8U, 16U, 64U, 128U, 256U, 512U};
/* APB prescaler table for PPRE values 4..7 (/2, /4, /8, /16). */
static const uint8_t  arrayApbPresc[4] = {2U, 4U, 8U, 16U};

/******************************************************************************
 * helper functions
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
 * The result is used by I2C_Init() to program CR2.FREQ, CCR and TRISE. or used to calcule USART DIV_Mantissa and DIV_Fraction
 *
 * @param  None
 * @retval PCLK1 frequency in Hz.
 *
 * @note   Only HSI (16 MHz) and HSE (8 MHz) are supported. If the PLL is the
 *         system clock source (SWS = 2) the system clock stays 0 (still tbd).
 */
uint32_t RCC_GetPCLK1Value(void)
{
	uint32_t tempPCKL1 = 0, sysClk = 0;
	uint8_t tempAhbPresc = 0, tempApbPresc = 0;
	uint8_t ahbPresc = 0, apbPresc = 0;
	// Bits 3:2 SWS: system clock switch status (0 = HSI, 1 = HSE, 2 = PLL)
	uint8_t clkSrc = ((RCC->CFGR >> 2U) & 0x3);

	if(clkSrc == 0U) // HSI oscillator used as the system clock
	{
		sysClk = RCC_HSI_CLK_HZ; // 16 MHZ
	}
	else if(clkSrc == 1U) // HSE oscillator used as the system clock
	{
		sysClk = RCC_HSE_CLK_HZ; // 8 MHZ in STM32 Discovery Board
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

/**
 * @brief  Calculates the APB2 peripheral clock frequency (PCLK1) in Hz.
 *
 * @details
 * PCLK1 feeds the I2C peripherals. It is derived from the system clock
 * source (RCC_CFGR.SWS), the AHB prescaler (HPRE) and the APB2 prescaler
 * (PPRE2):
 *
 *     PCLK1 = SYSCLK / AHB prescaler / APB2 prescaler
 *
 * The result is used by I2C_Init() to program CR2.FREQ, CCR and TRISE. or used to calcule USART DIV_Mantissa and DIV_Fraction
 *
 * @param  None
 * @retval PCLK1 frequency in Hz.
 *
 * @note   Only HSI (16 MHz) and HSE (8 MHz) are supported. If the PLL is the
 *         system clock source (SWS = 2) the system clock stays 0 (still tbd).
 */
uint32_t RCC_GetPCLK2Value(void)
{
	uint32_t tempPCKL2 = 0, sysClk = 0;
	uint8_t tempAhbPresc = 0, tempApbPresc = 0;
	uint8_t ahbPresc = 0, apbPresc = 0;
	// Bits 3:2 SWS: system clock switch status (0 = HSI, 1 = HSE, 2 = PLL)
	uint8_t clkSrc = ((RCC->CFGR >> 2U) & 0x3);

	if(clkSrc == 0U) // HSI oscillator used as the system clock
	{
		sysClk = RCC_HSI_CLK_HZ; // 16 MHZ
	}
	else if(clkSrc == 1U) // HSE oscillator used as the system clock
	{
		sysClk = RCC_HSE_CLK_HZ; // 8 MHZ in STM32 Discovery Board
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

	// Bits 15:13 PPRE2: APB High speed prescaler (APB2)
	tempApbPresc = ((RCC->CFGR >> 13U) & 0x7);
	if(tempApbPresc<4)
	{
		apbPresc = 1;
	}
	else
	{
		apbPresc = arrayApbPresc[tempApbPresc-4U];
	}

	// PCLK1 = SYSCLK / AHB prescaler / APB1 prescaler
	tempPCKL2 = (uint32_t)((sysClk/ahbPresc) / apbPresc);

	return tempPCKL2;
}
