/**
 * @file    app_i2c_it_mst_tx_slv_rx.h
 * @brief   Public interface of the interrupt-driven I2C master transmit / slave receive test (I2C1 -> I2C3).
 *
 * @details
 * The application is compiled only if APP_I2C_IT_MST_TX_SLV_RX is set to STD_ON.
 * It provides the GPIO configuration, the I2C peripheral configuration
 * and the test function that is called from main().
 *
 * @date    Nov 22, 2026
 * @author  Mohamed Ali Lihiouel, Embedded Software Engineer
 */

#ifndef APP_I2C_IT_MST_TX_SLV_RX_H_
#define APP_I2C_IT_MST_TX_SLV_RX_H_

/* Application switch: STD_ON = compile and run this test, STD_OFF = excluded. */
#define APP_I2C_IT_MST_TX_SLV_RX STD_ON

#if(APP_I2C_IT_MST_TX_SLV_RX == STD_ON)
/* Configure the GPIO pins (PA0 button, I2C1 and I2C3 pins). */
extern void I2C_ConfigGPIOPins(void);
/* Enable the clocks and initialize I2C1 (master) and I2C3 (slave). */
extern void I2C_ConfigAll(void);
/* Wait for the button and run the transfer test. */
extern void I2C_IT_MasterTx_SlaveRx(void);
#endif /* APP_I2C_IT_MST_TX_SLV_RX == STD_ON */

#endif /* APP_I2C_IT_MST_TX_SLV_RX_H_ */
