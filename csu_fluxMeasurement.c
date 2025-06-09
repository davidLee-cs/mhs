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


#define MAG_RESOLUTION      (32768.0L/200.0L)  // +/-range / 2^15
#define MAX_100UH           (10000)
#define MIN_100UH           (-10000)
#define MAX_120UH           (12000)
#define MIN_120UH           (-12000)


float64_t Box = 0.0L;  //자기장 x축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000 
float64_t Boy = 0.0L;  //자기장 y축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000 
float64_t Boz = 0.0L;  //자기장 z축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000 

int16_t filterBx = 0;
int16_t filterBy = 0;
int16_t filterBz = 0;
int16_t filterAx = 0;
int16_t filterAy = 0;


struct _mhsensor_data mhsensor_data;

static void converterToArinc429(void);
static void libConverterToArinc429(int16_t *pmag, float64_t fluxOut);

static float64_t gConstant_B[3][3] =
        { {1.0L, 0.0L, 0.0L},       //x축 행렬 3x3
          {0.0L, 1.0L, 0.0L},       //y축 행렬 3x3
          {0.0L, 0.0L, 1.0L}        //z축 행렬 3x3
        };



// eeprom에 저장된 자기장 축보정 행력 값을 읽어 gConstant_B 저장
// 입력 전역변수 :
//      gConstant_B[][] : 자기장 축보정값 행력 3x3 저장
// 이력 :
//    2024.05.23 : 이충순 : 초기 작성

void flux_matrix_init(void)
{
    // 1. eeprom에 읽은 각 x,y,z 축의 축보정값을 각 행력 gConstant 3x3 변수에 저장한다.
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

    int16_t realBx;
    int16_t realBy;
    int16_t realBz;

    int16_t maxBvalue;
    int16_t minBvalue;

    // 1.x,y,z 축별로 자기장값 계산 = (필터링 된 adc 값 - 옵셋) * 게인 
    int16_t fluxX0 = filterBx - mhsensor_calibration_Data.Offset_Bx;
    gAverageADC_B[0] = ((float64_t)(fluxX0)) * mhsensor_calibration_Data.Gain_Bx;
    int16_t fluxY0 = filterBy - mhsensor_calibration_Data.Offset_By;
    gAverageADC_B[1] = ((float64_t)(fluxY0)) * mhsensor_calibration_Data.Gain_By;
    int16_t fluxZ0 = filterBz - mhsensor_calibration_Data.Offset_Bz;
    gAverageADC_B[2] = ((float64_t)(fluxZ0)) * mhsensor_calibration_Data.Gain_Bz;


    // 2. x,y,z축 직각도 보정 = (x,y,z 축별로 자기장값) x (직각도 보정값 상수)
    for(i=0;i<3;i++)
    {
        Constx = (float64_t)gConstant_B[i][0]*gAverageADC_B[0];
        Consty= (float64_t)gConstant_B[i][1]*gAverageADC_B[1];
        Constz =  (float64_t)gConstant_B[i][2]*gAverageADC_B[2];
        Result_calibration[i] = Constx + Consty + Constz;
    }

    // 3. cal 모드 일때는 cal 옵셋 보정값을 제외하고 고유 자기장 값만 저장한다. 일반 모드에서는 cal 옵셋을 처리한 후 저장한다.
    if(calibration_mode == 1U)
    {
        realBx = (int16_t)Result_calibration[0];
        realBy = (int16_t)Result_calibration[1];
        realBz = (int16_t)Result_calibration[2];
    }
    else
    {
        realBx = (int16_t)Result_calibration[0] - mhsensor_calibration_Data.calOffsetBx;
        realBy = (int16_t)Result_calibration[1] - mhsensor_calibration_Data.calOffsetBy;
        realBz = (int16_t)Result_calibration[2] - mhsensor_calibration_Data.calOffsetBz;
    }


    // 4. 각 축별 최대, 최소값을 factory mode 일때는 +/-120 uT 설정,
    // operation mode와 calibration mode는 +/-100uT 로 설정
    if(factory_Fluxmode == 1U)
    {
        maxBvalue = MAX_120UH;
        minBvalue = MIN_120UH;
    }
    else
    {
        maxBvalue = MAX_100UH;
        minBvalue = MIN_100UH;
    }

    if(realBx > maxBvalue)
    {
        Box = (float64_t)maxBvalue;
    }
    else if(realBx < minBvalue)
    {
        Box = (float64_t)minBvalue;
    }
    else
    {
        Box = (float64_t)realBx;
    }

    if(realBy > maxBvalue)
    {
        Boy = (float64_t)maxBvalue;
    }
    else if(realBy < minBvalue)
    {
        Boy = (float64_t)minBvalue;
    }
    else
    {
        Boy = (float64_t)realBy;
    }

    if(realBz > maxBvalue)
    {
        Boz = (float64_t)maxBvalue;
    }
    else if(realBz < minBvalue)
    {
        Boz = (float64_t)minBvalue;
    }
    else
    {
        Boz = (float64_t)realBz;
    }

	// 5. 측정된 자기장을 ARINC429 형식으로 변환
	converterToArinc429();
}


// 필터된 자기장 x,y,z 값을 aring429 전달하기 위한 데이터로 변환 
// 입력 전역변수 : Box, Boy, Boz
// 출력 전역변수 : mhsensor_data
static void converterToArinc429(void)
{
    libConverterToArinc429(&mhsensor_data.Mag_x, Box);
    libConverterToArinc429(&mhsensor_data.Mag_y, Boy);
    libConverterToArinc429(&mhsensor_data.Mag_z, Boz);
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
    float64_t flux_uT;
    // 1. 필터된 자기장 값이 음수이면 psigmag 값을 1, 양수이면 0으로 설정, 자기장 값을 레졸류션값으로 나누어 데이터를 본환한다.
    // 변환된 자기장 값은 14이내로 표현해야 한다.

    flux_uT = fluxOut * 0.01L;

    flux = flux_uT * MAG_RESOLUTION;
    returntMagnectic = (int16_t)flux;

    *pmag  =  returntMagnectic;

}

