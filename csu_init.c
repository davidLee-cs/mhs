/*
 * csu_init.c
 *
 * 기능 :  MCU 초기화 하는 기능 
 * 구성요소 : 초기화 CSU (D-MHS-SFR-006)
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */


#include "mhs_project.h"

#define ADC_RESOLUTION_16_BIT       16
#define ADC_RESOLUTION_12_BIT       12

static void device_set(void);
static void mrInit(void);
static void init_interrupt(void);

/*
  기능 설명
    DSP 초기화 설정 함수
*/
void dspinit(void)
{
	// 1. DSP의 기본 장치 설정을 초기화
    device_set();

	// 2. GPIO 핀, 통신 인터페이스, 전원 장치 등을 설정
    Board_init();

    Interrupt_disable(INT_mySCI0_TX);
	// 4. 인터럽트 핸들러와 벡터 테이블을 초기화
    init_interrupt();

	// 5. 온도 센서 활성화
    ASysCtl_enableTemperatureSensor();
    delay_uS(500.0L);

    EINT;
    ERTM;

	// 6. EPROM과 특정 모듈 초기화
    eepromInit();
	// 7. HI 3587 리셋
    mrInit();

	// 8. ADC 변환을 트리거하기 위해 EPWM의 SOCA  활성화
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

	// 9. EPWM의 카운터 모드를 UP 모드로 설정
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);

    // 10. ADC 채널 후처리 옵셋 보정값 설정
    setupPPBOffset();

}

// 기능 설명 : DSP의 기본 시스템 및 주변 장치 초기화 함수
static void device_set(void)
{
	// 1. DSP의 시스템 초기화 
    Device_init();

	// 2. GPIO 초기화
    Device_initGPIO();

	// 3. I2C를 위한 GPIO 설정  
    I2C_GPIO_init();

	// 4. 인터럽트 모듈 초기화
    Interrupt_initModule();

	// 5. 인터럽트 벡터 테이블 초기화
    Interrupt_initVectorTable();

}

// 기능설명 :  와치독 타이머 및 HI 3587 리셋 시키는 함수
static void mrInit(void)
{

	// 1.워치독 타이머를 클리어
    GPIO_writePin(CLR_WDT, 1);
	// HI 3587 리셋 
	GPIO_writePin(MR, 1);
    delay_uS(100.0L);
    GPIO_writePin(MR, 0);

}

// 기능 설명 : 시스템의 다양한 주변 장치에 대한 인터럽트 설정하는 함수
static void init_interrupt(void)
{

	// 1. I2C, ADC, ePWM 초기화
    I2Cinit();
    initADCSOC();
    initEPWM();

	// 2. 인터럽트 핸들러 등록
    Interrupt_register(INT_ADCA1, &adcA1ISR);
    Interrupt_register(INT_I2CA_FIFO, &i2cFIFO_isr);
    Interrupt_register(INT_I2CA, &i2c_isr);
    Interrupt_register(INT_SCIC_RX, &INT_mySCI0_RX_ISR);

	// 3. ADC 해상도 설정
    configureADC(ADCA_BASE, ADC_RESOLUTION_12_BIT);		// volt.	
    configureADC(ADCB_BASE, ADC_RESOLUTION_16_BIT);		// accel
    configureADC(ADCD_BASE, ADC_RESOLUTION_16_BIT);		// flux

    CPUTimer_enableInterrupt(myCPUTIMER0_BASE);

	// 4. 인터럽트 활성화 및 비활성화
    Interrupt_enable(INT_I2CA_FIFO);
    Interrupt_enable(INT_I2CA);
    Interrupt_enable(INT_SCIC_RX);
    Interrupt_enable(INT_TIMER0);
    Interrupt_disable(mySCI0_BASE);
    Interrupt_enable(INT_ADCA1);

	// 5. 인터럽트 ACK 그룹 초기화
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);

}
