/*
 * operation_mode.c
 *
 * 기능 : 주기적(50ms)으로 자기장, 가속도, 전압, 온도를 측정하고, 자기장와 가속도를 이용하여 방위각 계산 후 SFI 에 데이터를 전송한다.
 * 구성요소 : Operation mode (D- )
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */

#include <stdbool.h>
#include "mhs_project.h"


/*
기능설명
주기적(50ms)으로 자기장, 가속도, 전압, 온도를 측정하고, 자기장과 가속도를 이용하여 방위각 계산 후 SFI 에 데이터를 전송한다.

입출력 전역변수
float64_t g0x;   가속도 x축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
float64_t g0y;   가속도 y축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
float64_t g0z;   가속도 z축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
float64_t Box;   자기장 x축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
float64_t Boy;   자기장 y축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
float64_t Boz;   자기장 z축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
uint16_t StatusMatrix;  C_bit 수행 중 에러 혹은 Parameter 가져오기 오류 발생 상태. -
(1: Error State, 0: No Error )
*/
void operation_mode(void)
{

	uint16_t AngleAy;

	uint16_t ssmData = 1U;


	// 1. interrupt 으로 수신한 filtered  된 ADC값을 자기장, 가속도, 전압, 온도로 변환
    MeasureFlux();
    MeasureAccelation();
    MeasureVoltage();
    MeasureBoardTemperature();

	// 2. 방위각 계산
    if (( isfinite( g0x ) !=0 ) && ( isfinite( g0y ) !=0 ) && ( isfinite( Box ) !=0 ) && ( isfinite( Boy ) !=0 ) && ( isfinite( Boz ) !=0 ))
    {
        AngleAy = angle_calculation(g0x,g0y,Box,Boy,Boz);
        mhsensor_data.MagHeading = AngleAy;
    }

	// 3. CBIT 수행
    uint16_t errorCbit =  checkCbit();
    if((errorCbit == 1U) || (bParameterError == 1U))
    {
        ssmData = STATUS_BCD_FW;
    }
    else
    {
        ssmData = STATUS_BCD_NORMAL;
    }

    if(mhsensor_calibration_Data.ssm == 1U)
    {
        ssmData = STATUS_NO_COMPUT_DATA;
    }

	// 4. 상태 값을 SFI로 전송
    mhs_status_trans(NORMAL_MODE, ssmData);

#if 1
    sendUart(Box, Boy, Boz, g0x, g0y, gFluxrx, gFluxry, AngleAy);
#endif

}

