/**
 * @file    stm32f407_I2C_driver.h
 * @brief   Public interface for the STM32F407 I2C driver.
 *
 * @details
 * This header defines the I2C configuration and handle structures, register
 * bit-position definitions, configuration macros, flag masks, handle states,
 * event codes and the public API prototypes used to initialize and control
 * the I2C peripherals of the STM32F407 microcontroller.
 *
 * It also provides functions for blocking and interrupt-driven master
 * transmission, interrupt-driven slave reception, peripheral clock control,
 * and NVIC interrupt configuration and handling.
 *
 * @date    Nov 21, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
 */

#ifndef STM32F407_I2C_DRIVER_H_
#define STM32F407_I2C_DRIVER_H_


#include "stm32f407.h"


/******************************************************************************
 * Structures
 ******************************************************************************/

/**
 * @brief I2C peripheral configuration structure.
 *
 * @details
 * Contains the configuration parameters required to initialize an I2C
 * peripheral, including the target device address, I2C clock speed,
 * acknowledgment control, and duty cycle configuration.
 *
 * The selected parameters are applied by I2C_Init().
 */
typedef struct
{
    uint8_t I2C_DeviceAddress;  /*!< I2C device address used for communication.
                                     @ref I2C_DeviceAddress */
    uint32_t I2C_SCLSpeed;       /*!< I2C communication speed (standard mode, fast mode, or other supported mode).
                                     @ref I2C_SCLSpeed */
    uint8_t I2C_ACKControl;     /*!< Enables or disables acknowledgment (ACK) generation.
                                     @ref I2C_ACKControl */
    uint8_t I2C_DMDutyCycle;    /*!< I2C clock duty cycle configuration for fast mode.
                                     @ref I2C_DMDutyCycle */
} I2C_Config_t;


/**
 * @brief I2C handle structure.
 *
 * @details
 * Associates an I2C peripheral with its configuration and holds the run-time
 * state of interrupt-driven transfers.
 *
 * The peripheral pointer identifies the I2C peripheral whose registers are
 * accessed by the driver, while I2C_Config contains the settings used to
 * configure it. The remaining members are managed by the driver and must
 * not be changed by the application while a transfer is running.
 *
 * @note The state members are modified in the interrupt handlers.
 */
typedef struct
{
	I2C_Registers_t *pI2Cx_StartAddr;   /*!< Base address of the I2C peripheral (I2C1, I2C2, I2C3). */
	I2C_Config_t     I2C_Config;        /*!< User configuration applied by I2C_Init(). */
	/* Slave receive (interrupt mode) */
	uint8_t         *pRxBuffer;         /*!< User buffer for the received bytes. */
	uint32_t         RxLen;             /*!< Number of bytes received so far. */
	uint32_t         RxSize;            /*!< Size of the user receive buffer in bytes. */
	uint8_t          RxState;           /*!< Receive state: I2C_READY or I2C_BUSY_IN_RX. */
	/* Master transmit (interrupt mode) */
	const uint8_t   *pTxBuffer;         /*!< Pointer to the next byte to send. */
	uint32_t         TxLen;             /*!< Number of bytes still to write into DR. */
	uint8_t          TxState;           /*!< Transmit state: I2C_READY or I2C_BUSY_IN_TX. */
	uint8_t          DevAddr;           /*!< 7-bit slave address (NOT shifted). */
	uint8_t          RepeatedStart;     /*!< I2C_REPEATED_START_ON or I2C_REPEATED_START_OFF. */
} I2C_Hndl_t;

/**
 * @brief  Return codes of the I2C transfer functions.
 */
typedef enum
{
    I2C_STATUS_OK = 0U,     /*!< Transfer finished without error     */
    I2C_STATUS_NACK,        /*!< Slave did not acknowledge (AF set)  */
    I2C_STATUS_BUS_ERROR,   /*!< Bus error or arbitration lost       */
    I2C_STATUS_TIMEOUT,     /*!< Flag was not set in time            */
	I2C_STATUS_BUSY			/*!< Bus busy in RX                      */
} I2C_Status_t;

/******************************************************************************
 * Macros
 ******************************************************************************/

/* The following macros are bit POSITIONS (not masks). Use (1U << I2C_xxx)
 * to build a mask. */
/* I2C_CR1 Register Bit Position Definitions */
#define I2C_CR1_PE              (0U)   /*!< Peripheral enable */
#define I2C_CR1_SMBUS           (1U)   /*!< SMBus mode */
#define I2C_CR1_SMBTYPE         (3U)   /*!< SMBus type */
#define I2C_CR1_ENARP           (4U)   /*!< ARP enable */
#define I2C_CR1_ENPEC           (5U)   /*!< PEC calculation enable */
#define I2C_CR1_ENGC            (6U)   /*!< General call enable */
#define I2C_CR1_NOSTRETCH       (7U)   /*!< Clock stretching disable */
#define I2C_CR1_START           (8U)   /*!< Start generation */
#define I2C_CR1_STOP            (9U)   /*!< Stop generation */
#define I2C_CR1_ACK             (10U)  /*!< Acknowledge enable */
#define I2C_CR1_POS             (11U)  /*!< Acknowledge/PEC position */
#define I2C_CR1_PEC             (12U)  /*!< Packet error checking */
#define I2C_CR1_ALERT           (13U)  /*!< SMBus alert */
#define I2C_CR1_SWRST           (15U)  /*!< Software reset */


/* I2C_CR2 Register Bit Position Definitions */
#define I2C_CR2_FREQ            (0U)   /*!< Peripheral clock frequency field, bits 0-5 */
#define I2C_CR2_ITERREN         (8U)   /*!< Error interrupt enable */
#define I2C_CR2_ITEVTEN         (9U)   /*!< Event interrupt enable */
#define I2C_CR2_ITBUFEN         (10U)  /*!< Buffer interrupt enable */
#define I2C_CR2_DMAEN           (11U)  /*!< DMA requests enable */
#define I2C_CR2_LAST            (12U)  /*!< DMA last transfer */


/* I2C_SR1 Register Bit Position Definitions */
#define I2C_SR1_SB              (0U)   /*!< Start bit generated */
#define I2C_SR1_ADDR            (1U)   /*!< Address sent/matched */
#define I2C_SR1_BTF             (2U)   /*!< Byte transfer finished */
#define I2C_SR1_ADD10           (3U)   /*!< 10-bit header sent */
#define I2C_SR1_STOPF           (4U)   /*!< Stop detection */
#define I2C_SR1_RXNE            (6U)   /*!< Data register not empty (receive) */
#define I2C_SR1_TXE             (7U)   /*!< Data register empty (transmit) */
#define I2C_SR1_BERR            (8U)   /*!< Bus error */
#define I2C_SR1_ARLO            (9U)   /*!< Arbitration lost */
#define I2C_SR1_AF              (10U)  /*!< Acknowledge failure */
#define I2C_SR1_OVR             (11U)  /*!< Overrun/underrun */
#define I2C_SR1_PECERR          (12U)  /*!< PEC error */
#define I2C_SR1_TIMEOUT         (14U)  /*!< Timeout or Tlow error */
#define I2C_SR1_SMBALERT        (15U)  /*!< SMBus alert */


/* I2C_SR2 Register Bit Position Definitions */
#define I2C_SR2_MSL             (0U)   /*!< Master/slave mode */
#define I2C_SR2_BUSY            (1U)   /*!< Bus busy */
#define I2C_SR2_TRA             (2U)   /*!< Transmitter/receiver */
#define I2C_SR2_GENCALL         (4U)   /*!< General call address received */
#define I2C_SR2_SMBDEFAULT      (5U)   /*!< SMBus default address received */
#define I2C_SR2_SMBHOST         (6U)   /*!< SMBus host address received */
#define I2C_SR2_DUALF           (7U)   /*!< Dual flag (Target mode) */
#define I2C_SR2_PEC             (8U)   /*!< Packet error checking register, bits 8-15 */


/* I2C_CCR Register Bit Position Definitions */
#define I2C_CCR_CCR             (0U)   /*!< Clock control value field, bits 0-11 */
#define I2C_CCR_DUTY            (14U)  /*!< Fast-mode duty cycle */
#define I2C_CCR_FS              (15U)  /*!< I2C master mode selection: standard/fast */

/* I2C_OAR1 Register Bit Position Definitions (own address register 1) */
#define I2C_OAR1_ADD0        (0U)   /* ADD0: bit 0 of the 10-bit address (unused in 7-bit mode) */
#define I2C_OAR1_ADD71       (1U)   /* ADD[7:1] */
#define I2C_OAR1_ADD98       (8U)   /* ADD[9:8]: upper address bits (10-bit mode only) */
#define I2C_OAR1_BIT14       (14U)  /* must be kept at 1 */
#define I2C_OAR1_ADDMODE     (15U)  /* ADDMODE: 0 = 7-bit, 1 = 10-bit slave address */


/**
 * @defgroup I2C_SCLSpeed I2C Clock Speed Configuration
 * @brief Defines the supported I2C communication speeds.
 * @{
 */
#define I2C_SCL_SPEED_STANDARD_MODE       (100000U) /*!< Standard mode: up to 100 kHz */
#define I2C_SCL_SPEED_FAST_MODE           (400000U) /*!< Fast mode: up to 400 kHz */
/** @} */

/**
 * @defgroup I2C_ACKControl I2C Acknowledgment Configuration
 * @brief Enables or disables acknowledgment generation.
 * @{
 */
#define I2C_ACK_DISABLE                   (0U) /*!< Disable acknowledgment control */
#define I2C_ACK_ENABLE                    (1U) /*!< Enable acknowledgment control */
/** @} */

/**
 * @defgroup I2C_DMDutyCycle I2C Fast-Mode Duty Cycle
 * @brief Defines the duty cycle of SCL in I2C Fast mode.
 *
 * @note These settings apply when Fast mode is selected.
 * @{
 */
#define I2C_DUTY_CYCLE_2                  (0U) /*!< T_low / T_high = 2 */
#define I2C_DUTY_CYCLE_16_9               (1U) /*!< T_low / T_high = 16/9 */
/** @} */

/**
 * @defgroup I2C_Flags I2C SR1 flag masks
 * @brief Masks (NOT bit positions) for I2C_GetFlagStatus().
 * @{
 */
#define I2C_FLAG_SB        (1U << I2C_SR1_SB)       /*!< Start bit generated   */
#define I2C_FLAG_ADDR      (1U << I2C_SR1_ADDR)     /*!< Address sent/matched  */
#define I2C_FLAG_BTF       (1U << I2C_SR1_BTF)      /*!< Byte transfer finished*/
#define I2C_FLAG_RXNE      (1U << I2C_SR1_RXNE)     /*!< RX data register full */
#define I2C_FLAG_TXE       (1U << I2C_SR1_TXE)      /*!< TX data register empty*/
#define I2C_FLAG_BERR      (1U << I2C_SR1_BERR)     /*!< Bus error             */
#define I2C_FLAG_ARLO      (1U << I2C_SR1_ARLO)     /*!< Arbitration lost      */
#define I2C_FLAG_AF        (1U << I2C_SR1_AF)       /*!< Acknowledge failure   */
/** @} */

/** @brief Return values of I2C_GetFlagStatus(). */
#define I2C_FLAG_SET           (1U)
#define I2C_FLAG_RESET         (0U)

/**
 * @defgroup I2C_RepeatedStart I2C Repeated Start Option
 * @brief Selects whether a STOP is generated after a master transfer.
 * @{
 */
#define I2C_REPEATED_START_ON  (1U)   /*!< Keep the bus (no STOP) after transfer */
#define I2C_REPEATED_START_OFF (0U)   /*!< Generate STOP after transfer          */
/** @} */

/** @brief Maximum number of loop passes while the blocking functions wait for a flag. */
#define I2C_WAIT_TIMEOUT       (100000U) /*!< Loop counter limit for flag waits  */

/**
 * @defgroup I2C_States I2C Handle States
 * @brief State of a transmission/reception stored in the I2C handle.
 * @{
 */
#define I2C_READY                (0U)   /*!< No transfer in progress                */
#define I2C_BUSY_IN_RX           (1U)   /*!< Slave receive in progress              */
#define I2C_BUSY_IN_TX           (2U)   /*!< Master transmit in progress            */
/** @} */

/**
 * @defgroup I2C_Events I2C Application Events and Errors
 * @brief Event codes passed to I2C_ApplicationEventCallback().
 * @{
 */
#define I2C_EVENT_STOP_DETECTED  (0U)   /*!< Master finished the message (STOP)   */
#define I2C_EVENT_RX_BUFFER_FULL (1U)   /*!< User buffer full, further bytes lost */
#define I2C_EVENT_TX_COMPLETE    (2U)   /*!< Master: all bytes sent (STOP generated) */
#define I2C_ERROR_BERR           (3U)   /*!< Bus error                            */
#define I2C_ERROR_ARLO           (4U)   /*!< Arbitration lost                     */
#define I2C_ERROR_AF             (5U)   /*!< Acknowledge failure                  */
#define I2C_ERROR_OVR            (6U)   /*!< Overrun / underrun                   */
/** @} */

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/** @defgroup I2C_Driver_APIs I2C Driver APIs
 * @brief Public functions provided by the I2C driver.
 *
 * @details
 * The API provides I2C initialisation, peripheral clock control,
 * Send/receive I2C Data, and interrupt configuration and handling.
 * @{
 */
/******************************************************************************
 * I2C Initialisation and Deinitialisation
 ******************************************************************************/
extern void I2C_Init(I2C_Hndl_t *pI2CHandle);
extern void I2C_DeInit(I2C_Registers_t *pI2Cx);


/******************************************************************************
 * I2C Peripheral Clock Control
 ******************************************************************************/
extern void I2C_PeriClkCtrl(I2C_Registers_t *pI2Cx, uint8_t EnOrDi);


/******************************************************************************
 * I2C Send and Receive
 ******************************************************************************/
extern uint8_t      I2C_GetFlagStatus(const I2C_Registers_t *pI2Cx, uint32_t FlagMask);
extern I2C_Status_t I2C_MasterSendData(I2C_Hndl_t *pI2CHandle, const uint8_t *pTxBuffer, uint32_t Length, uint8_t SlaveAddr, uint8_t RepeatedStart);
extern I2C_Status_t I2C_MasterSendDataIT(I2C_Hndl_t *pI2CHandle, const uint8_t *pTxBuffer, uint32_t Length, uint8_t SlaveAddr, uint8_t RepeatedStart);
extern I2C_Status_t I2C_SlaveReceiveDataIT(I2C_Hndl_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Length);

/******************************************************************************
 * I2C Peripheral Control: Enable/Disable
 ******************************************************************************/
extern void I2C_PeripheralCtrl(I2C_Registers_t *pI2Cx, uint8_t EnOrDi);


/******************************************************************************
 * I2C Interrupt Configuration and Handling
 ******************************************************************************/
extern void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
extern void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
extern void I2C_EV_IRQHandling(I2C_Hndl_t *pI2CHandle);
extern void I2C_ER_IRQHandling(I2C_Hndl_t *pI2CHandle);
extern void I2C_ApplicationEventCallback(I2C_Hndl_t *pI2CHandle, uint8_t Event);


/* ACK control: must be called after I2C_PeripheralCtrl(ENABLE). Not yet part of a prototype group. */
extern void I2C_ManageAcking(I2C_Registers_t *pI2Cx, uint8_t EnOrDi);


/** @} */ /* End of I2C_Driver_APIs */


#endif /* STM32F407_I2C_DRIVER_H_ */
