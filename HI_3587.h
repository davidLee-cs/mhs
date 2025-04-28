/*
 * HI_3587.h
 *
 *  Created on: 2024. 1. 11.
 *      Author: USER
 */

#ifndef HI_3587_H_
#define HI_3587_H_


#ifndef __HI_3587_H__
#define __HI_3587_H__

/*****************************************************************************/
/***************************** Include Files *********************************/
/*****************************************************************************/
#include "HI_3587m0x.h"
#include "config.h"
#include "Typedef.h"
/*****************************************************************************/
/******************* HI_3587 Constants ****************************************/
/*****************************************************************************/
#define CRC8_POLYNOMIAL_REPRESENTATION 0x07 // x8 + x3 + x + 1

/*****************************************************************************/
/************************ Functions Declarations *****************************/
/*****************************************************************************/
/*! Reads the value of the specified register. */
int32_t HI_3587_ReadRegister(st_reg* pReg);

/*! Writes the value of the specified register. */
int32_t HI_3587_WriteRegister(st_reg* reg);

/*! Resets the device. */
int32_t HI_3587_Reset(void);

/*! Waits until a new conversion result is available. */
int32_t HI_3587_WaitForReady(uint32_t timeout);

/*! Reads the conversion result from the device. */
//int32_t HI_3587_ReadData(int32_t* pData);
//extern long HI_3587_ReadData(long* pData);
long HI_3587_ReadData(long* pData);
/*! Computes the CRC checksum for a data buffer. */
//uint16_t HI_3587_ComputeCRC8(uint8_t* pBuf, uint8_t bufSize);

/*! Computes the XOR checksum for a data buffer. */
//uint16_t HI_3587_ComputeXOR8(uint8_t * pBuf, uint8_t bufSize);

/*! Updates the CRC settings. */
void HI_3587_UpdateCRCSetting(void);

/*! Initializes the HI_3587. */
int32_t HI_3587_Setup(void);
int32_t HI_3587_Setup1(void);

uint16_t readSingleRegister(st_reg* pReg);

//int itoa(int32_t anInteger , char *str);
void doubleToString(int32_t d, char* str, int type);
void intToString(int v, char* str);


extern void HI_3587_Calibration(void);

extern int AD7177_2_ReadData(char ch);
extern void readDataCPU1(void);
extern void writeDataCPU1(int16_t *adc);





#endif /* HI_3587_H_ */
