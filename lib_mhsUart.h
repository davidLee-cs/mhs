#ifndef LIB_MHSUART_H_
#define LIB_MHSUART_H_

#include "driverlib.h"
#include "device.h"
#include "board.h"

__interrupt void INT_mySCI0_RX_ISR(void);
__interrupt void INT_mySCI0_TX_ISR(void);

#endif /* LIB_MHSUART_H_ */

