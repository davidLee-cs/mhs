/*
 * csu_angle_calculation.h
 *
 *  Created on: 2024. 10. 24.
 *      Author: Derac SON
 */

#ifndef CSU_angle_calculation_H_
#define CSU_angle_calculation_H_

#include "Typedef.h"

#define PI                  (3.14159265359L)
#define RADIANS             (PI/180.0f)
#define ANGLE               (180.0/PI)

extern uint16_t angle_calculation(float64_t accelx,float64_t accely, float64_t bx, float64_t by, float64_t bz);


#endif /* CSU_angle_calculation_H_ */
