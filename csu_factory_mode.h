
#ifndef CSU_FACTORY_MODE_H_
#define CSU_FACTORY_MODE_H_

#include "Typedef.h"

void factory_mode(void);
void writeEepromCRC16(void);

#if 0
void sendUart(float64_t bx, float64_t by, float64_t bz, float64_t ax, float64_t ay, float64_t brx, float64_t bry, uint16_t angle);
void calsendUart(uint16_t cnt, uint16_t angle);
#endif

extern uint16_t gfirstOpen_factory;

#endif /* CSU_FACTORY_MODE_H_ */
