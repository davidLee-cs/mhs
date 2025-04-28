//#############################################################################
//
// FILE:   i2cLib_FIFO_master_interrupt.h
//
// TITLE:  C28x-I2C Library header file for FIFO interrupts
//
//#############################################################################
//#############################################################################
//
// 
// $Copyright:
// Copyright (C) 2013-2023 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

#ifndef I2CLIB_FIFO_INTERRUPT_H
#define I2CLIB_FIFO_INTERRUPT_H

#include "device.h"

//
// Error messages for read and writehandleI2C_ErrorCondition functions
//
#define ERROR_BUS_BUSY              0x1000
#define ERROR_NACK_RECEIVED         0x2000
#define ERROR_ARBITRATION_LOST      0x3000
#define ERROR_STOP_NOT_READY        0x5555
#define SUCCESS                     0x0000U

#define MAX_BUFFER_SIZE             64U
#define I2C_FIFO_LEVEL              16U

#define MAX_7_BIT_ADDRESS 127U
#define MAX_10_BIT_ADDRESS 1023U


#define EEPROM_GAIN_AX_ADDRESS      1U
#define EEPROM_GAIN_AY_ADDRESS      4U
#define EEPROM_GAIN_AZ_ADDRESS      7U
#define EEPROM_OFFSET_AX_ADDRESS    10U
#define EEPROM_OFFSET_AY_ADDRESS    13U
#define EEPROM_OFFSET_AZ_ADDRESS    16U

#define EEPROM_GAIN_BX_ADDRESS      19U
#define EEPROM_GAIN_BY_ADDRESS      22U
#define EEPROM_GAIN_BZ_ADDRESS      25U
#define EEPROM_OFFSET_BX_ADDRESS    28U
#define EEPROM_OFFSET_BY_ADDRESS    31U
#define EEPROM_OFFSET_BZ_ADDRESS    34U

#define EEPROM_BX_CAL_OFFSET_ADDRESS      37U
#define EEPROM_BY_CAL_OFFSET_ADDRESS      40U
#define EEPROM_BZ_CAL_OFFSET_ADDRESS      43U


#define EEPROM_BX00_RA_ADDRESS     46U
#define EEPROM_BX01_RA_ADDRESS      49U
#define EEPROM_BX02_RA_ADDRESS      52U

#define EEPROM_BY10_RA_ADDRESS      55U
#define EEPROM_BY11_RA_ADDRESS      58U
#define EEPROM_BY12_RA_ADDRESS      61U

#define EEPROM_BZ20_RA_ADDRESS      64U
#define EEPROM_BZ21_RA_ADDRESS      67U
#define EEPROM_BZ22_RA_ADDRESS      70U

#define EEPROM_AX00_RA_ADDRESS      73U
#define EEPROM_AX01_RA_ADDRESS      76U

#define EEPROM_AY10_RA_ADDRESS      79U
#define EEPROM_AY11_RA_ADDRESS      82U


#define EEPROM_CHK_SUM_ADDRESS      85U
#define EEPROM_VERSION_ADDRESS      88U

#define EEPROM_CHK_CRC_ADDRESS      91U


#if 0
#define EEPROM_28V_PBIT_ADDRESS     37U
#define EEPROM_5V0_PBIT_ADDRESS     40U
#define EEPROM_3V3_PBIT_ADDRESS     43U
#define EEPROM_1V2_PBIT_ADDRESS     46U
#define EEPROM_BX_PBIT_ADDRESS      49U
#define EEPROM_BY_PBIT_ADDRESS      52U
#define EEPROM_BZ_PBIT_ADDRESS      55U
#define EEPROM_AX_PBIT_ADDRESS      58U
#define EEPROM_AY_PBIT_ADDRESS      61U
#define EEPROM_28V_ID            10U
#define EEPROM_5V0_ID            11U
#define EEPROM_3V3_ID            12U
#define EEPROM_1V2_ID            13U
#define EEPROM_BX_ID             20U
#define EEPROM_BY_ID             21U
#define EEPROM_BZ_ID             22U
#define EEPROM_TEMP_ID           30U
#endif

#define EEPROM_CBIT_ADDRESS         80U
#define EEPROM_MAX_CBIT_SIZE        800
#define EEPROM_MAX_ADDR             (EEPROM_BZ_CAL_OFFSET_ADDRESS + 1U)


struct I2CHandle
{
    uint32_t base;
    uint16_t SlaveAddr;                  // Slave address tied to the message.
    uint32_t *pControlAddr;
    uint16_t NumOfAddrBytes;
    uint16_t *pTX_MsgBuffer;             // Pointer to TX message buffer
    uint16_t *pRX_MsgBuffer;             // Pointer to RX message buffer
    uint16_t NumOfDataBytes;             // Number of valid bytes in message.
    struct I2CHandle *currentHandlePtr;

    uint16_t numofSixteenByte;
    uint16_t remainingBytes;

    uint16_t WriteCycleTime_in_us;      //  Slave write cycle time. Depends on slave.
                                        //  Please check slave device datasheet
};

uint16_t verifyEEPROMRead(void);

void I2C_GPIO_init(void);
void I2Cinit(void);

extern uint16_t I2C_MasterTransmitter(struct I2CHandle *I2C_Params);
extern uint16_t I2C_MasterReceiver(struct I2CHandle *I2C_Params);

#if 0
uint16_t handleNACK(uint32_t base);
#endif

uint16_t I2CBusScan(uint32_t base, uint16_t *pAvailableI2C_slaves);
void handleI2C_ErrorCondition(struct I2CHandle *I2C_Params);
uint16_t I2C_TransmitSlaveAddress_ControlBytes(struct I2CHandle *I2C_Params);
uint16_t checkBusStatus(uint32_t base);
void Write_Read_TX_RX_FIFO(struct I2CHandle *I2C_Params);
void I2C_wait(uint32_t base);


extern void eepromInit(void);
void WaitUntilDone(void);
uint16_t data_Read(uint32_t address);
extern uint16_t TX_MsgBuffer[MAX_BUFFER_SIZE];
extern uint16_t RX_MsgBuffer[MAX_BUFFER_SIZE];
extern struct I2CHandle EEPROM;
extern uint32_t ControlAddr;

extern struct I2CHandle *currentResponderPtr;    // 현재 사용하는 I2C 인터페이스 정보
extern uint16_t status;


#endif


