/*
 * csu_fluxMeasurement.c
 *
 * 목적 : 1. 샘플인된 ADC 데이터에서 자기장값 변환 
 *       2. 변환 된 자기장값을 ARINC429 형식의 자기장 혁식으로 변환
 *
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 *
 */


#include "mhs_project.h"


#define MAG_RESOLUTION      (10000.0/32768.0L)  // range / 2^15

#define use_new_lib_funtion

float64_t Box = 0.0L;  //자기장 x축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000 
float64_t Boy = 0.0L;  //자기장 y축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000 
float64_t Boz = 0.0L;  //자기장 z축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000 

struct _mhsensor_data mhsensor_data;

static void converterToArinc429(void);
static void libConverterToArinc429(int16_t *pmag, float64_t fluxOut);

static float64_t gConstant_B[3][3] =
        { {1.0L, 0.0L, 0.0L},
          {0.0L, 1.0L, 0.0L},
          {0.0L, 0.0L, 1.0L}
        };


void flux_matrix_init(void)
{
    gConstant_B[0][0] = mhsensor_fluxrightAngle_Data.matrix_x00;
    gConstant_B[0][1] = mhsensor_fluxrightAngle_Data.matrix_x01;
    gConstant_B[0][2] = mhsensor_fluxrightAngle_Data.matrix_x02;

    gConstant_B[1][0] = mhsensor_fluxrightAngle_Data.matrix_y10;
    gConstant_B[1][1] = mhsensor_fluxrightAngle_Data.matrix_y11;
    gConstant_B[1][2] = mhsensor_fluxrightAngle_Data.matrix_y12;

    gConstant_B[2][0] = mhsensor_fluxrightAngle_Data.matrix_z20;
    gConstant_B[2][1] = mhsensor_fluxrightAngle_Data.matrix_z21;
    gConstant_B[2][2] = mhsensor_fluxrightAngle_Data.matrix_z22;
}


// adc에서 필터링 데이터를 이용하여 자기장 값으려 변환
// 입력 전역변수 : ema[ADC_CH_INDEX_FLUX_X], ema[ADC_CH_INDEX_FLUX_Y], ema[ADC_CH_INDEX_FLUX_Z], 
//              Offset_Bx, Offset_By, Offset_Bz, 
//              Gain_Bx, Gain_By, Gain_Bz, 
//              calOffsetBx, calOffsetBy, calOffsetBz
// 출력 : Box, Boy, Boz
void MeasureFlux(void)
{
    int16_t i;
    float64_t Result_calibration[3];
    float64_t gAverageADC_B[3];

    float64_t Constx;
    float64_t Consty;
    float64_t Constz;

    // 1.x,y,z 축별로 자기장값 계산 = (필터링 된 adc 값 - 옵셋) * 게인 
    int16_t fluxX0 = ema[ADC_CH_INDEX_FLUX_X] - mhsensor_calibration_Data.Offset_Bx;
    gAverageADC_B[0] = ((float64_t)(fluxX0)) * mhsensor_calibration_Data.Gain_Bx;
    int16_t fluxY0 = ema[ADC_CH_INDEX_FLUX_Y] - mhsensor_calibration_Data.Offset_By;
    gAverageADC_B[1] = ((float64_t)(fluxY0)) * mhsensor_calibration_Data.Gain_By;
    int16_t fluxZ0 = ema[ADC_CH_INDEX_FLUX_Z] - mhsensor_calibration_Data.Offset_Bz;
    gAverageADC_B[2] = ((float64_t)(fluxZ0)) * mhsensor_calibration_Data.Gain_Bz;


    // 2. x,y,z축 직각도 보정 = (x,y,z 축별로 자기장값) x (직각도 보정값 상수)
    for(i=0;i<3;i++)
    {
        Constx = (float64_t)gConstant_B[i][0]*gAverageADC_B[0];
        Consty= (float64_t)gConstant_B[i][1]*gAverageADC_B[1];
        Constz =  (float64_t)gConstant_B[i][2]*gAverageADC_B[2];
        Result_calibration[i] = Constx + Consty + Constz;
    }

    int16_t realBx = (int16_t)Result_calibration[0] - mhsensor_calibration_Data.calOffsetBx;
    int16_t realBy = (int16_t)Result_calibration[1] - mhsensor_calibration_Data.calOffsetBy;
    int16_t realBz = (int16_t)Result_calibration[2] - mhsensor_calibration_Data.calOffsetBz;

    if(realBx > 10000)
    {
        Box = 10000.0L;
    }
    else if(realBx < -10000)
    {
        Box = -10000.0L;
    }
    else
    {
        Box = (float64_t)realBx;
    }

    if(realBy > 10000)
    {
        Boy = 10000.0L;
    }
    else if(realBy < -10000)
    {
        Boy = -10000.0L;
    }
    else
    {
        Boy = (float64_t)realBy;
    }


    if(realBz > 10000)
    {
        Boz = 10000.0L;
    }
    else if(realBz < -10000)
    {
        Boz = -10000.0L;
    }
    else
    {
        Boz = (float64_t)realBz;
    }



	// 3. 측정된 자기장을 ARINC429 형식으로 변환
	converterToArinc429();
}


// 필터된 자기장 x,y,z 값을 aring429 전달하기 위한 데이터로 변환 
// 입력 전역변수 : Box, Boy, Boz
// 출력 전역변수 : mhsensor_data
static void converterToArinc429(void)
{
#ifndef  use_new_lib_funtion

    int16_t rmx, rmy, rmz;
    // 1. 직각도 보정을 한 x,y,z 자기장 값을 libConverterToArinc429() 함수를 사용하여 arinc 429 프로토콜 방식으로 데이터로 변환 후 mhsensor_data 구조체 전역변수에 저장한다.

    if(Box < 0.0f)
    {
        int16_t mx = -1*(int16_t)Box;
        if(magResolution == 0.0f)
        {
            rmx = 0;
        }
        else
        {
            float64_t magx = (float64_t)mx / MAG_RESOLUTION;
            rmx = ~((int16_t)(magx)) + 1;
        }

        mhsensor_data.Mag_x = (uint16_t)rmx & 0x3FFFU;     //14bit
        mhsensor_data.sign_mag_x = 1U;
    }
    else
    {
        int16_t mx = (int16_t)Box;
        if(magResolution == 0.0f)
        {
            rmx = 0;
        }
        else
        {
            float64_t magx = (float64_t)mx / MAG_RESOLUTION;
            rmx = ~((int16_t)(magx)) + 1;
        }

        mhsensor_data.Mag_x = (uint16_t)rmx & 0x3FFFU;     //14bit
        mhsensor_data.sign_mag_x = 0U;
    }
#else
    libConverterToArinc429(&mhsensor_data.Mag_x, Box);
#endif

#ifndef  use_new_lib_funtion
    if(Boy < 0.0f)
    {

        int16_t my = -1*(int16_t)Boy;
        if(magResolution == 0.0f)
        {
            rmy = 0;
        }
        else
        {
            float64_t magy = (float64_t)my / MAG_RESOLUTION;
            rmy = ~((int16_t)(magy)) + 1;
        }

        mhsensor_data.Mag_y = (uint16_t)rmy & 0x3FFFU;
        mhsensor_data.sign_mag_y = 1U;
    }
    else
    {
        int16_t my = (int16_t)Boy;
        if(magResolution == 0.0f)
        {
            rmy = 0;
        }
        else
        {
            float64_t magy = (float64_t)my / MAG_RESOLUTION;
            rmy = ~((int16_t)(magy)) + 1;
        }

        mhsensor_data.Mag_y = (uint16_t)rmy & 0x3FFFU;
        mhsensor_data.sign_mag_y = 0U;
    }
#else

    libConverterToArinc429(&mhsensor_data.Mag_y, Boy);

#endif

#ifndef  use_new_lib_funtion
    if(Boz < 0.0f)
    {

        int16_t mz = -1*(int16_t)Boz;
        if(magResolution == 0.0f)
        {
            rmz = 0;
        }
        else
        {
            float64_t magz = (float64_t)mz / MAG_RESOLUTION;
            rmz = ~((int16_t)(magz)) + 1;
        }

        mhsensor_data.Mag_z = (uint16_t)rmz & 0x3FFFU;
        mhsensor_data.sign_mag_z = 1U;
    }
    else
    {
        int16_t mz = (int16_t)Boz;
        if(magResolution == 0.0f)
        {
            rmz = 0;
        }
        else
        {
            float64_t magz = (float64_t)mz / MAG_RESOLUTION;
            rmz = ~((int16_t)(magz)) + 1;
        }

        mhsensor_data.Mag_z = (uint16_t)rmz & 0x3FFFU;
        mhsensor_data.sign_mag_z = 0U;
    }
#else
    libConverterToArinc429(&mhsensor_data.Mag_z, Boz);

#endif


}


/*
 기능설명
 필터된 자기장 값을 arinc429 프로토콜로 변환하는 함수

입력변수
uint16_t *pmag : mhsensor_data 구조체의 자기장 값 저장 위치 포인터
uint16_t *psignmag : mhsensor_data 구조체의 자기장 부호 위치 포인터
float64_t fluxOut : 자기장 값
*/
static void libConverterToArinc429(int16_t *pmag, float64_t fluxOut)
{
    int16_t returntMagnectic;
    float64_t flux;
    // 1. 필터된 자기장 값이 음수이면 psigmag 값을 1, 양수이면 0으로 설정, 자기장 값을 레졸류션값으로 나누어 데이터를 본환한다.
    // 변환된 자기장 값은 14이내로 표현해야 한다.

    flux = fluxOut / MAG_RESOLUTION;
    returntMagnectic = (int16_t)flux;

    *pmag  =  returntMagnectic;

}

