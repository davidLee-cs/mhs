/*
 * csu_voltageMeasurement.c
 *
 * ���� : ���ø��� ADC �����Ϳ��� ���а����� ��ȯ
 * ������� :
 * �̷� :
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 *
 *  2024.11.20
 *  - ���� ���� ���� ����
 *  - ADC offset ��� �߰�
 */

#include "mhs_project.h"

#define ADC_RESOLUTION_FACTOR_12BIT		(4096.0L)

//static float64_t voltage28v = 0.0L;   // 28V ����, ���� V, ���е� 0.1V , ����     0 ~ 28V
//static float64_t voltage5v = 0.0L;		// 5V ����, ���� V, ���е� 0.1V , ����  	   0 ~ 5V
//static float64_t voltage3v3 = 0.0L;   // 3.3V ����, ���� V, ���е� 0.1V , ����    0 ~ 3.3V
//static float64_t voltage1v2 = 0.0L;   // 1.2V ����, ���� V, ���е� 0.1V , ����    0 ~ 1.2V

// adc���� ���͸� �����͸� �̿��Ͽ� ���� ������ ��ȯ
// �Է� �������� : 
//		���͵� �Է°� : ema[ADC_CH_INDEX_VOLYAGE_28V], ema[ADC_CH_INDEX_VOLYAGE_5V], ema[ADC_CH_INDEX_VOLYAGE_3p3V], ema[ADC_CH_INDEX_VOLYAGE_1p2V] 
// ��� : voltage28v, voltage5v, voltage3v3, voltage1v2
// (R1 / (R1 + R2))  = ( measurement ADC / MAXresolution ) = rate
// Vin x rate  = measure volt
// real inpute volt = measure volt / rate 

/*
 adc���� ���͸� �����͸� �̿��Ͽ� ���� ������ ��ȯ
 �Է� �������� :
      ���͵� �Է°� : ema[ADC_CH_INDEX_VOLYAGE_28V], ema[ADC_CH_INDEX_VOLYAGE_5V], ema[ADC_CH_INDEX_VOLYAGE_3p3V], ema[ADC_CH_INDEX_VOLYAGE_1p2V]
 ��� : voltage28v, voltage5v, voltage3v3, voltage1v2

 ratio_Res = (R1 / (R1 + R2))

 ratio_Res 28V = 0.0475
 ratio_Res 5V = 0.1669
 ratio_Res 3.3V = 0.3329
 ratio_Res 1.2V = 1

 input volt  = ( measurement ADC / MAXresolution ) x 3.0 Vref
 real input volt = input Volt / ratio_Res
 *
 *
 * */
void MeasureVoltage(void)
{
	// 1. �� ���͵� ���� 28V, 5V, 3.3V, 1.2V ADC������ �����Ͽ� �������� ��ȯ�Ѵ�.
    float64_t inputVolt_28 = ((float64_t)ema[ADC_CH_INDEX_VOLYAGE_28V] / ADC_RESOLUTION_FACTOR_12BIT) * 3.0L;
    mhsensor_sensor_Data.voltage28v = inputVolt_28 / 0.0475L;

    float64_t inputVolt_5 = ((float64_t)ema[ADC_CH_INDEX_VOLYAGE_5V] / ADC_RESOLUTION_FACTOR_12BIT) * 3.0L;
    mhsensor_sensor_Data.voltage5v = inputVolt_5 / 0.1669L;

    float64_t inputVolt_3p3 = ((float64_t)ema[ADC_CH_INDEX_VOLYAGE_3p3V] / ADC_RESOLUTION_FACTOR_12BIT) * 3.0L;
    mhsensor_sensor_Data.voltage3v3 = inputVolt_3p3 / 0.3329L;

    float64_t inputVolt_1p2 = ((float64_t)ema[ADC_CH_INDEX_VOLYAGE_1p2V] / ADC_RESOLUTION_FACTOR_12BIT) * 3.0L;
    mhsensor_sensor_Data.voltage1v2 = inputVolt_1p2 / 1.0L;

}


