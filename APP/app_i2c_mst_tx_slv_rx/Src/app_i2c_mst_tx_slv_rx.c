/*
 * File: app_i2c_mst_tx_slv_rx.c
 *
 * Description:
 * This file contains the I2C test application for the STM32F407 Discovery
 * board. I2C1 (master) and I2C3 (slave) are used on the same board and are
 * connected by two jumper wires (SCL and SDA). The internal pull-up
 * resistors are used, so no external pull-ups are required (100 kHz, short wires).
 *
 * Test: I2C1 sends "hello World" with the blocking function
 * I2C_MasterSendData() to I2C3. I2C3 receives the bytes in interrupt mode.
 *
 * @date    Nov 22, 2026
 * @author  Mohamed Ali, Lihiouel
 */

#include <stdint.h>
#include <string.h>
#include "app_i2c_mst_tx_slv_rx.h"
#include "stm32f407_GPIO_driver.h"
#include "stm32f407_I2C_driver.h"

#if(APP_I2C_MST_TX_SLV_RX == STD_ON)

/******************************************************************************
 * Private macros and variables
 ******************************************************************************/

/* Number of elements of a C array. */
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

/* Size of the slave receive buffer in bytes. */
#define RX_BUFFER_SIZE  (32U)

/* Slave receive buffer (filled in the I2C3 interrupt handler). */
static uint8_t           i2cRxBuffer[RX_BUFFER_SIZE];
/* Flags set in I2C_ApplicationEventCallback() (interrupt context). */
static volatile uint8_t  i2cRxDone  = 0U;   /* set in the callback (ISR)  */
static volatile uint8_t  i2cRxError = 0U;

/******************************************************************************
 * I2C test application: I2C1 (master) <-> I2C3 (slave) on the same board
 * No external pull-up resistors: internal pull-ups + open-drain are used.
 ******************************************************************************/

/*
 * 7-bit I2C addresses (range 0x08 ... 0x77; 0x00-0x07 and 0x78-0x7F are
 * reserved). The address is NOT shifted here; the driver shifts it into
 * OAR1[7:1] and the master adds the R/nW bit.
 * Master and slave must have different addresses.
 */
#define MASTER_ADDR   (0x28U)   /*!< Own address of I2C1 (master)  */
#define SLAVE_ADDR    (0x68U)   /*!< Own address of I2C3 (slave)   */

/**
 * @brief GPIO configuration table of the I2C test application.
 *
 * @details
 * Pins used by I2C1 and I2C3 (STM32F407 Discovery):
 * - PA0 : User push-button B1 (input, starts the transfer)
 * - PB6 : AF4 I2C1_SCL
 * - PB7 : AF4 I2C1_SDA
 * - PA8 : AF4 I2C3_SCL   (PH7 does not exist on the 100-pin package)
 * - PC9 : AF4 I2C3_SDA   (PH8 does not exist on the 100-pin package)
 *
 * @note I2C pins must be open-drain (OD). Without external pull-ups the
 *       internal pull-up (about 40 kOhm) is enabled. This is only suitable
 *       for 100 kHz and short wires.
 */
static GPIO_PinHndl_t gpioConfig[] =
{
	/* PA0: User Push-Button B1 (Trigger for the I2C transmission) */
	{
		.pGPIO_StartAddr = GPIOA,
		.GPIO_PinConfig =
		{
			.GPIO_PinNumber    = GPIO_PIN_0,
			.GPIO_PinMode      = GPIO_MODE_INPUT,
			.GPIO_PinSpeed     = GPIO_SPEED_HIGH,
			.GPIO_PinPuPdRCtrl = GPIO_NO_PUPD
		}
	},
    /* PB6 : I2C1_SCL */
    {
        .pGPIO_StartAddr = GPIOB,
        .GPIO_PinConfig = {
            .GPIO_PinNumber      = GPIO_PIN_6,
            .GPIO_PinMode        = GPIO_MODE_ALTFUNC,   /* alternate function  */
            .GPIO_PinSpeed       = GPIO_SPEED_MEDIUM,   /* enough for 100 kHz  */
            .GPIO_PinOPType      = GPIO_OTYPE_OD,       /* open-drain (I2C)    */
            .GPIO_PinPuPdRCtrl   = GPIO_PIN_PU,         /* internal pull-up    */
            .GPIO_PinAltFuncMode = 4                    /* AF4 = I2C1..I2C3    */
        }
    },

    /* PB7 : I2C1_SDA */
    {
        .pGPIO_StartAddr = GPIOB,
        .GPIO_PinConfig = {
            .GPIO_PinNumber      = GPIO_PIN_7,
            .GPIO_PinMode        = GPIO_MODE_ALTFUNC,
            .GPIO_PinSpeed       = GPIO_SPEED_MEDIUM,
            .GPIO_PinOPType      = GPIO_OTYPE_OD,
            .GPIO_PinPuPdRCtrl   = GPIO_PIN_PU,
            .GPIO_PinAltFuncMode = 4
        }
    },

    /* PA8 : I2C3_SCL */
    {
        .pGPIO_StartAddr = GPIOA,
        .GPIO_PinConfig = {
            .GPIO_PinNumber      = GPIO_PIN_8,
            .GPIO_PinMode        = GPIO_MODE_ALTFUNC,
            .GPIO_PinSpeed       = GPIO_SPEED_MEDIUM,
            .GPIO_PinOPType      = GPIO_OTYPE_OD,
            .GPIO_PinPuPdRCtrl   = GPIO_PIN_PU,
            .GPIO_PinAltFuncMode = 4
        }
    },

    /* PC9 : I2C3_SDA */
    {
        .pGPIO_StartAddr = GPIOC,
        .GPIO_PinConfig = {
            .GPIO_PinNumber      = GPIO_PIN_9,
            .GPIO_PinMode        = GPIO_MODE_ALTFUNC,
            .GPIO_PinSpeed       = GPIO_SPEED_MEDIUM,
            .GPIO_PinOPType      = GPIO_OTYPE_OD,
            .GPIO_PinPuPdRCtrl   = GPIO_PIN_PU,
            .GPIO_PinAltFuncMode = 4
        }
    }
};

/**
 * @brief I2C peripheral configuration table of the test application.
 *
 * @details
 * One array element per peripheral:
 * - [0] = I2C1 (master)
 * - [1] = I2C3 (slave)
 *
 * Wiring: PB6 (I2C1_SCL) <-> PA8 (I2C3_SCL), PB7 (I2C1_SDA) <-> PC9 (I2C3_SDA),
 * plus a common GND connection.
 */
static I2C_Hndl_t i2cConfig[] =
{
	/* [0] I2C1: master */
	{
		.pI2Cx_StartAddr = I2C1,
		.I2C_Config =
		{
			.I2C_DeviceAddress   = MASTER_ADDR,                  /* own 7-bit address         */
			.I2C_SCLSpeed        = I2C_SCL_SPEED_STANDARD_MODE,  /* 100 kHz (needs uint32_t!) */
			.I2C_ACKControl      = I2C_ACK_ENABLE,               /* ACK own-address matches   */
			.I2C_DMDutyCycle     = I2C_DUTY_CYCLE_2              /* only used in fast mode    */
		}
	},
	/* [1] I2C3: slave (must be enabled before the master starts) */
	{
		.pI2Cx_StartAddr = I2C3,
		.I2C_Config =
		{
			.I2C_DeviceAddress   = SLAVE_ADDR,                   /* address the master sends  */
			.I2C_SCLSpeed        = I2C_SCL_SPEED_STANDARD_MODE,
			.I2C_ACKControl      = I2C_ACK_ENABLE,               /* slave must ACK its address*/
			.I2C_DMDutyCycle     = I2C_DUTY_CYCLE_2
		}
	}
};

/**
 * @brief  Generates a blocking software delay for button debouncing.
 * @param  None
 * @retval None
 */
void delay(void)
{
	for(uint32_t i = 0 ; i < 500000 ; i ++);
}

/**
 * @brief  Configure all GPIO pins defined in the configuration table.
 *
 * @details Enables the clock for each GPIO port referenced by the
 *          configuration table and initializes the corresponding pins.
 *
 * @param  None
 * @retval None
 */
void I2C_ConfigGPIOPins(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(gpioConfig); i++)
    {
        GPIO_PeriClkCtrl(gpioConfig[i].pGPIO_StartAddr, ENABLE);
        GPIO_Init(&gpioConfig[i]);
    }
}

/**
 * @brief  Configure and initialize all I2C peripherals in the configuration table.
 *
 * @details Enables the peripheral clock for each configured I2C instance
 *          (I2C1 master, I2C3 slave) and applies its settings using I2C_Init().
 *
 *          The peripherals are NOT enabled here (PE stays 0). They are enabled
 *          in the test function, followed by I2C_ManageAcking().
 *
 * @param  None
 * @retval None
 */
void I2C_ConfigAll(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(i2cConfig); i++)
    {
    	// 1) Enable the peripheral clock, then 2) write the configuration registers.
    	I2C_PeriClkCtrl(i2cConfig[i].pI2Cx_StartAddr, ENABLE);
    	I2C_Init(&i2cConfig[i]);
    }
}

/**
 * @brief  Transmit a test message with I2C1 (master) to I2C3 (slave).
 *
 * @details Sequence:
 *          1) Slave (I2C3): enable the NVIC interrupts, register the receive
 *             buffer, enable the peripheral, then enable ACK.
 *          2) Master (I2C1): enable the peripheral, then enable ACK.
 *          3) Wait for the user button (PA0) and debounce it.
 *          4) Send "hello World" with I2C_MasterSendData() (blocking).
 *          5) Wait until the slave has seen the STOP condition.
 *          6) Wait until the bus is free and disable both peripherals.
 *
 * @param  None
 * @retval None
 *
 * @note   ACK must be enabled AFTER the peripheral is enabled (PE = 1),
 *         because hardware clears CR1.ACK while PE = 0.
 * @note   Check in the debugger: status == I2C_STATUS_OK, i2cRxDone == 1,
 *         i2cRxBuffer == "hello World" (11 bytes).
 */
void I2C_MasterTx_SlaveRx(void)
{
    const char user_data[] = "hello World";
    I2C_Status_t status;

    /* 1. Slave: interrupts in the NVIC, receive buffer, then enable + ACK. */
    I2C_IRQInterruptConfig(IRQ_NO_I2C3_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C3_ER, ENABLE);
    (void)I2C_SlaveReceiveDataIT(&i2cConfig[1], i2cRxBuffer, RX_BUFFER_SIZE);
    I2C_PeripheralCtrl(I2C3, ENABLE);   /* enable the slave first */
    I2C_ManageAcking(I2C3, ENABLE);            /* after PE = 1 ! */

    /* 2. Master: enable, ACK and send. */
    I2C_PeripheralCtrl(I2C1, ENABLE);
    I2C_ManageAcking(I2C1, ENABLE);

    /* Wait until the user push-button (PA0) is pressed. */
    while (! GPIO_ReadInputPin(GPIOA, GPIO_PIN_0) );
	// Apply software delay (~200 ms) to avoid mechanical button bouncing.
	delay();

    status = I2C_MasterSendData(&i2cConfig[0],                    /* master = I2C1  */
                                (const uint8_t *)user_data,
                                (uint32_t)strlen(user_data),
                                i2cConfig[1].I2C_Config.I2C_DeviceAddress,   /* slave address  */
                                I2C_REPEATED_START_OFF);          /* send STOP      */

    /* 3. Wait until the slave saw the STOP (with a simple timeout). */
     uint32_t timeout = 1000000U;
     while ((i2cRxDone == 0U) && (timeout-- > 0U)) { }

     /* 4. Check the result in the debugger: status == I2C_STATUS_OK,
      *    i2cRxDone == 1, i2cRxBuffer == "hello World" (11 bytes). */
     (void)status;

     while ((i2cConfig[0].pI2Cx_StartAddr->SR2 & (1U << I2C_SR2_BUSY)) != 0U) { }
     I2C_PeripheralCtrl(I2C3, DISABLE);
     I2C_PeripheralCtrl(I2C1, DISABLE);
}


/******************************************************************************
 * Interrupt service routines and callback
 ******************************************************************************/

/**
 * @brief  I2C3 event interrupt handler (slave).
 * @note   The name must match the entry in the vector table of the startup file.
 */
void I2C3_EV_IRQHandler(void)
{
	I2C_EV_IRQHandling(&i2cConfig[1]);
}
/**
 * @brief  I2C3 error interrupt handler (slave).
 */
void I2C3_ER_IRQHandler(void)
{
	I2C_ER_IRQHandling(&i2cConfig[1]);
}

/**
 * @brief  Application event callback (overrides the weak driver function).
 *
 * @details Called from the interrupt handlers of the driver.
 *          - I2C_EVENT_STOP_DETECTED: the slave received the whole message.
 *          - Any other event is an error code and is stored in i2cRxError.
 *
 * @param  pI2CHandle: Pointer to the I2C handle that reported the event.
 * @param  Event:      I2C_EVENT_xxx or I2C_ERROR_xxx code.
 * @retval None
 */
void I2C_ApplicationEventCallback(I2C_Hndl_t *pI2CHandle, uint8_t Event)
{
    (void)pI2CHandle;
    if (Event == I2C_EVENT_STOP_DETECTED)
    {
        i2cRxDone = 1U;                 /* whole message received */
    }
    else
    {
        i2cRxError = Event;             /* remember the error code */
    }
}
#endif /* APP_I2C_MST_TX_SLV_RX == STD_ON */
