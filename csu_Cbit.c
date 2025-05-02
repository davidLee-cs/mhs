/*
 * csu_Cbit.c
 *
 * 기능 : 자기장값을 측정하여 에러 범위를 검사하는 기능 
 * 구성요소 : CBIT CSU (D-MHS-SFR-012)
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */

#include <stdbool.h> 
#include "mhs_project.h"
 
static uint16_t mag_acc_check(void);
static uint16_t temp_check(void);
static uint16_t volt_check(void);

#define VOLT_28_MORE        (32.0L)
#define VOLT_28_LESS        (12.0L)

#define VOLT_5_MORE         (5.0L*1.05L)
#define VOLT_5_LESS         (5.0L*0.95L)

#define VOLT_3V3_MORE       (3.3L*1.05L)
#define VOLT_3V3_LESS       (3.3L*0.95L)

#define VOLT_1V2_MORE       (1.2L*1.05L)
#define VOLT_1V2_LESS       (1.2L*0.95L)


// 기능설명
// CBIT 에러 확인하는 함수 
//
// 출력변수
// return 값 : CBIT 오류 발생 상황 (1: 오류 발생, 0 : 정상 작동)
uint16_t checkCbit(void)
{
    uint16_t error = 0U;

	// 1. 자기장 범위 검사 후 리턴값이 확인 
	// 리턴값 : 1 (error), 0 (No error)
    if(mag_acc_check() != 0U)
    {
        error = 1U;
        arinc429_error.fluxError = 1U;
    }
    else
    {
        arinc429_error.fluxError = 0U;
    }

    if(temp_check() != 0U)
    {
        error = 1U;
        arinc429_error.tempError = 1U;
    }
    else
    {
        arinc429_error.tempError = 0U;
    }

    if(volt_check() != 0U)
    {
        error = 1U;
        arinc429_error.voltageError = 1U;
    }
    else
    {
        arinc429_error.voltageError = 0U;
    }

    return error;

}


// 기능설명
// 자기장 범위 검사하는 함수 각 x,y,z 자기장 값이 -7000 ~ +7000 gauss 범위를 벗어나면 에러 발생
//  리턴값 : 1 : error,  0: No error
//
// 출력변수
// return 값 : CBIT 오류 발생 상황 (1: 오류 발생, 0 : 정상 작동)
static uint16_t mag_acc_check(void)
{
    uint16_t error = 0U;

	// 1. 필터된 자기장 Box,Boy,Boz 축의 값을 -70 uT ~ +70 uT 범위 안에 있으면 0, 벗어나면 1로 error 을 설정 하고 error을 반환
    if((Box > 7000.0L) || (Box < -7000.0L))
    {
        error = 1U;
    }

    if((Boy > 7000.0L) || (Boy < -7000.0L))
    {
        error = 1U;
    }

    if((Boz > 7000.0L) || (Boz < -7000.0L))
    {
        error = 1U;
    }
	// 2. 에러 상태 반화 
    return error;
}

// 기능설명
// 온도 범위 검사하는 함수 값이 -40 ~ +125 도 범위를 벗어나면 에러 발생
//  리턴값 : 1 : error,  0: No error
//
// 출력변수
// return 값 :  오류 발생 상황 (1: 오류 발생, 0 : 정상 작동)
static uint16_t temp_check(void)
{
    uint16_t error = 0U;

    // 1. 온도값을 -40 ~ +125  범위 안에 있으면 0, 벗어나면 1로 error 을 설정 하고 error을 반환
    if((mhsensor_sensor_Data.tempValue > 125) || (mhsensor_sensor_Data.tempValue < -40))
    {
        error = 1U;
    }

    // 2. 에러 상태 반화
    return error;
}

// 기능설명
// 전압 범위 검사하는 함수 각 전압 1.2, 3.3, 5, 28V 값이 -5% ~ +5% 이내 범위를 벗어나면 에러 발생
//  리턴값 : 1 : error,  0: No error
//
// 출력변수
// return 값 : 오류 발생 상황 (1: 오류 발생, 0 : 정상 작동)
static uint16_t volt_check(void)
{
    uint16_t error = 0U;

    // 1. 필터된 자기장 Box,Boy,Boz 축의 값을 -70 uT ~ +70 uT 범위 안에 있으면 0, 벗어나면 1로 error 을 설정 하고 error을 반환
    if((mhsensor_sensor_Data.voltage28v > VOLT_28_MORE) || (mhsensor_sensor_Data.voltage28v < VOLT_28_LESS))
    {
        error = 1U;
    }

    if((mhsensor_sensor_Data.voltage5v > VOLT_5_MORE) || (mhsensor_sensor_Data.voltage5v < VOLT_5_LESS))
    {
        error = 1U;
    }

    if((mhsensor_sensor_Data.voltage3v3 > VOLT_3V3_MORE) || (mhsensor_sensor_Data.voltage3v3 < VOLT_3V3_LESS))
    {
        error = 1U;
    }

    if((mhsensor_sensor_Data.voltage1v2 > VOLT_1V2_MORE) || (mhsensor_sensor_Data.voltage1v2 < VOLT_1V2_LESS))
    {
        error = 1U;
    }

    // 2. 에러 상태 반화
    return error;
}

