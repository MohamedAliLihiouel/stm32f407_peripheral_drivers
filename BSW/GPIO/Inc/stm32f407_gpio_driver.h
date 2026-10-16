/**
 * @file    stm32f407_gpio_driver.h
 * @brief   Public interface for the STM32F407 GPIO driver.
 *
 * @details
 * This header defines the GPIO pin configuration structures, configuration
 * macros, and public API prototypes used to initialize and control GPIO
 * peripherals on the STM32F407 microcontroller.
 *
 * It also provides functions for GPIO input/output operations, peripheral
 * clock control, and external interrupt configuration and handling.
 *
 * @date    Oct 16, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
 */

#ifndef STM32F407_GPIO_DRIVER_H_
#define STM32F407_GPIO_DRIVER_H_

#include "stm32f407.h"


/******************************************************************************
 * Structures
 ******************************************************************************/

/**
 * @brief GPIO pin configuration structure.
 *
 * @details
 * Contains the configuration parameters required to initialize a GPIO pin,
 * including its operating mode, output speed, pull-up/pull-down configuration,
 * output type, and alternate function selection.
 *
 * The selected parameters are applied by GPIO_Init().
 */
typedef struct
{
    uint8_t GPIO_PinNumber;       /*!< GPIO pin number to configure.
                                       @ref GPIO_pin_numbers */
    uint8_t GPIO_PinMode;         /*!< Operating mode of the selected pin.
                                       @ref GPIO_pin_modes */
    uint8_t GPIO_PinSpeed;        /*!< Output speed (slew rate) of the pin.
                                       @ref GPIO_pin_speed */
    uint8_t GPIO_PinPuPdRCtrl;    /*!< Internal pull-up/pull-down configuration.
                                       @ref GPIO_pin_pupd */
    uint8_t GPIO_PinOPType;       /*!< Output driver type: push-pull or open-drain.
                                       @ref GPIO_pin_op_types */
    uint8_t GPIO_PinAltFuncMode;  /*!< Alternate function selection used when
                                       the pin is configured in alternate
                                       function mode.
                                       @ref GPIO_alt_functions */
} GPIO_PinConfig_t;


/**
 * @brief GPIO pin handle structure.
 *
 * @details
 * Associates a GPIO peripheral with its pin configuration.
 *
 * The peripheral pointer identifies the GPIO port whose registers are
 * accessed by the driver, while GPIO_PinConfig contains the settings
 * used to configure the selected pin.
 */
typedef struct
{
    GPIO_Registers_t *pGPIO_StartAddr; /*!< Base address of the GPIO port
                                            registers (e.g., GPIOA, GPIOB). */
    GPIO_PinConfig_t GPIO_PinConfig;   /*!< Configuration parameters for
                                            the selected GPIO pin. */
} GPIO_PinHndl_t;


/******************************************************************************
 * Macros
 ******************************************************************************/

/**
 * @defgroup GPIO_Pin_States GPIO Pin States
 * @brief Logical states used when writing to a GPIO output pin.
 * @{
 */

#define GPIO_PIN_RESET  0U  /*!< Set the output pin to logic low. */
#define GPIO_PIN_SET    1U  /*!< Set the output pin to logic high. */

/** @} */


/**
 * @defgroup GPIO_pin_numbers GPIO Pin Numbers
 * @brief Identifiers for the 16 GPIO pins available on each GPIO port.
 * @{
 */

#define GPIO_PIN_0      0U  /*!< GPIO pin 0.  */
#define GPIO_PIN_1      1U  /*!< GPIO pin 1.  */
#define GPIO_PIN_2      2U  /*!< GPIO pin 2.  */
#define GPIO_PIN_3      3U  /*!< GPIO pin 3.  */
#define GPIO_PIN_4      4U  /*!< GPIO pin 4.  */
#define GPIO_PIN_5      5U  /*!< GPIO pin 5.  */
#define GPIO_PIN_6      6U  /*!< GPIO pin 6.  */
#define GPIO_PIN_7      7U  /*!< GPIO pin 7.  */
#define GPIO_PIN_8      8U  /*!< GPIO pin 8.  */
#define GPIO_PIN_9      9U  /*!< GPIO pin 9.  */
#define GPIO_PIN_10     10U /*!< GPIO pin 10. */
#define GPIO_PIN_11     11U /*!< GPIO pin 11. */
#define GPIO_PIN_12     12U /*!< GPIO pin 12. */
#define GPIO_PIN_13     13U /*!< GPIO pin 13. */
#define GPIO_PIN_14     14U /*!< GPIO pin 14. */
#define GPIO_PIN_15     15U /*!< GPIO pin 15. */

/** @} */


/**
 * @defgroup GPIO_pin_modes GPIO Pin Modes
 * @brief Operating modes supported by the GPIO driver.
 * @{
 */

#define GPIO_MODE_INPUT     0U /*!< General-purpose input mode. */
#define GPIO_MODE_OUTPUT    1U /*!< General-purpose output mode. */
#define GPIO_MODE_ALTFUNC   2U /*!< Alternate function mode for connecting
                                    peripheral signals to a GPIO pin. */
#define GPIO_MODE_ANALOG    3U /*!< Analog mode, typically used for ADC or
                                    other analog peripheral functions. */
#define GPIO_MODE_IT_FT      4U /*!< Interrupt mode triggered by a falling edge. */
#define GPIO_MODE_IT_RT      5U /*!< Interrupt mode triggered by a rising edge. */
#define GPIO_MODE_IT_RFT     6U /*!< Interrupt mode triggered by both rising
                                     and falling edges. */

/** @} */


/**
 * @defgroup GPIO_pin_op_types GPIO Output Types
 * @brief Output driver configurations for GPIO pins.
 * @{
 */

#define GPIO_OTYPE_PP  0U /*!< Push-pull output: actively drives the pin
                               both high and low. */
#define GPIO_OTYPE_OD  1U /*!< Open-drain output: actively drives the pin low
                               and releases it when high. An appropriate
                               pull-up is generally required. */

/** @} */


/**
 * @defgroup GPIO_pin_speed GPIO Output Speeds
 * @brief Output transition speed settings for GPIO pins.
 * @{
 */

#define GPIO_SPEED_LOW     0U /*!< Low output speed. */
#define GPIO_SPEED_MEDIUM  1U /*!< Medium output speed. */
#define GPIO_SPEED_FAST    2U /*!< High output speed. */
#define GPIO_SPEED_HIGH    3U /*!< Very high output speed. */

/** @} */


/**
 * @defgroup GPIO_pin_pupd GPIO Pull-Up/Pull-Down Configuration
 * @brief Internal pull resistor settings for GPIO pins.
 * @{
 */

#define GPIO_NO_PUPD       0U /*!< No internal pull-up or pull-down resistor. */
#define GPIO_PIN_PU        1U /*!< Enable the internal pull-up resistor. */
#define GPIO_PIN_PD        2U /*!< Enable the internal pull-down resistor. */
#define GPIO_PIN_RESERVED  3U /*!< Reserved configuration; do not use. */

/** @} */


/******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/** @defgroup GPIO_Driver_APIs GPIO Driver APIs
 * @brief Public functions provided by the GPIO driver.
 *
 * @details
 * The API provides GPIO initialization, peripheral clock control, digital
 * input/output operations, and interrupt configuration and handling.
 * @{
 */


/******************************************************************************
 * GPIO Initialization and Deinitialization
 ******************************************************************************/
extern void GPIO_Init(GPIO_PinHndl_t *pGPIOHandle);
extern void GPIO_DeInit(GPIO_Registers_t *pGPIOx);


/******************************************************************************
 * GPIO Peripheral Clock Control
 ******************************************************************************/
extern void GPIO_PeriClkCtrl(GPIO_Registers_t *pGPIOx, uint8_t EnOrDi);


/******************************************************************************
 * GPIO Input Operations
 ******************************************************************************/
extern uint8_t GPIO_ReadInputPin(GPIO_Registers_t *pGPIOx, uint16_t PinNumber);
extern uint16_t GPIO_ReadInputPort(GPIO_Registers_t *pGPIOx);


/******************************************************************************
 * GPIO Output Operations
 ******************************************************************************/
extern void GPIO_WriteOutputPin(GPIO_Registers_t *pGPIOx,
                                uint16_t PinNumber,
                                uint8_t Value);
extern void GPIO_WriteOutputPort(GPIO_Registers_t *pGPIOx, uint16_t Value);
extern void GPIO_ToggleOutputPin(GPIO_Registers_t *pGPIOx, uint16_t PinNumber);


/******************************************************************************
 * GPIO Interrupt Configuration and Handling
 ******************************************************************************/
extern void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
extern void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
extern void GPIO_IRQHandling(uint8_t PinNumber);


/** @} */ /* End of GPIO_Driver_APIs */


#endif /* STM32F407_GPIO_DRIVER_H_ */
