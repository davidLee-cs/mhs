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
    uint16_t MagHeading;		// �������� ������ ����,  degree 0.1 , 0 ~ 359.9

    int16_t Mag_x;				//�ڱ��� x��, ���� gauss, ���е� 100nT , ����    0 ~ 10000,
    int16_t Mag_y;				//�ڱ��� y��, ���� gauss, ���е� 100nT , ����    0 ~ 10000,
    int16_t Mag_z;				//�ڱ��� z��, ���� gauss, ���е� 100nT , ����    0 ~ 10000,

    int16_t Accel_x;			//���ӵ� x��, ���� g, ���е�  , ����    0 ~  +/- 1.6g
    int16_t Accel_y;			//���ӵ� y��, ���� g, ���е�  , ����    0 ~  +/- 1.6g

    uint16_t sign_mag_x;		//�ڱ��� ��ȣ x��, 0/1
    uint16_t sign_mag_y;		//�ڱ��� ��ȣ y��, 0/1
    uint16_t sign_mag_z;		//�ڱ��� ��ȣ z��, 0/1
    uint16_t sign_accel_x;		//���ӵ� ��ȣ x��, 0/1
    uint16_t sign_accel_y;		//���ӵ� ��ȣ y��, 0/1

};

extern float64_t Box; 
extern float64_t Boy;
extern float64_t Boz;

extern struct _mhsensor_data mhsensor_data;

extern void MeasureFlux(void);
void flux_matrix_init(void);

#endif /* CSU_FLUXMEASUREMENT_H_ */
