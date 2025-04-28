/*
 * lib.mhsAdc.c
 *
 * 기능 :ADC 초기설정,  자기장, 가속도, 전압, 온도 측정 및 필터 적용
 * 구성요소 : 별첨
 * 이력 :
 *    2024.05.23 : 이충순 : 초기 작성
 *
 *
 * */
 
#include "mhs_project.h"

static int16_t ema_filter(float64_t adc, float64_t alpha, int16_t previous_ema_filtered_value);
static void initValue(void);


int16_t ema[ADC_CH_INDEX_MAX] = {0};	// 지수이동평균(EMA)필터된 값을 저장하는 버퍼 


#define EMA_FILTER_ALPHA        (0.7L)
#define EMA_FILTER_ALPHA_VOLT   (0.9L)

/*
기능설명
자기장, 가속도 센서 보정값 초기화 함수

입출력 전역변수
float64_t mhsensor_calibration_Data.Gain_Ax;   가속도 x축 이득
float64_t mhsensor_calibration_Data.Gain_Ay;   가속도 y축 이득
float64_t mhsensor_calibration_Data.Gain_Az;   가속도 z축 이득
int16_t  mhsensor_calibration_Data.Offset_Ax;  가속도 x축 옵셋
int16_t  mhsensor_calibration_Data.Offset_Ay;  가속도 y축 옵셋
int16_t  mhsensor_calibration_Data.Offset_Az;  가속도 z축 옵셋
float64_t mhsensor_calibration_Data.Gain_Bx;   자기장 x축 이득
float64_t mhsensor_calibration_Data.Gain_By;   자기장 y축 이득
float64_t mhsensor_calibration_Data.Gain_Bz;   자기장 z축 이득
int16_t  mhsensor_calibration_Data.Offset_Bx;  자기장 x축 옵셋
int16_t  mhsensor_calibration_Data.Offset_By;  자기장 y축 옵셋
int16_t  mhsensor_calibration_Data.Offset_Bz;  자기장 z축 옵셋
*/
static void initValue(void)
{
    // 옥천보정
    mhsensor_calibration_Data.Gain_Bx = -0.965251;
    mhsensor_calibration_Data.Gain_By = -0.949307;
    mhsensor_calibration_Data.Gain_Bz = -0.954563;
    mhsensor_calibration_Data.Offset_Bx   = -120;
    mhsensor_calibration_Data.Offset_By   = 80;
    mhsensor_calibration_Data.Offset_Bz   = -147;

    mhsensor_calibration_Data.Gain_Ax = 0.000169;
    mhsensor_calibration_Data.Gain_Ay = -0.00019;
    mhsensor_calibration_Data.Gain_Az = 0.0001;
    mhsensor_calibration_Data.Offset_Ax   = 160;
    mhsensor_calibration_Data.Offset_Ay   = -700;
    mhsensor_calibration_Data.Offset_Az   = 0;

}

/*
기능설명
분해능(bittype)에 따라 ADC의 작동 모드와 클럭 설정, 인터럽트 모드, 변환기 활성화 등을 설정하는 함수

입력 변수
uint32_t adcBase,: ADC 모듈 주소
uint16_t bittype : ADC 분해능 설정값
*/

void configureADC(uint32_t adcBase, uint16_t bittype)
{
    uint16_t resoulution = bittype;

	// 1. ADC 클럭을 시스템 클럭의 4분의 1로 설정
    ADC_setPrescaler(adcBase, ADC_CLK_DIV_4_0);

	// 2. 분해능이 ADC 모드 설정
    if(resoulution == 12U)
    {
    	//2.1 12bit 분해능, 싱글 앤드 모드로 설정 
		ADC_setMode(adcBase, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    }
    else
    {
    	//2.2 16bit 분해능, 차동 모드로 설정 
        ADC_setMode(adcBase, ADC_RESOLUTION_16BIT, ADC_MODE_DIFFERENTIAL);
    }

	// 3. ADC 변환 종료 시점에 인터럽트 발생으로 설정 
    ADC_setInterruptPulseMode(adcBase, ADC_PULSE_END_OF_CONV);

	//4. ADC 모듈을 활성화하여 변환이 시작될 준
    ADC_enableConverter(adcBase);

	// ADC 안정화를 위한 지연 시간 
    delay_uS(1000.0L);
}


/*
기능설명
 ADC 모듈의 SOC 설정, 인터럽트 처리를 구성하는 초기화 함수
*/
void initADCSOC(void)
{
    // 1. initValue()를 이용하여 자기장, 가속도 보정 변수 초기화
    initValue();

#ifdef NEW_BOARD
    // 2. 16bit ADCA 모듈의 SOC(Sample and Hold Circuit) 채널별  설정
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0_ADCIN1, 63);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2_ADCIN3, 63);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN14_ADCIN15, 63);

    // 3. 16bit ADCB 모듈의 SOC(Sample and Hold Circuit) 채널별  설정
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0_ADCIN1, 63);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2_ADCIN3, 63);

    // 4. 12bit ADCD 모듈의 SOC(Sample and Hold Circuit) 채널별  설정
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0, 200);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN1, 200);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2, 200);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN3, 200);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER4, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN13, 200);

#else
    // 2. 16bit ADCA 모듈의 SOC(Sample and Hold Circuit) 채널별  설정
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0_ADCIN1, 63);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2_ADCIN3, 63);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN4_ADCIN5, 63);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN12_ADCIN13, 63);

    // 3. 16bit ADCB 모듈의 SOC(Sample and Hold Circuit) 채널별  설정
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0_ADCIN1, 63);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2_ADCIN3, 63);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN4_ADCIN5, 63);

    // 4. 12bit ADCD 모듈의 SOC(Sample and Hold Circuit) 채널별  설정
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0, 14);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN1, 14);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2, 14);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN3, 14);
#endif

    // 5. ADC_INT_NUMBER1 인터럽트가 ADC_SOC_NUMBER3 변환 완료 시 발생
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER3);
	
	// 6. ADC_INT_NUMBER1 인터럽트를 활성화하여 변환이 완료될 때 인터럽트가 발생
	ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);

	// 7. 설정 초기 상태에서 인터럽트 클리어 시킴.
	ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);


}

/*
기능설명
ADCA 모듈에서 ADC 변환 완료 시 발생하는 인터럽트 서비스 루틴 함수

입출력 전역변수

int16_t ema[12];   지수이동평균(EMA)필터된 값을 저장하는 버퍼
*/
__interrupt void adcA1ISR(void)
{

    uint16_t read0,read1,read2;

    int16_t fluxread[3];
    uint16_t adcax0, adcax1;
    int16_t accread[3];

    uint16_t gadcvalue6=0;
    uint16_t gadcvalue7=0;
    uint16_t gadcvalue8=0;
    uint16_t gadcvalue9=0;
    uint16_t gadcvalue10=0;

    int16_t gBxADC;
    int16_t gByADC;
    int16_t gBzADC;
    int16_t gAxADC;
    int16_t gAyADC;
    int16_t gAzADC;

    uint16_t result_0, result_1, result_2;
    uint16_t result_a0, result_a1;

	// 1. x,y,z축 자기장 센서 데이터 읽기 및 변환
#ifdef NEW_BOARD
    read0 =  ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER0);  // x
    read1 =  ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER1);  // y
    read2 =  ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER2);  // z
#else
    read0 =  ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);  // x
    read1 =  ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);  // y
    read2 =  ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);  // z

#endif

    // 2. ADCA 모듈의 SOC 번호 0의 결과를 읽어 자기장 x 축 데이터(fluxread)로 변환
    //    단, 읽어온 값이 0x7FFFU보다 크면 그대로 사용하고, 작다면 보정하여 부호화
    if(read0 > 0x7FFFU)
    {
        result_0 = read0 - 0x7FFFU;
        fluxread[0] = (int16_t)result_0;
    }
    else
    {
        result_0 = 0x7FFFU - read0;
        fluxread[0] = (int16_t)result_0 * -1;
    }

    // 3. ADCA 모듈의 SOC 번호 1의 결과를 읽어 자기장 y 축 데이터(fluxread)로 변환
    //    단, 읽어온 값이 0x7FFFU보다 크면 그대로 사용하고, 작다면 보정하여 부호화
    if(read1 > 0x7FFFU)
    {
        result_1 = read1 - 0x7FFFU;
        fluxread[1] = (int16_t)result_1;
    }
    else
    {
        result_1 = 0x7FFFU - read1;
        fluxread[1] = (int16_t)result_1 * -1;
    }

    // 4. ADCA 모듈의 SOC 번호 2의 결과를 읽어 자기장 z 축 데이터(fluxread)로 변환
    //    단, 읽어온 값이 0x7FFFU보다 크면 그대로 사용하고, 작다면 보정하여 부호화
    if(read2 > 0x7FFFU)
    {
        result_2 = read2 - 0x7FFFU;
        fluxread[2] = (int16_t)result_2;
    }
    else
    {
        result_2 = 0x7FFFU - read2;
        fluxread[2] = (int16_t)result_2 * -1;
    }


	// 5. x,y,z축 가속도 센서 데이터 읽기 및 변환
    adcax0 = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0);
    adcax1 = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);

    // 6. ADCB모듈의 SOC 번호 0의 결과를 읽어 가속도 x 축 데이터(accread)로 변환
    //    단, 읽어온 값이 0x7FFFU보다 크면 그대로 사용하고, 작다면 보정하여 부호화
    if(adcax0 > 0x7FFFU)
    {
        result_a0 = adcax0 - 0x7FFFU;
        accread[0] = (int16_t)result_a0;
    }
    else
    {
        result_a0 = 0x7FFFU - adcax0;
        accread[0] = (int16_t)result_a0 * -1;
    }

    // 7. ADCB모듈의 SOC 번호 1의 결과를 읽어 가속도 y 축 데이터(accread)로 변환
    //    단, 읽어온 값이 0x7FFFU보다 크면 그대로 사용하고, 작다면 보정하여 부호화
    if(adcax1 > 0x7FFFU)
    {
        result_a1 = adcax1 - 0x7FFFU;
        accread[1] = (int16_t)result_a1;
    }
    else
    {
        result_a1 = 0x7FFFU - adcax1;
        accread[1] = (int16_t)result_a1 * -1;
    }

    // 8. ADCB모듈의 SOC 번호 2의 결과를 읽어 가속도 z 축 데이터(accread)로 변환
    //    단, 읽어온 값이 0x7FFFU보다 크면 그대로 사용하고, 작다면 보정하여 부호화

    accread[2] = 0;


	// 9. ADCD와 ADCA 모듈의 결과 값을 읽어, 전압, 온도 등과 같은 추가적인 값들을 gadcvalue6에서 gadcvalue10에 저장
#ifdef NEW_BOARD
    gadcvalue6 = (uint16_t)ADC_readPPBResult(ADCARESULT_BASE, ADC_PPB_NUMBER1);
    gadcvalue7 = (uint16_t)ADC_readPPBResult(ADCARESULT_BASE, ADC_PPB_NUMBER2);
    gadcvalue8 = (uint16_t)ADC_readPPBResult(ADCARESULT_BASE, ADC_PPB_NUMBER3);
    gadcvalue9 = (uint16_t)ADC_readPPBResult(ADCARESULT_BASE, ADC_PPB_NUMBER4);
    gadcvalue10 = (uint16_t)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4);


#else

    gadcvalue6 = (uint32_t)ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER0);
    gadcvalue7 = (uint32_t)ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER1);
    gadcvalue8 = (uint32_t)ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER2);
    gadcvalue9 = (uint32_t)ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER3);

#endif

	// 10. ema 필터를 위해 자기장 값들을 각 변수에 저장 
#ifdef NEW_BOARD
    gBxADC = (int16_t)fluxread[0];  // x
    gByADC = (int16_t)fluxread[1];  // y
    gBzADC = (int16_t)fluxread[2];  // z
#else
    gBxADC = (int16_t)fluxread[2];  // x
    gByADC = (int16_t)fluxread[1];  // y
    gBzADC = (int16_t)fluxread[0];
#endif

    gAxADC = (int16_t)accread[0];
    gAyADC = (int16_t)accread[1];
    gAzADC = (int16_t)accread[2];

	// 11. 각 센서 데이터 및 전압 값을 ema_filter 함수를 이용해 지수 이동 평균 필터를 적용하여 ema 배열에 저장.
    ema[ADC_CH_INDEX_FLUX_X] = ema_filter((float64_t)gBxADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_FLUX_X]);
    ema[ADC_CH_INDEX_FLUX_Y] = ema_filter((float64_t)gByADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_FLUX_Y]);
    ema[ADC_CH_INDEX_FLUX_Z] = ema_filter((float64_t)gBzADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_FLUX_Z]);
    ema[ADC_CH_INDEX_ACCEL_X] = ema_filter((float64_t)gAxADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_ACCEL_X]);
    ema[ADC_CH_INDEX_ACCEL_Y] = ema_filter((float64_t)gAyADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_ACCEL_Y]);
    ema[ADC_CH_INDEX_ACCEL_Z] = ema_filter((float64_t)gAzADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_ACCEL_Z]);

    ema[ADC_CH_INDEX_VOLYAGE_28V] = ema_filter((float64_t)gadcvalue6, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_28V]);
    ema[ADC_CH_INDEX_VOLYAGE_5V] = ema_filter((float64_t)gadcvalue7, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_5V]);
    ema[ADC_CH_INDEX_VOLYAGE_3p3V] = ema_filter((float64_t)gadcvalue8, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_3p3V]);
    ema[ADC_CH_INDEX_VOLYAGE_1p2V] = ema_filter((float64_t)gadcvalue9, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_1p2V]);
    ema[ADC_CH_INDEX_VOLYAGE_TEMPERATUR] = ema_filter((float64_t)gadcvalue10, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_TEMPERATUR]);

	// 12. 인터럽트 플래그를 클리어하고, 오버플로우 발생 여부를 확인
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    if(TRUE == ADC_getInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1))
    {
		//12.1  오버플로우가 발생했을 경우 인터럽트 오버플로우 플래그와 상태를 초기화
        ADC_clearInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    }

	// 13. 그룹 1 ACK 신호를 클리어하여 다음 인터럽트를 수락할 수 있도록 준비
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


/*
기능설명
 지수 이동 평균(EMA, Exponential Moving Average) 필터를 적용하여 ADC 값을 필터링 하는 함수

입출력 변수
float64_t adc : 현재 측정된 ADC 값
float64_t alpha : 필터 가중 지수 값
int16_t previous_ema_filtered_value : 이전 필터된 값
*/
static int16_t ema_filter(float64_t adc, float64_t alpha, int16_t previous_ema_filtered_value)
{
	float64_t new_ema_filtered_value;

	// 1. 지수이동평균(EMA) 
	// 수식 : EMAnew=(1?α)×ADC+α×EMAprev
	new_ema_filtered_value  = ((1.0 - alpha) * adc) + (alpha * (float64_t)previous_ema_filtered_value);

	// 2. 필터된 값 리턴
	return 	(int16_t)new_ema_filtered_value;
}

/*
 기능설명
 ADC 채널 후처리 옵셋 보정값 설정

    계산        28        5     3.3             1.2
    측정값(ADC)  1640      0     1470            1600
               0.4004    0     0.35889         0.3906

    이론값       1.3308    0     1.0985          1.2
               0.4436          0.366166667     0.4
    이론adc     1816.9856 0     1499.818667     1638.4
    이론과측정차이 176.9856  0     29.81866667     38.4

    측정 방법

    1. 우선 저항 분해 이론값을 전압별 계산한다.
    2. 실제 각 전압별 ADC값을 읽는다.
    3. 이론값과 실제값의 ADC 차이값을 계산한다.
    4. 차이값을 ADC_PPB 보정값에 입력한다.
    5. 만약 차이값이 양수이면 보정에는 음수로 입력한다.
    음수면 반대로 양수로 입력한다.
*/

void setupPPBOffset(void)
{
    // 1. ADCA CH0의 후처리 블록(PPB)을 구성
    ADC_setupPPB(ADCA_BASE, ADC_PPB_NUMBER1, ADC_SOC_NUMBER0);
    // 2. ADCA CH0 후처리 블럭 옵셋 보정값 설정
    ADC_setPPBCalibrationOffset(ADCA_BASE, ADC_PPB_NUMBER1, -177);

    // 1. ADCA CH1 후처리 블록(PPB)을 구성
    ADC_setupPPB(ADCA_BASE, ADC_PPB_NUMBER2, ADC_SOC_NUMBER1);
    // 2. ADCA CH1 후처리 블럭 옵셋 보정값 설정
    ADC_setPPBCalibrationOffset(ADCA_BASE, ADC_PPB_NUMBER2, -90);

    // 1. ADCA CH2의 후처리 블록(PPB)을 구성
    ADC_setupPPB(ADCA_BASE, ADC_PPB_NUMBER3, ADC_SOC_NUMBER2);
    // 2. ADCA CH2 후처리 블럭 옵셋 보정값 설정
    ADC_setPPBCalibrationOffset(ADCA_BASE, ADC_PPB_NUMBER3, -30);

    // 1. ADCA CH3의 후처리 블록(PPB)을 구성
    ADC_setupPPB(ADCA_BASE, ADC_PPB_NUMBER4, ADC_SOC_NUMBER3);
    // 2. ADCA CH3 후처리 블럭 옵셋 보정값 설정
    ADC_setPPBCalibrationOffset(ADCA_BASE, ADC_PPB_NUMBER4, -38);

}

