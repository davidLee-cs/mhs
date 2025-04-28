/*
 * csu_fluxMeasurement.h
 *
 *  Created on: 2024. 10. 17.
 *      Author: Derac SON
 */

#ifndef CSU_FLUXMEASUREMENT_H_
#define CSU_FLUXMEASUREMENT_H_

#include "Typedef.h"

struct _mhsensor_data
{
    uint16_t MagHeading;		// 정북으로 부터의 각도,  degree 0.1 , 0 ~ 359.9

    int16_t Mag_x;				//자기장 x축, 단위 gauss, 정밀도 100nT , 범위    0 ~ 10000,
    int16_t Mag_y;				//자기장 y축, 단위 gauss, 정밀도 100nT , 범위    0 ~ 10000,
    int16_t Mag_z;				//자기장 z축, 단위 gauss, 정밀도 100nT , 범위    0 ~ 10000,

    int16_t Accel_x;			//가속도 x축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
    int16_t Accel_y;			//가속도 y축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g

    uint16_t sign_mag_x;		//자기장 부호 x축, 0/1
    uint16_t sign_mag_y;		//자기장 부호 y축, 0/1
    uint16_t sign_mag_z;		//자기장 부호 z축, 0/1
    uint16_t sign_accel_x;		//가속도 부호 x축, 0/1
    uint16_t sign_accel_y;		//가속도 부호 y축, 0/1

};

extern float64_t Box; 
extern float64_t Boy;
extern float64_t Boz;

extern struct _mhsensor_data mhsensor_data;

extern void MeasureFlux(void);
void flux_matrix_init(void);

#endif /* CSU_FLUXMEASUREMENT_H_ */
