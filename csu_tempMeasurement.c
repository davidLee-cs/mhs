/*
 * csu_tempMeasurement.c
 *
 * ��� : ���ø��� ADC ������(MCU���� �µ� ����)���� ���� �µ������� ��ȯ
 * ������� : 1.1.1.1. �µ� ���� CSU (D-MHS-SFR-004)
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */


#include "mhs_project.h"

#define REFERENCE_VOLT 		(3.0f) //(2.847f) // MCU ���۷��� ����


// adc���� ���͸� �����͸� �̿��Ͽ� �µ� ������ ��ȯ
// �Է� �������� : 
//		���͵� �Է°� : ema[ADC_CH_INDEX_VOLYAGE_TEMPERATUR] 
// ��� : mhsensor_sensor_Data.tempValue : �µ� ����
// ��� ���� �µ�  = ADC_getTemperatureC(���͵� �Է°�, refVolt)
void MeasureBoardTemperature(void)
{

//   1. ���͵� �µ� ADC ���� �̿��Ͽ� ��� �µ��� ��ȯ�Ѵ�.
    mhsensor_sensor_Data.tempValue = ADC_getTemperatureC((uint16_t)ema[ADC_CH_INDEX_VOLYAGE_TEMPERATUR], REFERENCE_VOLT);
}

