STM32F407 Driver Development
Checkpoint 1 — STM32CubeIDE Baseline

1) Overview:
	This checkpoint establishes the initial STM32F407 development environment using STM32CubeIDE. 
	It serves as the clean baseline for the step-by-step implementation of low-level peripheral drivers.
	The project is intended as a learning and experimentation platform for embedded software development on ARM Cortex-M4 microcontrollers.

2) Hardware:
	* Microcontroller: STM32F407VG
	* Board: STM32F4 Discovery
	* Core: ARM Cortex-M4
	* IDE: STM32CubeIDE

3) Language: C

4) Project Scope
This checkpoint focuses on establishing a working project structure and verifying the basic build and debug workflow.
Included:
	* Initial STM32CubeIDE project
	* Startup assembly file
	* Linker scripts for Flash and RAM
	* System initialization files
	* Basic build and debug configuration

Not included:
	* Custom GPIO driver
	* Custom EXTI driver
	* SPI, UART, I2C, or CAN drivers
	* Peripheral-specific test applications

5) Project Structure:
	stm32f407_peripheral_drivers/
	├── Inc/
	├── Src/
	├── Startup/
	├── Debug/
	├── STM32F407VG_FLASH.ld
	├── STM32F407VG_RAM.ld
	└── README.md

Note: The directory structure may evolve as the project grows.

6) Build and Debug:
	* Import the project into STM32CubeIDE.
	* Select the STM32F407VG target configuration.
	* Build the project.
	* Connect the STM32F4 Discovery board using ST-LINK.
	* Flash the firmware and start a debug session.
