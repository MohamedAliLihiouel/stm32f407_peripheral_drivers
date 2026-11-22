## STM32F407 Driver Development
	Low-level peripheral driver development for the **STM32F407VG** on the STM32F4 Discovery board using C and STM32CubeIDE.

## CP1 — STM32CubeIDE Baseline
	- Project setup and target configuration
	- Startup code and system initialization
	- Flash/RAM linker configuration
	- Build and debug environment

## CP2 — GPIO Driver and External Interrupts
	Custom register-level GPIO driver for the ARM Cortex-M4:
	- GPIO initialization and clock control
	- Digital input/output and pin toggling
	- GPIO modes, speed, output type, and pull configuration
	- Alternate-function configuration
	- EXTI configuration and interrupt handling
	- NVIC interrupt enable/disable and priority configuration

	Applications: Button polling and button-triggered LED toggling using external interrupts.

## CP3 — SPI Driver Development
	- Implement a register-level SPI driver
	- Configure SPI mode, clock, data frame, and GPIO alternate functions
	- Implement polling-based 8-bit and 16-bit data transmission
	- Develop an SPI2 master transmit application

## CP4 – Interrupt-Driven SPI Communication
	- Implemented interrupt-driven SPI transmission and reception, including centralized interrupt handling and error management.
	- Added an SPI application demonstrating asynchronous master communication triggered by a push button, with LED-based error indication.
	- Integrated the SPI interrupt application into the main project and performed related code and project structure updates.

## CP5 – I2C driver with master TX and slave RX
	Add I2C driver header and initialization:
	- Add the I2C configuration and handle structures, register definitions, flag  masks, states and event codes. 
	- Implement initialization, de-initialization, clock control and peripheral enable.

	Add blocking I2C master transmit:
	- Implement polling master transmission with NACK, bus-error and timeout detection and optional repeated start. Add ACK control.

	Add interrupt-driven I2C master transmit and slave receive:
	- Implement interrupt-driven master transmission and slave reception, event and error handling, NVIC configuration and an application callback.

	Add I2C test applications:
	- Add test applications for polling and interrupt master transmit. 
	- I2C1 sends to I2C3 on the same board; the transfer is started with the user button and verified with logic analyzer.

	
