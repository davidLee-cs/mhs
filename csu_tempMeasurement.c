/*
 * csu_tempMeasurement.c
 *
 * 기능 : 샘플링된 ADC 데이터(MCU내부 온도 센서)에서 보드 온도값으로 변환
 * 구성요소 : 1.1.1.1. 온도 측정 CSU (D-MHS-SFR-004)
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */


#include "mhs_project.h"

#define REFERENCE_VOLT 		(3.0f) //(2.847f) // MCU 레퍼런스 전압


// adc에서 필터링 데이터를 이용하여 온도 값으로 변환
// 입력 전역변수 : 
//		필터된 입력값 : ema[ADC_CH_INDEX_VOLYAGE_TEMPERATUR] 
// 출력 : mhsensor_sensor_Data.tempValue : 온도 저장
// 출력 보드 온도  = ADC_getTemperatureC(필터된 입력값, refVolt)
void MeasureBoardTemperature(void)
{

//   1. 필터된 온도 ADC 값을 이용하여 썹시 온도로 변환한다.
    mhsensor_sensor_Data.tempValue = ADC_getTemperatureC((uint16_t)ema[ADC_CH_INDEX_VOLYAGE_TEMPERATUR], REFERENCE_VOLT);
}

