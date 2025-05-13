/*
 * mhs_project.h
 *
 *  Created on: 2024. 10. 25.
 *      Author: Derac SON
 */

#ifndef MHS_PROJECT_H_
#define MHS_PROJECT_H_

#include "Typedef.h"

#include "init.h"

#include "lib_mhsAdc.h"
#include "lib_mhsEpwm.h"
#include "lib_mhsStd.h"
#include "lib_mhsUart.h"
#include "lib_mhsI2c.h"
#include "i2cLib_FIFO_master_interrupt.h"


#include "csu_angle_calculation.h"
#include "csu_calibrationData.h"
#include "csu_calibration_mode.h"

#include "csu_factory_mode.h"
#include "csu_operation_mode.h"
#include "csu_status_mode.h"

#include "csu_fluxMeasurement.h"
#include "csu_tempMeasurement.h"
#include "csu_voltageMeasurement.h"
#include "csu_accelMeasurement.h"

#include "csu_mhs_Status_trans.h"
#include "csu_test_status_trans.h"


#define NEW_BOARD
#define MHS_VERSION     (0xA100U)


extern int16_t filterBx;
extern int16_t filterBy;
extern int16_t filterBz;
extern int16_t filterAx;
extern int16_t filterAy;


#endif /* MHS_PROJECT_H_ */
