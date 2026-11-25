/**
 * @file    stm32f407_USART_driver.h
 * @brief   Public interface for the STM32F407 USART driver.
 *
 * @details
 * This header defines the USART configuration and handle structures, register
 * bit-position definitions, configuration macros, flag masks, handle states,
 * event codes and the public API prototypes used to initialize and control
 * the USART peripherals of the STM32F407 microcontroller.
 *
 * It also provides functions for blocking (polling) and interrupt-driven
 * transmission and reception, peripheral clock control, and NVIC interrupt
 * configuration and handling.
 *
 * @date    Nov 24, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
 */

#ifndef STM32F407_USART_DRIVER_H_
#define STM32F407_USART_DRIVER_H_

#include "stm32f407.h"

/******************************************************************************
 * Structures
 ******************************************************************************/

/**
 * @brief USART peripheral configuration structure.
 *
 * @details
 * Contains the configuration parameters required to initialise a USART
 * peripheral, including the transfer mode, baud rate, number of stop bits,
 * word length, parity control and hardware flow control.
 *
 * The selected parameters are applied by USART_Init().
 */
typedef struct
{
    uint8_t  USART_Mode;          /*!< Transfer direction: TX only, RX only or TX and RX.
                                       @ref USART_Mode */
    uint32_t USART_BaudRate;      /*!< Baud rate in bit/s (e.g. 115200).
                                       @ref USART_BaudRate */
    uint8_t  USART_NoOfStopBits;  /*!< Number of stop bits (1, 0.5, 2 or 1.5).
                                       @ref USART_NoOfStopBits */
    uint8_t  USART_WordLength;    /*!< Word length: 8 or 9 data bits (incl. parity bit if enabled).
                                       @ref USART_WordLength */
    uint8_t  USART_ParityControl; /*!< Parity control: disabled, even or odd.
                                       @ref USART_ParityControl */
    uint8_t  USART_HWFlowControl; /*!< Hardware flow control: none, CTS, RTS or CTS and RTS.
                                       @ref USART_HWFlowControl */
} USART_Config_t;


/**
 * @brief USART handle structure.
 *
 * @details
 * Associates a USART peripheral with its configuration and holds the
 * run-time state of interrupt-driven transfers.
 *
 * The peripheral pointer identifies the USART peripheral whose registers are
 * accessed by the driver, while USART_Config contains the settings used to
 * configure it. The remaining members are managed by the driver and must
 * not be changed by the application while a transfer is running.
 *
 * @note The length and state members are modified in the interrupt handler,
 *       therefore they are volatile.
 */
typedef struct
{
    USART_Registers_t *pUSARTx_StartAddr; /*!< Base address of the USART peripheral (e.g. USART1, USART2). */
    USART_Config_t     USART_Config;      /*!< Configuration parameters for the selected USART peripheral. */
	uint8_t *pTxBuffer;      /*!< Pointer to the next byte to send. */
	uint8_t *pRxBuffer;      /*!< Pointer to the next free byte of the RX buffer. */
	volatile uint32_t TxLen;          /*!< Number of bytes still to send.                     */
	volatile uint32_t RxLen;          /*!< Number of bytes still to receive.                  */
	volatile uint8_t  TxBusyState;    /*!< Transmit state: USART_READY or USART_BUSY_IN_TX.   */
	volatile uint8_t  RxBusyState;    /*!< Receive state: USART_READY or USART_BUSY_IN_RX. */
} USART_Hndl_t;


/******************************************************************************
 * Macros
 ******************************************************************************/

/* The following macros are bit POSITIONS (not masks). Use (1U << USART_xxx)
 * to build a mask. */
/* USART_CR1 Register Bit Position Definitions */
#define USART_CR1_SBK           (0U)   /*!< Send break                         */
#define USART_CR1_RWU           (1U)   /*!< Receiver wakeup                    */
#define USART_CR1_RE            (2U)   /*!< Receiver enable                    */
#define USART_CR1_TE            (3U)   /*!< Transmitter enable                 */
#define USART_CR1_IDLEIE        (4U)   /*!< IDLE interrupt enable              */
#define USART_CR1_RXNEIE        (5U)   /*!< RXNE interrupt enable              */
#define USART_CR1_TCIE          (6U)   /*!< Transmission complete interrupt enable */
#define USART_CR1_TXEIE         (7U)   /*!< TXE interrupt enable               */
#define USART_CR1_PEIE          (8U)   /*!< PE interrupt enable                */
#define USART_CR1_PS            (9U)   /*!< Parity selection (0 = even, 1 = odd) */
#define USART_CR1_PCE           (10U)  /*!< Parity control enable              */
#define USART_CR1_WAKE          (11U)  /*!< Wakeup method                      */
#define USART_CR1_M             (12U)  /*!< Word length (0 = 8 bit, 1 = 9 bit) */
#define USART_CR1_UE            (13U)  /*!< USART enable                       */
#define USART_CR1_OVER8         (15U)  /*!< Oversampling mode (0 = 16, 1 = 8)  */

/* USART_CR2 Register Bit Position Definitions */
#define USART_CR2_ADD           (0U)   /*!< Address of the USART node, bits 3:0 */
#define USART_CR2_LBDL          (5U)   /*!< LIN break detection length         */
#define USART_CR2_LBDIE         (6U)   /*!< LIN break detection interrupt enable */
#define USART_CR2_LBCL          (8U)   /*!< Last bit clock pulse               */
#define USART_CR2_CPHA          (9U)   /*!< Clock phase                        */
#define USART_CR2_CPOL          (10U)  /*!< Clock polarity                     */
#define USART_CR2_CLKEN         (11U)  /*!< Clock enable                       */
#define USART_CR2_STOP          (12U)  /*!< Stop bits field, bits 13:12        */
#define USART_CR2_LINEN         (14U)  /*!< LIN mode enable                    */

/* USART_CR3 Register Bit Position Definitions */
#define USART_CR3_EIE           (0U)   /*!< Error interrupt enable             */
#define USART_CR3_IREN          (1U)   /*!< IrDA mode enable                   */
#define USART_CR3_IRLP          (2U)   /*!< IrDA low-power                     */
#define USART_CR3_HDSEL         (3U)   /*!< Half-duplex selection              */
#define USART_CR3_NACK          (4U)   /*!< Smartcard NACK enable              */
#define USART_CR3_SCEN          (5U)   /*!< Smartcard mode enable              */
#define USART_CR3_DMAR          (6U)   /*!< DMA enable receiver                */
#define USART_CR3_DMAT          (7U)   /*!< DMA enable transmitter             */
#define USART_CR3_RTSE          (8U)   /*!< RTS enable                         */
#define USART_CR3_CTSE          (9U)   /*!< CTS enable                         */
#define USART_CR3_CTSIE         (10U)  /*!< CTS interrupt enable               */
#define USART_CR3_ONEBIT        (11U)  /*!< One sample bit method enable       */

/**
 * @defgroup USART_SR_Bits USART SR bit positions
 * @brief Bit POSITIONS in USART_SR. Use (1U << USART_SR_xxx) to build a mask.
 * @{
 */
#define USART_SR_PE       (0U)   /*!< Parity error                   */
#define USART_SR_FE       (1U)   /*!< Framing error                  */
#define USART_SR_NF       (2U)   /*!< Noise detected flag            */
#define USART_SR_ORE      (3U)   /*!< Overrun error                  */
#define USART_SR_IDLE     (4U)   /*!< IDLE line detected             */
#define USART_SR_RXNE     (5U)   /*!< Read data register not empty   */
#define USART_SR_TC       (6U)   /*!< Transmission complete          */
#define USART_SR_TXE      (7U)   /*!< Transmit data register empty   */
#define USART_SR_LBD      (8U)   /*!< LIN break detection            */
#define USART_SR_CTS      (9U)   /*!< CTS flag                       */
/** @} */

/**
 * @defgroup USART_Flags USART SR flag masks
 * @brief Masks (NOT bit positions) for USART_GetFlagStatus() and USART_ClearFlag().
 * @{
 */
#define USART_FLAG_PE     (1U << USART_SR_PE)     /*!< Parity error                 */
#define USART_FLAG_FE     (1U << USART_SR_FE)     /*!< Framing error                */
#define USART_FLAG_NF     (1U << USART_SR_NF)     /*!< Noise detected               */
#define USART_FLAG_ORE    (1U << USART_SR_ORE)    /*!< Overrun error                */
#define USART_FLAG_IDLE   (1U << USART_SR_IDLE)   /*!< IDLE line detected           */
#define USART_FLAG_RXNE   (1U << USART_SR_RXNE)   /*!< RX data register not empty   */
#define USART_FLAG_TC     (1U << USART_SR_TC)     /*!< Transmission complete        */
#define USART_FLAG_TXE    (1U << USART_SR_TXE)    /*!< TX data register empty       */
#define USART_FLAG_LBD    (1U << USART_SR_LBD)    /*!< LIN break detected           */
#define USART_FLAG_CTS    (1U << USART_SR_CTS)    /*!< CTS flag                     */
/** @} */

/** @brief Return values of USART_GetFlagStatus(). */
#define USART_FLAG_SET    (1U)
#define USART_FLAG_RESET  (0U)

/**
 * @defgroup USART_Mode USART Transfer Mode
 * @brief Selects the transfer direction (CR1.TE / CR1.RE).
 * @note  The driver maps these values to the TE and RE bits.
 * @{
 */
#define USART_MODE_ONLY_TX        (0U) /*!< Transmitter only (TE = 1, RE = 0) */
#define USART_MODE_ONLY_RX        (1U) /*!< Receiver only (TE = 0, RE = 1)    */
#define USART_MODE_TXRX           (2U) /*!< Transmitter and receiver          */
/** @} */

/**
 * @defgroup USART_BaudRate USART Baud Rate
 * @brief Common baud rates in bit/s.
 * @note  The driver calculates USARTDIV (BRR) from PCLK and this value.
 *        USART1 and USART6 are on APB2, USART2/3 and UART4/5 on APB1.
 * @{
 */
#define USART_STD_BAUD_1200       (1200U)
#define USART_STD_BAUD_2400       (2400U)
#define USART_STD_BAUD_9600       (9600U)
#define USART_STD_BAUD_19200      (19200U)
#define USART_STD_BAUD_38400      (38400U)
#define USART_STD_BAUD_57600      (57600U)
#define USART_STD_BAUD_115200     (115200U)
#define USART_STD_BAUD_230400     (230400U)
#define USART_STD_BAUD_460800     (460800U)
#define USART_STD_BAUD_921600     (921600U)
/** @} */

/**
 * @defgroup USART_NoOfStopBits USART Number of Stop Bits
 * @brief Values are identical to the CR2.STOP[1:0] field.
 * @note  0.5 and 1.5 stop bits are only available in Smartcard mode and
 *        are not supported by UART4 and UART5.
 * @{
 */
#define USART_STOPBITS_1          (0U) /*!< 1 stop bit   (STOP = 00) */
#define USART_STOPBITS_0_5        (1U) /*!< 0.5 stop bit (STOP = 01) */
#define USART_STOPBITS_2          (2U) /*!< 2 stop bits  (STOP = 10) */
#define USART_STOPBITS_1_5        (3U) /*!< 1.5 stop bits (STOP = 11) */
/** @} */

/**
 * @defgroup USART_WordLength USART Word Length
 * @brief Values are identical to the CR1.M bit.
 * @{
 */
#define USART_WORDLEN_8BITS       (0U) /*!< 8 data bits (M = 0) */
#define USART_WORDLEN_9BITS       (1U) /*!< 9 data bits (M = 1) */
/** @} */

/**
 * @defgroup USART_ParityControl USART Parity Control
 * @brief Selects the parity (CR1.PCE and CR1.PS).
 * @note  With parity enabled the parity bit is part of the word:
 *        8 bit word = 7 data bits + parity, 9 bit word = 8 data bits + parity.
 * @{
 */
#define USART_PARITY_DISABLE      (0U) /*!< No parity       (PCE = 0)           */
#define USART_PARITY_EVEN         (1U) /*!< Even parity     (PCE = 1, PS = 0)   */
#define USART_PARITY_ODD          (2U) /*!< Odd parity      (PCE = 1, PS = 1)   */
/** @} */

/**
 * @defgroup USART_HWFlowControl USART Hardware Flow Control
 * @brief Selects the hardware flow control (CR3.CTSE and CR3.RTSE).
 * @note  UART4 and UART5 have no CTS/RTS pins.
 * @{
 */
#define USART_HW_FLOW_CTRL_NONE     (0U) /*!< No flow control                */
#define USART_HW_FLOW_CTRL_CTS      (1U) /*!< CTS only (CTSE = 1)            */
#define USART_HW_FLOW_CTRL_RTS      (2U) /*!< RTS only (RTSE = 1)            */
#define USART_HW_FLOW_CTRL_CTS_RTS  (3U) /*!< CTS and RTS (CTSE = RTSE = 1)  */
/** @} */

/* USART_BRR Register Bit Position Definitions */
#define USART_BRR_DIV_FRACTION   (0U)   /*!< DIV_Fraction field, bits 3:0  */
#define USART_BRR_DIV_MANTISSA   (4U)   /*!< DIV_Mantissa field, bits 15:4 */

/**
 * @defgroup USART_States USART Handle States
 * @brief State of a transmission/reception stored in the USART handle.
 * @{
 */
#define USART_BUSY_IN_RX   (1U)   /*!< Reception in progress   */
#define USART_BUSY_IN_TX   (2U)   /*!< Transmission in progress */
#define USART_READY        (0U)   /*!< No transfer in progress  */
/** @} */


/**
 * @defgroup USART_Events USART Application Events and Errors
 * @brief Event codes passed to USART_ApplicationEventCallback().
 * @{
 */
#define USART_EVENT_TX_CMPLT   (0U)   /*!< All bytes were sent (TC = 1)             */
#define USART_EVENT_RX_CMPLT   (1U)   /*!< All requested bytes were received        */
#define USART_EVENT_IDLE       (2U)   /*!< IDLE line detected (not used yet)        */
#define USART_EVENT_CTS        (3U)   /*!< CTS flag changed (not used yet)          */
#define USART_EVENT_PE         (4U)   /*!< Parity error (not used yet)              */
#define USART_ERR_FE           (5U)   /*!< Framing error                            */
#define USART_ERR_NE           (6U)   /*!< Noise error                              */
#define USART_ERR_ORE          (7U)   /*!< Overrun error: a received byte was lost  */
/** @} */


/******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/** @defgroup USART_Driver_APIs USART Driver APIs
 * @brief Public functions provided by the USART driver.
 *
 * @details
 * The API provides USART initialization, peripheral clock control,
 * blocking and interrupt-driven data transfer, peripheral control,
 * and NVIC/USART interrupt configuration and handling.
 * @{
 */
/******************************************************************************
 * USART Initialisation and Deinitialisation
 ******************************************************************************/
extern void USART_Init(USART_Hndl_t *pUSARTHandle);
extern void USART_DeInit(USART_Registers_t *pUSARTx);


/******************************************************************************
 * USART Peripheral Clock Control
 ******************************************************************************/
extern void USART_PeriClkCtrl(USART_Registers_t *pUSARTx, uint8_t EnOrDi);


/******************************************************************************
 * USART Send and Receive (Blocking & Interrupt-Driven)
 ******************************************************************************/
extern void USART_SendData(USART_Hndl_t *pUSARTHandle, const uint8_t *pTxBuffer, uint32_t Length);
extern void USART_ReceiveData(USART_Hndl_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Length);
extern uint8_t USART_SendDataIT(USART_Hndl_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Length);
extern uint8_t USART_ReceiveDataIT(USART_Hndl_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Length);

/******************************************************************************
 * USART Get/Clear flag status
 ******************************************************************************/
extern uint8_t USART_GetFlagStatus(const USART_Registers_t *pUSARTx, uint32_t FlagMask);
extern void USART_ClearFlag(USART_Registers_t *pUSARTx, uint8_t FlagMask);

/******************************************************************************
 * USART Peripheral Control: Enable/Disable
 ******************************************************************************/
extern void USART_PeripheralCtrl(USART_Registers_t *pUSARTx, uint8_t EnOrDi);

/******************************************************************************
 * USART Interrupt Configuration and Handling
 ******************************************************************************/
extern void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
extern void USART_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
extern void USART_IRQHandling(USART_Hndl_t *pUSARTHandle);

/******************************************************************************
 * USART Application Callbacks
 ******************************************************************************/
extern void USART_ApplicationEventCallback(USART_Hndl_t *pUSARTHandle, uint8_t Event);

/** @} */ /* End of USART_Driver_APIs */

#endif /* STM32F407_USART_DRIVER_H_ */
