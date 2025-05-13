/*
 * calibration_mode.c
 *
 * 기능 : 영구자성 옵셋 측정 후 EEProm에 저장하는 기능
 * 구성요소 : Calibration mode CSU (D-MHS-SFR-009)
 * 이력 :
 *    2024.05.23 : 이충순 : 초기 작성
 */

#include "mhs_project.h"


#define X_AXIS  0
#define Y_AXIS  1
#define Z_AXIS  2


// 회전 감지 변수
static int16_t cw_rotations;     // CW 방향 회전 횟수

static uint16_t emaCnt;          // 지연 시간을 주기 위한 카운터
static uint16_t calRotationDone; // MHS 회전 완료 플래그

static int16_t calibration_ema_filter(float64_t adc, float64_t alpha, int16_t previous_ema_filtered_value);
static void detect_rotation(float64_t sensor_value);


// 기능 : discrete_1 스위치가 LOW 이면 cal과 CBIT 을 수행하고, MHS가 2회 이상 회전 후에 eeprom 모드에 데이터 저장하고 ssm 상태를 cal 모드에서 noraml 모드로 변경한다.
//       단, CBIT 수행 시 에러가 발생 이후에는 ssm 상태를  STATUS_BCD_FW 상태로 전송한다.
// 입출력 전역변수
// float64_t Box;  //자기장 x축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
// float64_t Boy;  //자기장 y축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
// float64_t Boz;  //자기장 z축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
// mhsensor_calibration_Data.calOffsetBx;    // 자기장 x축 calibration 모드 옵셋
// mhsensor_calibration_Data.calOffsetBy;    // 자기장 y축 calibration 모드 옵셋
// mhsensor_calibration_Data.calOffsetBz;    // 자기장 z축 calibration 모드 옵셋

// 이력 :
//      2024.05.23 : 이충순 : 초기 작성
void calibrationMode(void)
{

    uint16_t ssmData = 0U;

    static uint16_t errorCbit = 0U;
    static int16_t emaBx=0;
    static int16_t firstCalRun = 0;
    static int16_t calModeCnt= 0;
    static uint16_t HeandingCnt= 0U;
    static uint16_t HeandingAngle= 0U;
    static uint16_t calOffsetDone = 0U;

	static int16_t maxB[3] = {0,};	// calibration mode에서 측정된 자기장 x축 최대값
	static int16_t minB[3] = {0,};	// calibration mode에서 측정된 자기장 x축 최소값
    int16_t getB[3] = {0,};

    static int16_t maxB_1[6] = {0,};   // calibration mode에서 측정된 자기장 x축 최대값
    static int16_t maxB_2[6] = {0,};   // calibration mode에서 측정된 자기장 x축 최대값
    float64_t error_B[6] = {0,};
    int16_t i;

    // 1. 자기장 측정, 전압, 온도를 측정
    MeasureFlux();
    MeasureVoltage();
    MeasureBoardTemperature();

    // 2. 자기장 x축을 ema 필터를 적용
    emaBx = calibration_ema_filter(Box, 0.90, emaBx);

    // 3. 각 자기장값을 버퍼에 저장
    getB[X_AXIS] = (int16_t)Box; // min/max 비교를 위하여 float을 int 형 변환
    getB[Y_AXIS] = (int16_t)Boy;
    getB[Z_AXIS] = (int16_t)Boz;

    // 4. 처음 동작 시 각축 min, max 값을 현재 값으로 설정.
    if(firstCalRun == 0)
    {
        firstCalRun = 1;

        maxB[X_AXIS] = getB[X_AXIS];
        minB[X_AXIS] = getB[X_AXIS];

        maxB[Y_AXIS] = getB[Y_AXIS];
        minB[Y_AXIS] = getB[Y_AXIS];

        maxB[Z_AXIS] = getB[Z_AXIS];
        minB[Z_AXIS] = getB[Z_AXIS];

    }


    // 5. 각 축별 현재값이 max[] 보다 크면 max[]로 저장, min[] 보다 작으면  min[]에 저장
    for(i=0; i<3; i++)
    {

        if(getB[i] >= maxB[i])
        {
           maxB[i] = getB[i];
        }

        if(minB[i] > getB[i])
        {
           minB[i] = getB[i];
        }
    }

    // 6. 1회전 진행 중이면 maxB_1[] 버퍼에 저장
    if(cw_rotations == 1)
    {
        maxB_1[0] = maxB[X_AXIS];
        maxB_1[1] = maxB[Y_AXIS];
        maxB_1[2] = maxB[Z_AXIS];
        maxB_1[3] = minB[X_AXIS];
        maxB_1[4] = minB[Y_AXIS];
        maxB_1[5] = minB[Z_AXIS];
    }

    // 7. 2회전 진행중이면 maxB_2[] 버퍼에 저장
    if(cw_rotations == 2)
    {
        maxB_2[0] = maxB[X_AXIS];
        maxB_2[1] = maxB[Y_AXIS];
        maxB_2[2] = maxB[Z_AXIS];
        maxB_2[3] = minB[X_AXIS];
        maxB_2[4] = minB[Y_AXIS];
        maxB_2[5] = minB[Z_AXIS];
    }

    // 8. 2.5초 마다 detect_rotation() 함수 호출하여 회전 감시 진행
    if(calModeCnt++ > 50) /// 2.5초 대기
    {
        calModeCnt = 0;
        // 8.1 calOffsetDone =0, 즉, cal 모드일때만 진행
        if(calOffsetDone == 0U) // cal 모드 일때만 적용,
        {
            detect_rotation((float64_t)emaBx);
        }
    }

    // 9. checkCbit()을 호출하여 CBIT 진행 후 에러 확인
    if(checkCbit() == 1U)
    {
        errorCbit = 1U;
    }

    // 10. 2회전 이상 회전 후 calOffsetDone  = 1 로 셋하면 1회전 중 각 축 최대,최소와 2회신 중 각 축 최대, 최고값의 차이값 에러 비율이 +/- 20% 이상 벗어나면
    // cbit error를 1로 설정한다.
    if(calOffsetDone == 1U)
    {
        for(i=0; i<6; i++)
        {
            error_B[i] = (float64_t)maxB_1[i] / (float64_t)maxB_2[i];

            if((1.2L < error_B[i]) || (error_B[i] < 0.8L))
            {
                errorCbit = 1U;
                calOffsetDone = 0U;
            }
        }
    }


    // 11. errorCbit 한번 이상 발생하면 cal 모드에서는 계속 STATUS_BCD_FW 전송
    if((errorCbit == 1U) || (bParameterError == 1U))
    {
        ssmData = STATUS_BCD_FW;
    }
    else
    {
        // 11.1 eeprom 데이털르 정상적으로 읽었는지 확인 후 ssm 상태 비트 전송
        // mhsensor_calibration_Data.ssm = 1 이면 STATUS_NO_COMPUT_DATA 전송
        //  mhsensor_calibration_Data.ssm  = 0 이면 STATUS_BCD_NORMAL 전송
        if(mhsensor_calibration_Data.ssm == 1U)
        {
            ssmData = STATUS_NO_COMPUT_DATA;
        }
        else
        {
            ssmData = STATUS_BCD_NORMAL;
        }
    }

    // 12. 2회전 이상 회전 후 calOffsetDone  = 1 로 셋하면 상태 모드를 NORMAL_MODE 로 설정 데이터 전송
    // 즉, 항공보정이 완료되어 정상 모드로 정작 함.
    if(calOffsetDone == 1U)
    {
        mhs_status_trans(NORMAL_MODE, ssmData);
    }
    else
    {
        // 12.1 항공 보정 모드에서는 1초 주기로 105 도 씩 더해 CAL_MODE로 데이터 전송
        if(HeandingCnt++ >= 20U) // 1초
        {
            HeandingCnt = 0U;
            HeandingAngle += 1050U;
            if(HeandingAngle >= 3600U)
            {
                HeandingAngle = HeandingAngle - 3600U;
            }

            mhsensor_data.MagHeading = HeandingAngle;
        }
        // 1.1..4. 측정값을 SFI로 전송
        mhs_status_trans(CAL_MODE, ssmData);
    }

    // 13. calRotationDone 1 이면 cal offset x,y,z 값을 eeprom 에 저장
    if(calRotationDone == 1U)
    {

    	// 13.1  CAL 완료시 EEPROM 저장하고 eerorm 데이터 crc16 체크 후 calRotationDone = 0 으로 설정하여 더 cal 모드를 완료하고 operation mode로 데이터를 전동한다.
        mhsensor_calibration_Data.calOffsetBx = (maxB[X_AXIS] + minB[X_AXIS]) / 2 ;
        mhsensor_calibration_Data.calOffsetBy = (maxB[Y_AXIS] + minB[Y_AXIS]) / 2 ;
        mhsensor_calibration_Data.calOffsetBz = (maxB[Z_AXIS] + minB[Z_AXIS]) / 2 ;

        (void)data_write_to_eeprom_calibraion(EEPROM_BX_CAL_OFFSET_ADDRESS, (uint16_t)mhsensor_calibration_Data.calOffsetBx);
        (void)data_write_to_eeprom_calibraion(EEPROM_BY_CAL_OFFSET_ADDRESS, (uint16_t)mhsensor_calibration_Data.calOffsetBy);
        (void)data_write_to_eeprom_calibraion(EEPROM_BZ_CAL_OFFSET_ADDRESS, (uint16_t)mhsensor_calibration_Data.calOffsetBz);

        writeEepromCRC16();

        calRotationDone = 0;
        cw_rotations = 0; // 초기화
        emaCnt = 0;
        firstCalRun = 0;
        calOffsetDone = 1U;  // cal 완료되면 전원 리셋 전에는 cal 하지 않음.
        mhsensor_calibration_Data.ssm = 0U;
    }


#if 0
        calsendUart(cw_rotations, emaBx);
#endif

    // 14. 다음 상태를 calibration mode 로 설정
    Can_State_Ptr = &calibrationMode;//calibration mode
}

// 기능 : calibration 모드 진행 중 회전을 확인하는 함수
// 입출력 전역변수
// float64_t sensor_value;  //자기장 x축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
// static uint16_t emaCnt;          // 일정 시간동안 지연 시간을 주기 위한 카운터;
// static int16_t cw_rotations;     // CW 방향 회전 횟수
// static uint16_t calRotationDone; // MHS 회전 완료 플래그

// 이력 :
//      2024.05.23 : 이충순 : 초기 작성
static void detect_rotation(float64_t sensor_value) {

    static uint16_t is_increasing = 0U;  // 현재 값이 증가 중인지 여부
    static int16_t first_measurement = 1; // 첫 측정 여부
    static float64_t baseline = 0.0L;     // 첫 번째 측정값 (기준값)

    // 1. 첫 번째 측정값을 baseline 기준값으로 설정
    if (first_measurement == 1)
    {

        if(emaCnt++ > 3U)
        {
            baseline = sensor_value;
            first_measurement = 0;
        }
    }
    else
    {
        // 1.1 회전 시 측정값 차이가 0.2uT 이하이면 현재 회전 중지로 판단하고 회전 감시를 하지 않고 return 시킴.
        if(fabsl(sensor_value - baseline) < 20.0L)
        {
            return;
        }
        else
        {

            // 1.1.1 센서 값이 증가한 후, 기울기가 양수이면 한 바퀴 증가 시킨 후 is_increasing 상태를 1로 설정하여 현재 회전 상태 유지
            if ((sensor_value > baseline) && (is_increasing == 0U)) {
                // 센서 값이 증가한 후, 기준값을 초과하면 한 바퀴 증가
                cw_rotations++;
                is_increasing = 1U; // 증가 상태로 바꿈
            }

            // 1.1.2 센서 값이 감소 시 기울기가 음수 임.is_increasing 상태를 0로 설정하여 기울기가 양수일 때를 대기
            if ((sensor_value < baseline) && (is_increasing == 1U)) {
                // 기준값 이하로 내려가면, 회전이 끝났음을 감지하고 증가 상태를 리셋
                is_increasing = 0;
            }

            // 1.1.3 CW 방향 2회전(720도) 감지 후 calRotationDone = 1로 설정하여 cal 완료 됨.
            if (cw_rotations >= 3) {
                calRotationDone = 1U;
            }

        }
    }

}


/*
기능설명
 지수 이동 평균(EMA, Exponential Moving Average) 필터를 적용하여 ADC 값을 필터링 하는 함수

입출력 변수
float64_t adc : 현재 측정된 ADC 값
float64_t alpha : 필터 가중 지수 값
int16_t previous_ema_filtered_value : 이전 필터된 값
*/
static int16_t calibration_ema_filter(float64_t adc, float64_t alpha, int16_t previous_ema_filtered_value)
{
    float64_t new_ema_filtered_value;

    // 1. 지수이동평균(EMA)
    // 수식 : EMAnew=(1?α)×ADC+α×EMAprev
    new_ema_filtered_value  = ((1.0 - alpha) * adc) + (alpha * (float64_t)previous_ema_filtered_value);

    // 2. 필터된 값 리턴
    return  (int16_t)new_ema_filtered_value;
}


