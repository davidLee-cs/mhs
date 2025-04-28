#ifndef LIB_MHSEPWM_H_
#define LIB_MHSEPWM_H_

#include <lib_mhsAdc.h>
#include "driverlib.h"
#include "device.h"
#include "board.h"


#define EPWM_CMP_UP           1U
#define EPWM_CMP_DOWN         0U

#if 0
__interrupt void INT_myEPWM0_ISR(void);
#endif
void initEPWM(void);
#endif /* LIB_MHSEPWM_H_ */
