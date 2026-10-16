/**
 * @file    stm32F407.h
 * @brief   Device-level definitions for the STM32F407 microcontroller.
 *
 * @details
 * Defines the memory map, peripheral base addresses, register structures,
 * peripheral access macros, and RCC clock-control macros used by the
 * low-level STM32F407 peripheral drivers.
 *
 * These definitions provide direct access to memory-mapped hardware
 * registers and form the hardware abstraction layer for peripheral drivers.
 *
 * @note    Register addresses and structure layouts must match the target
 *          device and the STM32F4 reference manual, RM0090.
 *
 * @date    Oct 16, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
 */

#ifndef INC_STM32F407_H_
#define INC_STM32F407_H_

#include <stdint.h>


/******************************************************************************
 * General-Purpose Definitions
 ******************************************************************************/

#define STD_ON     1U
#define STD_OFF    0U

/**
 * @defgroup General_Constants General-Purpose Constants
 * @brief Common logic levels and peripheral control values.
 * @{
 */

#define HIGH    1U  /*!< Logical high level. */
#define LOW     0U  /*!< Logical low level. */

#define ENABLE  1U  /*!< Enable a peripheral or feature. */
#define DISABLE 0U  /*!< Disable a peripheral or feature. */

/** @} */


/******************************************************************************
 * GPIO Port Identification
 ******************************************************************************/

/**
 * @brief Convert a GPIO peripheral base address to its port index.
 *
 * @param ADDR GPIO peripheral pointer or base address.
 *
 * @return Port index from 0 to 9 for GPIOA through GPIOJ.
 *         Returns 0 if no listed address matches.
 *
 * @note GPIOK is not included in this mapping.
 */
#define GPIO_STR_ADDR_TO_PORT(ADDR) ( (ADDR == GPIOA)?0:\
                                      (ADDR == GPIOB)?1:\
                                      (ADDR == GPIOC)?2:\
                                      (ADDR == GPIOD)?3:\
                                      (ADDR == GPIOE)?4:\
                                      (ADDR == GPIOF)?5:\
                                      (ADDR == GPIOG)?6:\
                                      (ADDR == GPIOH)?7:\
                                      (ADDR == GPIOI)?8:\
                                      (ADDR == GPIOJ)?9: 0)


/******************************************************************************
 * NVIC Interrupt Numbers
 ******************************************************************************/

/**
 * @defgroup EXTI_IRQ_Numbers External Interrupt IRQ Numbers
 * @brief NVIC interrupt numbers associated with the EXTI lines.
 *
 * @details
 * EXTI lines 0 through 4 have dedicated IRQs. Lines 5 through 9 share one
 * IRQ, as do lines 10 through 15.
 *
 * These values represent Cortex-M NVIC IRQ numbers, not EXTI line numbers.
 * @{
 */

#define IRQ_NO_EXTI0          6U  /*!< EXTI line 0 interrupt. */
#define IRQ_NO_EXTI1          7U  /*!< EXTI line 1 interrupt. */
#define IRQ_NO_EXTI2          8U  /*!< EXTI line 2 interrupt. */
#define IRQ_NO_EXTI3          9U  /*!< EXTI line 3 interrupt. */
#define IRQ_NO_EXTI4          10U /*!< EXTI line 4 interrupt. */
#define IRQ_NO_EXTI9_5        23U /*!< Shared interrupt for EXTI lines 5-9. */
#define IRQ_NO_EXTI15_10      40U /*!< Shared interrupt for EXTI lines 10-15. */

/** @} */


/******************************************************************************
 * Memory Map
 ******************************************************************************/

/**
 * @defgroup Memory_Base_Addresses Memory Base Addresses
 * @brief Base addresses of the main memory regions.
 *
 * @details
 * These addresses identify the main memory regions used by the STM32F407.
 * Refer to RM0090 for the memory map and memory-region descriptions.
 * @{
 */

// Embedded Flash memory.
// Ref: Reference manual RM0090, 3.3 Embedded flash memory in
//      STM32F405xx/07xx (P75).
#define FLASH_START_ADDRESS  0x08000000UL

// SRAM1: 112 KiB region.
// Ref: Reference manual RM0090, 2.3.1 Embedded SRAM (P68).
#define SRAM1_START_ADDRESS  0x20000000UL

// SRAM2: begins immediately after the 112 KiB SRAM1 region.
// 112 KiB = 112 * 1024 bytes = 0x1C000.
// Ref: Reference manual RM0090, 2.1 System architecture (P59).
#define SRAM2_START_ADDRESS  0x2001C000UL

// System memory, including the factory-programmed bootloader region.
// Ref: Reference manual RM0090, 3.3 Embedded flash memory in
//      STM32F405xx/07xx (P75).
#define ROM_START_ADDRESS    0x1FFF0000UL

/** @} */


/******************************************************************************
 * Peripheral Bus Base Addresses
 ******************************************************************************/

/**
 * @defgroup Peripheral_Bus_Addresses Peripheral Bus Base Addresses
 * @brief Base addresses of the peripheral bus regions.
 *
 * @details
 * Peripheral registers are memory-mapped into the microcontroller's address
 * space. A peripheral address is calculated from its bus base address and
 * its peripheral-specific offset.
 * @{
 */

// APB1 peripheral bus.
// Ref: Reference manual RM0090, 2.3 Memory map (P67).
#define APB1_START_ADDRESS  0x40000000UL

// APB2 peripheral bus.
// Ref: Reference manual RM0090, 2.3 Memory map (P66).
#define APB2_START_ADDRESS  0x40010000UL

// AHB1 peripheral bus.
// Ref: Reference manual RM0090, 2.3 Memory map (P65).
#define AHB1_START_ADDRESS  0x40020000UL

// AHB2 peripheral bus.
// Ref: Reference manual RM0090, 2.3 Memory map (P65).
#define AHB2_START_ADDRESS  0x50000000UL

/** @} */


/******************************************************************************
 * AHB1 Peripheral Addresses
 ******************************************************************************/

/**
 * @defgroup AHB1_Peripheral_Addresses AHB1 Peripheral Addresses
 * @brief Offsets and start addresses for peripherals connected to AHB1.
 *
 * @details
 * Each peripheral start address is calculated from the AHB1 bus base
 * address and the corresponding peripheral offset.
 *
 * Ref: Reference manual RM0090, 2.3 Memory map (P65).
 * @{
 */

// GPIOA
#define GPIOA_OFFSET         0x00000000UL
#define GPIOA_START_ADDRESS  (AHB1_START_ADDRESS + GPIOA_OFFSET)

// GPIOB
#define GPIOB_OFFSET         0x00000400UL
#define GPIOB_START_ADDRESS  (AHB1_START_ADDRESS + GPIOB_OFFSET)

// GPIOC
#define GPIOC_OFFSET         0x00000800UL
#define GPIOC_START_ADDRESS  (AHB1_START_ADDRESS + GPIOC_OFFSET)

// GPIOD
#define GPIOD_OFFSET         0x00000C00UL
#define GPIOD_START_ADDRESS  (AHB1_START_ADDRESS + GPIOD_OFFSET)

// GPIOE
#define GPIOE_OFFSET         0x00001000UL
#define GPIOE_START_ADDRESS  (AHB1_START_ADDRESS + GPIOE_OFFSET)

// GPIOF
#define GPIOF_OFFSET         0x00001400UL
#define GPIOF_START_ADDRESS  (AHB1_START_ADDRESS + GPIOF_OFFSET)

// GPIOG
#define GPIOG_OFFSET         0x00001800UL
#define GPIOG_START_ADDRESS  (AHB1_START_ADDRESS + GPIOG_OFFSET)

// GPIOH
#define GPIOH_OFFSET         0x00001C00UL
#define GPIOH_START_ADDRESS  (AHB1_START_ADDRESS + GPIOH_OFFSET)

// GPIOI
#define GPIOI_OFFSET         0x00002000UL
#define GPIOI_START_ADDRESS  (AHB1_START_ADDRESS + GPIOI_OFFSET)

// GPIOJ
#define GPIOJ_OFFSET         0x00002400UL
#define GPIOJ_START_ADDRESS  (AHB1_START_ADDRESS + GPIOJ_OFFSET)

// GPIOK
#define GPIOK_OFFSET         0x000028000UL
#define GPIOK_START_ADDRESS  (AHB1_START_ADDRESS + GPIOK_OFFSET)

// Reset and clock control (RCC).
#define RCC_OFFSET           0x00003800L
#define RCC_START_ADDRESS    (AHB1_START_ADDRESS + RCC_OFFSET)

/** @} */


/******************************************************************************
 * APB1 Peripheral Addresses
 ******************************************************************************/

/**
 * @defgroup APB1_Peripheral_Addresses APB1 Peripheral Addresses
 * @brief Offsets and start addresses for peripherals connected to APB1.
 *
 * @details
 * Each peripheral start address is calculated from the APB1 bus base
 * address and the corresponding peripheral offset.
 *
 * Ref: Reference manual RM0090, 2.3 Memory map (P67).
 * @{
 */

// SPI2
#define SPI2_OFFSET          0x00003800UL
#define SPI2_START_ADDRESS   APB1_START_ADDRESS + SPI2_OFFSET

// SPI3
#define SPI3_OFFSET          0x00003C00UL
#define SPI3_START_ADDRESS   APB1_START_ADDRESS + SPI3_OFFSET

// USART2
#define USART2_OFFSET        0x00004400UL
#define USART2_START_ADDRESS APB1_START_ADDRESS + USART2_OFFSET

// USART3
#define USART3_OFFSET        0x00004800UL
#define USART3_START_ADDRESS APB1_START_ADDRESS + USART3_OFFSET

// UART4
#define UART4_OFFSET         0x00004C00UL
#define UART4_START_ADDRESS  APB1_START_ADDRESS + UART4_OFFSET

// UART5
#define UART5_OFFSET         0x00005000UL
#define UART5_START_ADDRESS  APB1_START_ADDRESS + UART5_OFFSET

// I2C1
#define I2C1_OFFSET          0x00005400UL
#define I2C1_START_ADDRESS   APB1_START_ADDRESS + I2C1_OFFSET

// I2C2
#define I2C2_OFFSET          0x00005800UL
#define I2C2_START_ADDRESS   APB1_START_ADDRESS + I2C2_OFFSET

// I2C3
#define I2C3_OFFSET          0x00005C00UL
#define I2C3_START_ADDRESS   APB1_START_ADDRESS + I2C3_OFFSET

// CAN1
#define CAN1_OFFSET          0x00006400UL
#define CAN1_START_ADDRESS   APB1_START_ADDRESS + CAN1_OFFSET

// CAN2
#define CAN2_OFFSET          0x00006800UL
#define CAN2_START_ADDRESS   APB1_START_ADDRESS + CAN2_OFFSET

/** @} */


/******************************************************************************
 * APB2 Peripheral Addresses
 ******************************************************************************/

/**
 * @defgroup APB2_Peripheral_Addresses APB2 Peripheral Addresses
 * @brief Offsets and start addresses for peripherals connected to APB2.
 *
 * @details
 * Each peripheral start address is calculated from the APB2 bus base
 * address and the corresponding peripheral offset.
 *
 * Ref: Reference manual RM0090, 2.3 Memory map (P66).
 * @{
 */

// USART1
#define USART1_OFFSET        0x00001000UL
#define USART1_START_ADDRESS APB2_START_ADDRESS + USART1_OFFSET

// USART6
#define USART6_OFFSET        0x00001400UL
#define USART6_START_ADDRESS APB2_START_ADDRESS + USART6_OFFSET

// SPI1
#define SPI1_OFFSET          0x00003000UL
#define SPI1_START_ADDRESS   APB2_START_ADDRESS + SPI1_OFFSET

// SPI4
#define SPI4_OFFSET          0x00003400UL
#define SPI4_START_ADDRESS   APB2_START_ADDRESS + SPI4_OFFSET

// System configuration controller (SYSCFG).
#define SYSCFG_OFFSET        0x00003800UL
#define SYSCFG_START_ADDRESS (APB2_START_ADDRESS + SYSCFG_OFFSET)

// External interrupt/event controller (EXTI).
#define EXTI_OFFSET          0x00003C00UL
#define EXTI_START_ADDRESS   (APB2_START_ADDRESS + EXTI_OFFSET)

/** @} */


/******************************************************************************
 * Register Structures
 ******************************************************************************/

/**
 * @defgroup Peripheral_Register_Structures Peripheral Register Structures
 * @brief C structures representing memory-mapped peripheral registers.
 *
 * @details
 * Each structure models the register layout of a hardware peripheral.
 * Volatile-qualified members are used for memory-mapped register access.
 *
 * Member order, data types, and reserved fields must preserve the register
 * offsets defined in the device reference manual.
 * @{
 */

/**
 * @brief SYSCFG peripheral register layout.
 *
 * @details
 * Represents the system configuration controller registers, including
 * memory remapping, peripheral mode configuration, and EXTI line selection.
 */
typedef struct
{
    volatile uint32_t MEMRMP;     /*!< Memory remap register. Offset: 0x00. */
    volatile uint32_t PMC;        /*!< Peripheral mode configuration register.
                                       Offset: 0x04. */
    volatile uint32_t EXTICR[4];  /*!< External interrupt configuration registers.
                                       Offsets: 0x08-0x14. */
    uint32_t reserved0[2];        /*!< Reserved registers. Offsets: 0x18-0x1C. */
    volatile uint32_t CMPCR;      /*!< Compensation cell control register.
                                       Offset: 0x20. */
} SYSCFG_Registers_t;


/**
 * @brief EXTI peripheral register layout.
 *
 * @details
 * Represents the registers used to configure external interrupt/event lines,
 * select trigger edges, generate software interrupts, and inspect or clear
 * pending interrupt flags.
 */
typedef struct
{
    volatile uint32_t IMR;    /*!< Interrupt mask register. Offset: 0x00. */
    volatile uint32_t EMR;    /*!< Event mask register. Offset: 0x04. */
    volatile uint32_t RTSR;   /*!< Rising trigger selection register.
                                   Offset: 0x08. */
    volatile uint32_t FTSR;   /*!< Falling trigger selection register.
                                   Offset: 0x0C. */
    volatile uint32_t SWIER;  /*!< Software interrupt event register.
                                   Offset: 0x10. */
    volatile uint32_t PR;     /*!< Pending register. Offset: 0x14. */
} EXTI_Registers_t;


/**
 * @brief GPIO peripheral register layout.
 *
 * @details
 * Represents the memory-mapped registers of a GPIO port, arranged in
 * hardware address order.
 *
 * Each port provides pin mode, output type, speed, pull configuration,
 * input/output data, bit set/reset, configuration locking, and alternate
 * function selection registers.
 */
typedef struct
{
    volatile uint32_t MODER;    /*!< GPIO port mode register. Offset: 0x00. */
    volatile uint32_t OTYPER;   /*!< GPIO port output type register.
                                     Offset: 0x04. */
    volatile uint32_t OSPEEDR;  /*!< GPIO port output speed register.
                                     Offset: 0x08. */
    volatile uint32_t PUPDR;    /*!< GPIO port pull-up/pull-down register.
                                     Offset: 0x0C. */
    volatile uint32_t IDR;      /*!< GPIO port input data register.
                                     Offset: 0x10. */
    volatile uint32_t ODR;      /*!< GPIO port output data register.
                                     Offset: 0x14. */
    volatile uint32_t BSRR;     /*!< GPIO port bit set/reset register.
                                     Offset: 0x18. */
    volatile uint32_t LCKR;     /*!< GPIO port configuration lock register.
                                     Offset: 0x1C. */
    volatile uint32_t AFRL;     /*!< GPIO alternate function low register.
                                     Offset: 0x20. */
    volatile uint32_t AFRH;     /*!< GPIO alternate function high register.
                                     Offset: 0x24. */
} GPIO_Registers_t;


/**
 * @brief RCC peripheral register layout.
 *
 * @details
 * Represents the reset and clock control registers. These registers manage
 * oscillator and PLL configuration, system and peripheral clock selection,
 * peripheral resets, clock enables, and low-power clock enables.
 *
 * Reserved members preserve the expected register offsets.
 */
typedef struct
{
    volatile uint32_t CR;          /*!< Clock control register. Offset: 0x00. */
    volatile uint32_t PLLCFGR;     /*!< PLL configuration register.
                                        Offset: 0x04. */
    volatile uint32_t CFGR;        /*!< Clock configuration register.
                                        Offset: 0x08. */
    volatile uint32_t CIR;         /*!< Clock interrupt register.
                                        Offset: 0x0C. */
    volatile uint32_t AHB1RSTR;    /*!< AHB1 peripheral reset register.
                                        Offset: 0x10. */
    volatile uint32_t AHB2RSTR;    /*!< AHB2 peripheral reset register.
                                        Offset: 0x14. */
    volatile uint32_t AHB3RSTR;    /*!< AHB3 peripheral reset register.
                                        Offset: 0x18. */
    volatile uint32_t APB1RSTR;    /*!< APB1 peripheral reset register.
                                        Offset: 0x1C. */
    uint32_t reserved0;            /*!< Reserved. Offset: 0x20. */
    volatile uint32_t APB2RSTR;    /*!< APB2 peripheral reset register.
                                        Offset: 0x24. */
    uint32_t reserved1[2];         /*!< Reserved. Offsets: 0x28-0x2C. */
    volatile uint32_t AHB1ENR;     /*!< AHB1 peripheral clock enable register.
                                        Offset: 0x30. */
    volatile uint32_t AHB2ENR;     /*!< AHB2 peripheral clock enable register.
                                        Offset: 0x34. */
    volatile uint32_t AHB3ENR;     /*!< AHB3 peripheral clock enable register.
                                        Offset: 0x38. */
    uint32_t reserved2;            /*!< Reserved. Offset: 0x3C. */
    volatile uint32_t APB1ENR;     /*!< APB1 peripheral clock enable register.
                                        Offset: 0x40. */
    volatile uint32_t APB2ENR;     /*!< APB2 peripheral clock enable register.
                                        Offset: 0x44. */
    uint32_t reserved3[2];         /*!< Reserved. Offsets: 0x48-0x4C. */
    volatile uint32_t AHB1LPENR;   /*!< AHB1 peripheral low-power clock enable
                                        register. Offset: 0x50. */
    volatile uint32_t AHB2LPENR;   /*!< AHB2 peripheral low-power clock enable
                                        register. Offset: 0x54. */
    volatile uint32_t AHB3LPENR;   /*!< AHB3 peripheral low-power clock enable
                                        register. Offset: 0x58. */
    uint32_t reserved4;            /*!< Reserved. Offset: 0x5C. */
    volatile uint32_t APB1LPENR;   /*!< APB1 peripheral low-power clock enable
                                        register. Offset: 0x60. */
    volatile uint32_t APB2LPENR;   /*!< APB2 peripheral low-power clock enable
                                        register. Offset: 0x64. */
    uint32_t reserved5[2];         /*!< Reserved. Offsets: 0x68-0x6C. */
    volatile uint32_t BDCR;        /*!< Backup domain control register.
                                        Offset: 0x70. */
    volatile uint32_t CSR;         /*!< Clock control and status register.
                                        Offset: 0x74. */
    uint32_t reserved6[2];         /*!< Reserved. Offsets: 0x78-0x7C. */
    volatile uint32_t SSCGR;       /*!< Spread spectrum clock generation
                                        register. Offset: 0x80. */
    volatile uint32_t PLLI2SCFGR;  /*!< PLLI2S configuration register.
                                        Offset: 0x84. */
} RCC_Registers_t;

/** @} */


/******************************************************************************
 * Peripheral Access Macros
 ******************************************************************************/

/**
 * @defgroup Peripheral_Access_Macros Peripheral Access Macros
 * @brief Typed pointers to memory-mapped peripheral register blocks.
 *
 * @details
 * These macros convert peripheral base addresses into pointers to their
 * corresponding register structures, allowing register access through
 * structure-member notation.
 * @{
 */

// SYSCFG and EXTI peripherals.
#define SYSCFG ((SYSCFG_Registers_t*)SYSCFG_START_ADDRESS)
#define EXTI   ((EXTI_Registers_t*)EXTI_START_ADDRESS)

// GPIO peripherals.
#define GPIOA  (GPIO_Registers_t*)(GPIOA_START_ADDRESS)
#define GPIOB  (GPIO_Registers_t*)(GPIOB_START_ADDRESS)
#define GPIOC  (GPIO_Registers_t*)(GPIOC_START_ADDRESS)
#define GPIOD  (GPIO_Registers_t*)(GPIOD_START_ADDRESS)
#define GPIOE  (GPIO_Registers_t*)(GPIOE_START_ADDRESS)
#define GPIOF  (GPIO_Registers_t*)(GPIOF_START_ADDRESS)
#define GPIOG  (GPIO_Registers_t*)(GPIOG_START_ADDRESS)
#define GPIOH  (GPIO_Registers_t*)(GPIOH_START_ADDRESS)
#define GPIOI  (GPIO_Registers_t*)(GPIOI_START_ADDRESS)
#define GPIOJ  (GPIO_Registers_t*)(GPIOJ_START_ADDRESS)
#define GPIOK  (GPIO_Registers_t*)(GPIOK_START_ADDRESS)

// Reset and clock control peripheral.
#define RCC    ((RCC_Registers_t*)RCC_START_ADDRESS)

/** @} */


/******************************************************************************
 * RCC Clock Control Macros
 ******************************************************************************/

/**
 * @defgroup RCC_GPIO_Clock_Control GPIO Clock Control
 * @brief Enable, disable, and reset GPIO peripheral clocks.
 * @{
 */

/* GPIO clock enable macros. */
#define RCC_GPIOA_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 0))
#define RCC_GPIOB_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 1))
#define RCC_GPIOC_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 2))
#define RCC_GPIOD_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 3))
#define RCC_GPIOE_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 4))
#define RCC_GPIOF_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 5))
#define RCC_GPIOG_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 6))
#define RCC_GPIOH_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 7))
#define RCC_GPIOI_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 8))
#define RCC_GPIOJ_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 9))
#define RCC_GPIOK_CLK_ENABLE() (RCC->AHB1ENR |= (1 << 10))

/* GPIO clock disable macros. */
#define RCC_GPIOA_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 0))
#define RCC_GPIOB_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 1))
#define RCC_GPIOC_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 2))
#define RCC_GPIOD_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 3))
#define RCC_GPIOE_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 4))
#define RCC_GPIOF_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 5))
#define RCC_GPIOG_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 6))
#define RCC_GPIOH_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 7))
#define RCC_GPIOI_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 8))
#define RCC_GPIOJ_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 9))
#define RCC_GPIOK_CLK_DISABLE() (RCC->AHB1ENR &= ~(1 << 10))

/**
 * @brief Generate a GPIO port reset sequence through the RCC peripheral.
 *
 * @details
 * Each reset macro asserts the corresponding AHB1 peripheral reset bit
 * and then clears it to release the peripheral from reset.
 *
 * The reset bit positions correspond to the GPIO port indices.
 */

/* GPIO port reset macros. */
#define RCC_GPIOA_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 0);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 0);      /* Release reset */  \
    } while (0)

#define RCC_GPIOB_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 1);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 1);      /* Release reset */  \
    } while (0)

#define RCC_GPIOC_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 2);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 2);      /* Release reset */  \
    } while (0)

#define RCC_GPIOD_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 3);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 3);      /* Release reset */  \
    } while (0)

#define RCC_GPIOE_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 4);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 4);      /* Release reset */  \
    } while (0)

#define RCC_GPIOF_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 5);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 5);      /* Release reset */  \
    } while (0)

#define RCC_GPIOG_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 6);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 6);      /* Release reset */  \
    } while (0)

#define RCC_GPIOH_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 7);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 7);      /* Release reset */  \
    } while (0)

#define RCC_GPIOI_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 8);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 8);      /* Release reset */  \
    } while (0)

#define RCC_GPIOJ_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 9);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 9);      /* Release reset */  \
    } while (0)

#define RCC_GPIOK_PORT_RESET()             \
    do {                                   \
        RCC->AHB1RSTR |=  (1U << 10);      /* Assert reset */   \
        RCC->AHB1RSTR &= ~(1U << 10);      /* Release reset */  \
    } while (0)

/** @} */


/**
 * @defgroup RCC_I2C_Clock_Control I2C Clock Control
 * @brief Enable or disable the clocks of I2C peripherals.
 * @{
 */

/* I2C clock enable macros. */
#define RCC_I2C1_CLK_ENABLE() (RCC->APB1ENR |= (1 << 21))
#define RCC_I2C2_CLK_ENABLE() (RCC->APB1ENR |= (1 << 22))
#define RCC_I2C3_CLK_ENABLE() (RCC->APB1ENR |= (1 << 23))

/* I2C clock disable macros. */
#define RCC_I2C1_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 21))
#define RCC_I2C2_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 22))
#define RCC_I2C3_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 23))

/** @} */


/**
 * @defgroup RCC_CAN_Clock_Control CAN Clock Control
 * @brief Enable or disable the clocks of CAN peripherals.
 * @{
 */

/* CAN clock enable macros. */
#define RCC_CAN1_CLK_ENABLE() (RCC->APB1ENR |= (1 << 25))
#define RCC_CAN2_CLK_ENABLE() (RCC->APB1ENR |= (1 << 26))

/* CAN clock disable macros. */
#define RCC_CAN1_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 25))
#define RCC_CAN2_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 26))

/** @} */


/**
 * @defgroup RCC_SPI_Clock_Control SPI Clock Control
 * @brief Enable or disable the clocks of SPI peripherals.
 * @{
 */

/* SPI clock enable macros. */
#define RCC_SPI1_CLK_ENABLE() (RCC->APB2ENR |= (1 << 12))
#define RCC_SPI2_CLK_ENABLE() (RCC->APB1ENR |= (1 << 14))
#define RCC_SPI3_CLK_ENABLE() (RCC->APB1ENR |= (1 << 15))
#define RCC_SPI4_CLK_ENABLE() (RCC->APB2ENR |= (1 << 13))

/* SPI clock disable macros. */
#define RCC_SPI1_CLK_DISABLE() (RCC->APB2ENR &= ~(1 << 12))
#define RCC_SPI2_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 14))
#define RCC_SPI3_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 15))
#define RCC_SPI4_CLK_DISABLE() (RCC->APB2ENR &= ~(1 << 13))

/** @} */


/**
 * @defgroup RCC_USART_Clock_Control USART Clock Control
 * @brief Enable or disable the clocks of USART peripherals.
 * @{
 */

/* USART clock enable macros. */
#define RCC_USART1_CLK_ENABLE() (RCC->APB2ENR |= (1 << 4))
#define RCC_USART2_CLK_ENABLE() (RCC->APB1ENR |= (1 << 17))
#define RCC_USART3_CLK_ENABLE() (RCC->APB1ENR |= (1 << 18))
#define RCC_USART6_CLK_ENABLE() (RCC->APB2ENR |= (1 << 5))

/* USART clock disable macros. */
#define RCC_USART1_CLK_DISABLE() (RCC->APB2ENR &= ~(1 << 4))
#define RCC_USART2_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 17))
#define RCC_USART3_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 18))
#define RCC_USART6_CLK_DISABLE() (RCC->APB2ENR &= ~(1 << 5))

/** @} */


/**
 * @defgroup RCC_UART_Clock_Control UART Clock Control
 * @brief Enable or disable the clocks of UART peripherals.
 * @{
 */

/* UART clock enable macros. */
#define RCC_UART4_CLK_ENABLE() (RCC->APB1ENR |= (1 << 19))
#define RCC_UART5_CLK_ENABLE() (RCC->APB1ENR |= (1 << 20))

/* UART clock disable macros. */
#define RCC_UART4_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 19))
#define RCC_UART5_CLK_DISABLE() (RCC->APB1ENR &= ~(1 << 20))

/** @} */


/**
 * @defgroup RCC_SYSCFG_Clock_Control SYSCFG Clock Control
 * @brief Enable or disable the SYSCFG peripheral clock.
 * @{
 */

/* SYSCFG clock enable macro. */
#define RCC_SYSCFG_CLK_ENABLE() (RCC->APB2ENR |= (1 << 14))

/* SYSCFG clock disable macro. */
#define RCC_SYSCFG_CLK_DISABLE() (RCC->APB2ENR &= ~(1 << 14))

/** @} */


#endif /* INC_STM32F407_H_ */
