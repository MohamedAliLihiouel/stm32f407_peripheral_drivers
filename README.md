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

## Next Checkpoint
	CP3 — SPI Driver Development:
	Implement a polling-based SPI driver for peripheral communication.
