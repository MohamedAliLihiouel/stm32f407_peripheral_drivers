/**
 * @file    app_usart_pol_tx_rx.h
 * @brief   USART2 -> USART3 loopback test application in polling mode.
 *
 * @date    Nov 24, 2026
 * @author  Mohamed Ali, Lihiouel
 *
 * @note    Enable only ONE of the USART test applications at a time, because
 *          both define the same function names.
 */

#ifndef APP_USART_POL_TX_RX_H_
#define APP_USART_POL_TX_RX_H_

#define APP_USART_POL_TX_RX STD_OFF   /*!< STD_ON = application is compiled */

#if(APP_USART_POL_TX_RX == STD_ON)
extern void USART_ConfigGPIOPins(void);    /*!< Configure PA0, PA2, PA3, PD8, PD9      */
extern void USART_ConfigAll(void);         /*!< Clock and USART_Init() of USART2/3     */
extern void USART_PollingTx_Rx(void);      /*!< Test run (call cyclically in main)     */
#endif /* APP_USART_POL_TX_RX == STD_ON */

#endif /* APP_USART_POL_TX_RX_H_ */
