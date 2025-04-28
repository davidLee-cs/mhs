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

#include "board.h"

//*****************************************************************************
//
// Board Configurations
// Initializes the rest of the modules. 
// Call this function in your application if you wish to do all module 
// initialization.
// If you wish to not use some of the initializations, instead of the 
// Board_init use the individual Module_inits
//
//*****************************************************************************
void Board_init()
{
	EALLOW;

	PinMux_init();
	CPUTIMER_init();
	GPIO_init();
	SCI_init();
	SPI_init();
	INTERRUPT_init();

	EDIS;
}

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void PinMux_init()
{
	//
	// PinMux for modules assigned to CPU1
	//
	
	// GPIO14 -> MR Pinmux
	GPIO_setPinConfig(GPIO_14_GPIO14);
	// GPIO13 -> TFLAG Pinmux
	GPIO_setPinConfig(GPIO_13_GPIO13);
	// GPIO15 -> SPI_CS Pinmux
	GPIO_setPinConfig(GPIO_15_GPIO15);
	// GPIO54 -> DISCRETE_1 Pinmux
	GPIO_setPinConfig(GPIO_54_GPIO54);
	// GPIO94 -> CLR_WDT Pinmux
	GPIO_setPinConfig(GPIO_94_GPIO94);
	// GPIO53 -> DISCRETE_2 Pinmux
	GPIO_setPinConfig(GPIO_53_GPIO53);
	//
	// SCIA -> mySCI0 Pinmux
	//
	GPIO_setPinConfig(mySCI0_SCIRX_PIN_CONFIG);
	GPIO_setPadConfig(mySCI0_SCIRX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(mySCI0_SCIRX_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(mySCI0_SCITX_PIN_CONFIG);
	GPIO_setPadConfig(mySCI0_SCITX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(mySCI0_SCITX_GPIO, GPIO_QUAL_ASYNC);

	//
	// SPIA -> mySPI0 Pinmux
	//
	GPIO_setPinConfig(mySPI0_SPIPICO_PIN_CONFIG);
	GPIO_setPadConfig(mySPI0_SPIPICO_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(mySPI0_SPIPICO_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(mySPI0_SPIPOCI_PIN_CONFIG);
	GPIO_setPadConfig(mySPI0_SPIPOCI_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(mySPI0_SPIPOCI_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(mySPI0_SPICLK_PIN_CONFIG);
	GPIO_setPadConfig(mySPI0_SPICLK_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(mySPI0_SPICLK_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(mySPI0_SPIPTE_PIN_CONFIG);
	GPIO_setPadConfig(mySPI0_SPIPTE_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(mySPI0_SPIPTE_GPIO, GPIO_QUAL_ASYNC);


}

//*****************************************************************************
//
// CPUTIMER Configurations
//
//*****************************************************************************
void CPUTIMER_init(){
	myCPUTIMER0_init();
}

void myCPUTIMER0_init(){
	CPUTimer_setEmulationMode(myCPUTIMER0_BASE, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
	CPUTimer_setPreScaler(myCPUTIMER0_BASE, 1U);
	CPUTimer_setPeriod(myCPUTIMER0_BASE, 5000000U);
	CPUTimer_enableInterrupt(myCPUTIMER0_BASE);
	CPUTimer_stopTimer(myCPUTIMER0_BASE);

	CPUTimer_reloadTimerCounter(myCPUTIMER0_BASE);
	CPUTimer_startTimer(myCPUTIMER0_BASE);
}

//*****************************************************************************
//
// GPIO Configurations
//
//*****************************************************************************
void GPIO_init(){
	MR_init();
	TFLAG_init();
	SPI_CS_init();
	DISCRETE_1_init();
	CLR_WDT_init();
	DISCRETE_2_init();
}

void MR_init(){
	GPIO_setPadConfig(MR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(MR, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(MR, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(MR, GPIO_CORE_CPU1);
}
void TFLAG_init(){
	GPIO_setPadConfig(TFLAG, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(TFLAG, GPIO_QUAL_6SAMPLE);
	GPIO_setDirectionMode(TFLAG, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(TFLAG, GPIO_CORE_CPU1);
}
void SPI_CS_init(){
	GPIO_writePin(SPI_CS, 1);
	GPIO_setPadConfig(SPI_CS, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(SPI_CS, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(SPI_CS, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(SPI_CS, GPIO_CORE_CPU1);
}
void DISCRETE_1_init(){
	GPIO_setPadConfig(DISCRETE_1, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(DISCRETE_1, GPIO_QUAL_6SAMPLE);
	GPIO_setDirectionMode(DISCRETE_1, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(DISCRETE_1, GPIO_CORE_CPU1);
}
void CLR_WDT_init(){
	GPIO_writePin(CLR_WDT, 1);
	GPIO_setPadConfig(CLR_WDT, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(CLR_WDT, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(CLR_WDT, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(CLR_WDT, GPIO_CORE_CPU1);
}
void DISCRETE_2_init(){
	GPIO_setPadConfig(DISCRETE_2, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(DISCRETE_2, GPIO_QUAL_6SAMPLE);
	GPIO_setDirectionMode(DISCRETE_2, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(DISCRETE_2, GPIO_CORE_CPU1);
}

//*****************************************************************************
//
// INTERRUPT Configurations
//
//*****************************************************************************
void INTERRUPT_init(){
	
	// Interrupt Setings for INT_myCPUTIMER0
	Interrupt_register(INT_myCPUTIMER0, &cpuTimer0ISR);
	Interrupt_enable(INT_myCPUTIMER0);
	
	// Interrupt Setings for INT_mySCI0_RX
	Interrupt_register(INT_mySCI0_RX, &INT_mySCI0_RX_ISR);
	Interrupt_enable(INT_mySCI0_RX);
	
	// Interrupt Setings for INT_mySCI0_TX
	Interrupt_register(INT_mySCI0_TX, &INT_mySCI0_TX_ISR);
	Interrupt_disable(INT_mySCI0_TX);
}
//*****************************************************************************
//
// SCI Configurations
//
//*****************************************************************************
void SCI_init(){
	mySCI0_init();
}

void mySCI0_init(){
	SCI_clearInterruptStatus(mySCI0_BASE, SCI_INT_RXFF | SCI_INT_TXFF | SCI_INT_FE | SCI_INT_OE | SCI_INT_PE | SCI_INT_RXERR | SCI_INT_RXRDY_BRKDT | SCI_INT_TXRDY);
	SCI_clearOverflowStatus(mySCI0_BASE);
	SCI_resetTxFIFO(mySCI0_BASE);
	SCI_resetRxFIFO(mySCI0_BASE);
	SCI_resetChannels(mySCI0_BASE);
	SCI_setConfig(mySCI0_BASE, DEVICE_LSPCLK_FREQ, mySCI0_BAUDRATE, (SCI_CONFIG_WLEN_8|SCI_CONFIG_STOP_ONE|SCI_CONFIG_PAR_NONE));
	SCI_disableLoopback(mySCI0_BASE);
	SCI_performSoftwareReset(mySCI0_BASE);
	SCI_enableInterrupt(mySCI0_BASE, SCI_INT_RXFF);
	SCI_setFIFOInterruptLevel(mySCI0_BASE, SCI_FIFO_TX0, SCI_FIFO_RX1);
	SCI_enableFIFO(mySCI0_BASE);
	SCI_enableModule(mySCI0_BASE);
}

//*****************************************************************************
//
// SPI Configurations
//
//*****************************************************************************
void SPI_init(){
	mySPI0_init();
}

void mySPI0_init(){
	SPI_disableModule(mySPI0_BASE);
	SPI_setConfig(mySPI0_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL1PHA0,
				  SPI_MODE_CONTROLLER, 25000, 8);
	SPI_setPTESignalPolarity(mySPI0_BASE, SPI_PTE_ACTIVE_LOW);
	SPI_disableFIFO(mySPI0_BASE);
	SPI_disableLoopback(mySPI0_BASE);
	SPI_setEmulationMode(mySPI0_BASE, SPI_EMULATION_FREE_RUN);
	SPI_enableModule(mySPI0_BASE);
}

