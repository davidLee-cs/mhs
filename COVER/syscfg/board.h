/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef BOARD_H
#define BOARD_H

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//
// Included Files
//

#include "driverlib.h"
#include "device.h"

//*****************************************************************************
//
// PinMux Configurations
//
//*****************************************************************************
//
// GPIO14 - GPIO Settings
//
#define MR_GPIO_PIN_CONFIG GPIO_14_GPIO14
//
// GPIO13 - GPIO Settings
//
#define TFLAG_GPIO_PIN_CONFIG GPIO_13_GPIO13
//
// GPIO15 - GPIO Settings
//
#define SPI_CS_GPIO_PIN_CONFIG GPIO_15_GPIO15
//
// GPIO54 - GPIO Settings
//
#define DISCRETE_1_GPIO_PIN_CONFIG GPIO_54_GPIO54
//
// GPIO94 - GPIO Settings
//
#define CLR_WDT_GPIO_PIN_CONFIG GPIO_94_GPIO94
//
// GPIO53 - GPIO Settings
//
#define DISCRETE_2_GPIO_PIN_CONFIG GPIO_53_GPIO53

//
// SCIA -> mySCI0 Pinmux
//
//
// SCIRXDA - GPIO Settings
//
#define GPIO_PIN_SCIRXDA 28
#define mySCI0_SCIRX_GPIO 28
#define mySCI0_SCIRX_PIN_CONFIG GPIO_28_SCIRXDA
//
// SCITXDA - GPIO Settings
//
#define GPIO_PIN_SCITXDA 29
#define mySCI0_SCITX_GPIO 29
#define mySCI0_SCITX_PIN_CONFIG GPIO_29_SCITXDA

//
// SPIA -> mySPI0 Pinmux
//
//
// SPIA_PICO - GPIO Settings
//
#define GPIO_PIN_SPIA_PICO 16
#define mySPI0_SPIPICO_GPIO 16
#define mySPI0_SPIPICO_PIN_CONFIG GPIO_16_SPISIMOA
//
// SPIA_POCI - GPIO Settings
//
#define GPIO_PIN_SPIA_POCI 17
#define mySPI0_SPIPOCI_GPIO 17
#define mySPI0_SPIPOCI_PIN_CONFIG GPIO_17_SPISOMIA
//
// SPIA_CLK - GPIO Settings
//
#define GPIO_PIN_SPIA_CLK 18
#define mySPI0_SPICLK_GPIO 18
#define mySPI0_SPICLK_PIN_CONFIG GPIO_18_SPICLKA
//
// SPIA_PTE - GPIO Settings
//
#define GPIO_PIN_SPIA_PTE 19
#define mySPI0_SPIPTE_GPIO 19
#define mySPI0_SPIPTE_PIN_CONFIG GPIO_19_SPISTEA

//*****************************************************************************
//
// CPUTIMER Configurations
//
//*****************************************************************************
#define myCPUTIMER0_BASE CPUTIMER0_BASE
void myCPUTIMER0_init();

//*****************************************************************************
//
// GPIO Configurations
//
//*****************************************************************************
#define MR 14
void MR_init();
#define TFLAG 13
void TFLAG_init();
#define SPI_CS 15
void SPI_CS_init();
#define DISCRETE_1 54
void DISCRETE_1_init();
#define CLR_WDT 94
void CLR_WDT_init();
#define DISCRETE_2 53
void DISCRETE_2_init();

//*****************************************************************************
//
// INTERRUPT Configurations
//
//*****************************************************************************

// Interrupt Settings for INT_myCPUTIMER0
#define INT_myCPUTIMER0 INT_TIMER0
#define INT_myCPUTIMER0_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP1
extern __interrupt void cpuTimer0ISR(void);

// Interrupt Settings for INT_mySCI0_RX
#define INT_mySCI0_RX INT_SCIA_RX
#define INT_mySCI0_RX_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP9
extern __interrupt void INT_mySCI0_RX_ISR(void);

// Interrupt Settings for INT_mySCI0_TX
#define INT_mySCI0_TX INT_SCIA_TX
#define INT_mySCI0_TX_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP9
extern __interrupt void INT_mySCI0_TX_ISR(void);

//*****************************************************************************
//
// SCI Configurations
//
//*****************************************************************************
#define mySCI0_BASE SCIA_BASE
#define mySCI0_BAUDRATE 9600
#define mySCI0_CONFIG_WLEN SCI_CONFIG_WLEN_8
#define mySCI0_CONFIG_STOP SCI_CONFIG_STOP_ONE
#define mySCI0_CONFIG_PAR SCI_CONFIG_PAR_NONE
#define mySCI0_FIFO_TX_LVL SCI_FIFO_TX0
#define mySCI0_FIFO_RX_LVL SCI_FIFO_RX1
void mySCI0_init();

//*****************************************************************************
//
// SPI Configurations
//
//*****************************************************************************
#define mySPI0_BASE SPIA_BASE
#define mySPI0_BITRATE 25000
void mySPI0_init();

//*****************************************************************************
//
// Board Configurations
//
//*****************************************************************************
void	Board_init();
void	CPUTIMER_init();
void	GPIO_init();
void	INTERRUPT_init();
void	SCI_init();
void	SPI_init();
void	PinMux_init();

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif  // end of BOARD_H definition
