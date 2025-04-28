/*
 * csu_accelMeasurement.c
 *
 * 기능 : 필터된 가속도 ADC 값을 보정한다.
 * 구성요소 : 가속도 측정 CSU (D-MHS-SFR-003)
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */


#include "mhs_project.h"

#if 0
#define ACCE_RESOLUTION     (1000.0/16384.0f)
#endif



float64_t g0x = 0.0L;  // 가속도 x축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
float64_t g0y = 0.0L;  // 가속도 y축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g

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

// adc에서 필터링 데이터를 이용하여 가속도 값으려 변환
// 입력 전역변수 : 
//		필터된 입력값 : ema[ADC_CH_INDEX_ACCEL_X], ema[ADC_CH_INDEX_ACCEL_Y], ema[ADC_CH_INDEX_ACCEL_Z] 
//		옵셋값 :Offset_Ax, Offset_Ay, Offset_Az
//		게인 : Gain_Ax, Gain_Ay, Gain_Az
// 출력 : g0x, g0y, g0z
// 출력 가속도 = (필터된 입력값 - 옵셋값) * 게인
void MeasureAccelation(void)
{
    int16_t i;

    // 교정 초기화

    float64_t gAverageADC_A[2];
    float64_t Result_calibration[2];

    float64_t Constx;
    float64_t Consty;


    // 1.x,y,z 축별로 가속도값 계산 = (필터링 된 adc 값 - 옵셋) * 게인 
    int16_t accX0 = ema[ADC_CH_INDEX_ACCEL_X] - mhsensor_calibration_Data.Offset_Ax;
    gAverageADC_A[0] = (float64_t)(accX0) * mhsensor_calibration_Data.Gain_Ax;
    int16_t accY0 = ema[ADC_CH_INDEX_ACCEL_Y] - mhsensor_calibration_Data.Offset_Ay;
    gAverageADC_A[1] = ((float64_t)(accY0)) * mhsensor_calibration_Data.Gain_Ay;


    // 2. x,y축 직각도 보정 = (x,y,축별로 가속도값) x (직각도 보정값 상수)
    for(i=0;i<2;i++)
    {
        Constx = (float64_t)gConstant_A[i][0]*gAverageADC_A[0];
        Consty = (float64_t)gConstant_A[i][1]*gAverageADC_A[1];
        Result_calibration[i] = Constx + Consty;
    }

    g0x = Result_calibration[0];
    g0y = Result_calibration[1];

}
