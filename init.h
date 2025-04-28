/*
 * init.h
 *
 *  Created on: 2024. 6. 13.
 *      Author: USER
 */

#ifndef INIT_H_
#define INIT_H_

//#include <stdbool.h>
#include "HI_3587_arinc.h"
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "Typedef.h"

extern void (*Can_State_Ptr)(void);        // State pointer

void  dspinit(void);
uint16_t checkCbit(void);

void delay_uS(float64_t x);
__interrupt void i2cFIFO_isr(void);
__interrupt void i2c_isr(void);

extern uint16_t gRx_done;
extern uint16_t rDataPointA[100];
extern int16_t gRx_cnt;
#endif /* INIT_H_ */
