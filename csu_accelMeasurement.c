/*
 * csu_accelMeasurement.c
 *
 * ��� : ���͵� ���ӵ� ADC ���� �����Ѵ�.
 * ������� : ���ӵ� ���� CSU (D-MHS-SFR-003)
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */


#include "mhs_project.h"

#if 0
#define ACCE_RESOLUTION     (1000.0/16384.0f)
#endif



float64_t g0x = 0.0L;  // ���ӵ� x��, ���� g, ���е�  , ����    0 ~  +/- 1.6g
float64_t g0y = 0.0L;  // ���ӵ� y��, ���� g, ���е�  , ����    0 ~  +/- 1.6g

static float64_t gConstant_A[2][2] =
        { {1.0L, 0.0L},
          {0.0L, 1.0L}
        };


void accel_matrix_init(void)
{
    gConstant_A[0][0] = mhsensor_accelrightAngle_Data.matrix_x00;
    gConstant_A[0][1] = mhsensor_accelrightAngle_Data.matrix_x01;

    gConstant_A[1][0] = mhsensor_accelrightAngle_Data.matrix_y10;
    gConstant_A[1][1] = mhsensor_accelrightAngle_Data.matrix_y11;
}

// adc���� ���͸� �����͸� �̿��Ͽ� ���ӵ� ������ ��ȯ
// �Է� �������� : 
//		���͵� �Է°� : ema[ADC_CH_INDEX_ACCEL_X], ema[ADC_CH_INDEX_ACCEL_Y], ema[ADC_CH_INDEX_ACCEL_Z] 
//		�ɼ°� :Offset_Ax, Offset_Ay, Offset_Az
//		���� : Gain_Ax, Gain_Ay, Gain_Az
// ��� : g0x, g0y, g0z
// ��� ���ӵ� = (���͵� �Է°� - �ɼ°�) * ����
void MeasureAccelation(void)
{
    int16_t i;

    // ���� �ʱ�ȭ

    float64_t gAverageADC_A[2];
    float64_t Result_calibration[2];

    float64_t Constx;
    float64_t Consty;


    // 1.x,y,z �ະ�� ���ӵ��� ��� = (���͸� �� adc �� - �ɼ�) * ���� 
    int16_t accX0 = ema[ADC_CH_INDEX_ACCEL_X] - mhsensor_calibration_Data.Offset_Ax;
    gAverageADC_A[0] = (float64_t)(accX0) * mhsensor_calibration_Data.Gain_Ax;
    int16_t accY0 = ema[ADC_CH_INDEX_ACCEL_Y] - mhsensor_calibration_Data.Offset_Ay;
    gAverageADC_A[1] = ((float64_t)(accY0)) * mhsensor_calibration_Data.Gain_Ay;


    // 2. x,y�� ������ ���� = (x,y,�ະ�� ���ӵ���) x (������ ������ ���)
    for(i=0;i<2;i++)
    {
        Constx = (float64_t)gConstant_A[i][0]*gAverageADC_A[0];
        Consty = (float64_t)gConstant_A[i][1]*gAverageADC_A[1];
        Result_calibration[i] = Constx + Consty;
    }

    g0x = Result_calibration[0];
    g0y = Result_calibration[1];

}
