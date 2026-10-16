/**
 * @file    nvic_config.h
 * @brief   Register definitions for the ARM Cortex-M Nested Vectored
 * Interrupt Controller (NVIC).
 *
 * @details
 * Defines the memory-mapped addresses of the NVIC interrupt set-enable
 * (ISER), interrupt clear-enable (ICER), interrupt clear-pending (ICPR),
 * and interrupt priority (IPR) registers.
 *
 * These definitions provide direct access to NVIC registers and are
 * intended for use by low-level interrupt configuration drivers.
 *
 * @note
 * Register addresses and priority configuration must match the
 * Cortex-M core implemented by the target microcontroller.
 *
 * @date    Oct 16, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
*/

#ifndef INC_NVIC_CONFIG_H_
#define INC_NVIC_CONFIG_H_

#include <stdint.h>


/*
 * Number of priority bits not implemented by the MCU.
 * Example: If 4 bits are implemented, the lower 4 bits
 * of each 8-bit priority field are not available.
 */
#define NUM_PR_BITS_NOT_IMPLEMENTED    4U

/*------------------------------------------------------------------------------
 * NVIC Register Base Addresses
 *----------------------------------------------------------------------------*/
#define CORTEX_M_NVIC_ISER_START_ADDRESS     (0xE000E100UL)
#define CORTEX_M_NVIC_ICER_START_ADDRESS     (0XE000E180UL)
#define CORTEX_M_NVIC_ICPR_START_ADDRESS     (0XE000E280UL)
#define CORTEX_M_NVIC_IPR_START_ADDRESS      ((volatile uint32_t *)0xE000E400UL)

/*------------------------------------------------------------------------------
 * Individual NVIC ISER Register Defines (Interrupt Set-enable Registers)
 * Each register controls 32 interrupts (ISER0: 0-31, ISER1: 32-63, etc.)
 *----------------------------------------------------------------------------*/
#define NVIC_ISER0                  ((volatile uint32_t *)(CORTEX_M_NVIC_ISER_START_ADDRESS + 0x00UL))
#define NVIC_ISER1                  ((volatile uint32_t *)(CORTEX_M_NVIC_ISER_START_ADDRESS + 0x04UL))
#define NVIC_ISER2                  ((volatile uint32_t *)(CORTEX_M_NVIC_ISER_START_ADDRESS + 0x08UL))
#define NVIC_ISER3                  ((volatile uint32_t *)(CORTEX_M_NVIC_ISER_START_ADDRESS + 0x0CUL))
#define NVIC_ISER4                  ((volatile uint32_t *)(CORTEX_M_NVIC_ISER_START_ADDRESS + 0x10UL))
#define NVIC_ISER5                  ((volatile uint32_t *)(CORTEX_M_NVIC_ISER_START_ADDRESS + 0x14UL))
#define NVIC_ISER6                  ((volatile uint32_t *)(CORTEX_M_NVIC_ISER_START_ADDRESS + 0x18UL))
#define NVIC_ISER7                  ((volatile uint32_t *)(CORTEX_M_NVIC_ISER_START_ADDRESS + 0x1CUL))

/*------------------------------------------------------------------------------
 * Individual NVIC ICER Register Defines (Interrupt Clear-enable Registers)
 * Each register controls 32 interrupts (ISER0: 0-31, ISER1: 32-63, etc.)
 *----------------------------------------------------------------------------*/
#define NVIC_ICER0                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICER_START_ADDRESS + 0x00UL))
#define NVIC_ICER1                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICER_START_ADDRESS + 0x04UL))
#define NVIC_ICER2                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICER_START_ADDRESS + 0x08UL))
#define NVIC_ICER3                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICER_START_ADDRESS + 0x0CUL))
#define NVIC_ICER4                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICER_START_ADDRESS + 0x10UL))
#define NVIC_ICER5                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICER_START_ADDRESS + 0x14UL))
#define NVIC_ICER6                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICER_START_ADDRESS + 0x18UL))
#define NVIC_ICER7                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICER_START_ADDRESS + 0x1CUL))

/*------------------------------------------------------------------------------
 * Individual NVIC ICER Register Defines (Interrupt Clear-enable Registers)
 * Each register controls 32 interrupts (ISER0: 0-31, ISER1: 32-63, etc.)
 *----------------------------------------------------------------------------*/
#define NVIC_ICPR0                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICPR_START_ADDRESS + 0x00UL))
#define NVIC_ICPR1                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICPR_START_ADDRESS + 0x04UL))
#define NVIC_ICPR2                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICPR_START_ADDRESS + 0x08UL))
#define NVIC_ICPR3                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICPR_START_ADDRESS + 0x0CUL))
#define NVIC_ICPR4                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICPR_START_ADDRESS + 0x10UL))
#define NVIC_ICPR5                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICPR_START_ADDRESS + 0x14UL))
#define NVIC_ICPR6                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICPR_START_ADDRESS + 0x18UL))
#define NVIC_ICPR7                  ((volatile uint32_t *)(CORTEX_M_NVIC_ICPR_START_ADDRESS + 0x1CUL))

#endif /* INC_NVIC_CONFIG_H_ */
