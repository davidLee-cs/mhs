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

	// 6. 글로벌 인터럽트를 활성화
    EINT;
    ERTM;

	// 7. EPROM과 특정 모듈 초기화
    eepromInit();
	// 8. HI 3587 리셋
    mrInit();

	// 9. ADC 변환을 트리거하기 위해 EPWM의 SOCA  활성화
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

	// 10. EPWM의 카운터 모드를 UP 모드로 설정
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);

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
#ifdef NEW_BOARD
    configureADC(ADCA_BASE, ADC_RESOLUTION_12_BIT);		// volt.	
    configureADC(ADCB_BASE, ADC_RESOLUTION_16_BIT);		// accel
    configureADC(ADCD_BASE, ADC_RESOLUTION_16_BIT);		// flux
#else
    configureADC(ADCA_BASE, ADC_RESOLUTION_16_BIT);
    configureADC(ADCB_BASE, ADC_RESOLUTION_16_BIT);
    configureADC(ADCD_BASE, ADC_RESOLUTION_12_BIT);

#endif

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

#if 0
void configCPUTimer(uint32_t cpuTimer, float32_t freq, float32_t period)
{
    uint32_t temp;

    //
    // Initialize timer period:
    //
    temp = (uint32_t)(freq / 1000000 * period);
    CPUTimer_setPeriod(cpuTimer, temp);

    //
    // Set pre-scale counter to divide by 1 (SYSCLKOUT):fff
    //
    CPUTimer_setPreScaler(cpuTimer, 0U);

    //
    // Initializes timer control register. The timer is stopped, reloaded,
    // free run disabled, and interrupt enabled.
    // Additionally, the free and soft bits are set
    //
    CPUTimer_stopTimer(cpuTimer);
    CPUTimer_reloadTimerCounter(cpuTimer);
    CPUTimer_setEmulationMode(cpuTimer,
                              CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
    CPUTimer_enableInterrupt(cpuTimer);

}
#endif
