/*
 * csu_accelMeasurement.h
 *
 *  Created on: 2024. 10. 17.
 *      Author: Derac SON
 */

#ifndef CSU_ACCELMEASUREMENT_H_
#define CSU_ACCELMEASUREMENT_H_

extern float64_t g0x;
extern float64_t g0y;

extern void MeasureAccelation(void);
void accel_matrix_init(void);

#endif /* CSU_ACCELMEASUREMENT_H_ */
