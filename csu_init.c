/*
 * csu_init.c
 *
 * ��� :  MCU �ʱ�ȭ �ϴ� ��� 
 * ������� : �ʱ�ȭ CSU (D-MHS-SFR-006)
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */


#include "mhs_project.h"

#define ADC_RESOLUTION_16_BIT       16
#define ADC_RESOLUTION_12_BIT       12

static void device_set(void);
static void mrInit(void);
static void init_interrupt(void);

/*
  ��� ����
    DSP �ʱ�ȭ ���� �Լ�
*/
void dspinit(void)
{
	// 1. DSP�� �⺻ ��ġ ������ �ʱ�ȭ
    device_set();

	// 2. GPIO ��, ��� �������̽�, ���� ��ġ ���� ����
    Board_init();

    Interrupt_disable(INT_mySCI0_TX);
	// 4. ���ͷ�Ʈ �ڵ鷯�� ���� ���̺��� �ʱ�ȭ
    init_interrupt();

	// 5. �µ� ���� Ȱ��ȭ
    ASysCtl_enableTemperatureSensor();
    delay_uS(500.0L);

    EINT;
    ERTM;

	// 6. EPROM�� Ư�� ��� �ʱ�ȭ
    eepromInit();
	// 7. HI 3587 ����
    mrInit();

	// 8. ADC ��ȯ�� Ʈ�����ϱ� ���� EPWM�� SOCA  Ȱ��ȭ
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

	// 9. EPWM�� ī���� ��带 UP ���� ����
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);

    // 10. ADC ä�� ��ó�� �ɼ� ������ ����
    setupPPBOffset();

}

// ��� ���� : DSP�� �⺻ �ý��� �� �ֺ� ��ġ �ʱ�ȭ �Լ�
static void device_set(void)
{
	// 1. DSP�� �ý��� �ʱ�ȭ 
    Device_init();

	// 2. GPIO �ʱ�ȭ
    Device_initGPIO();

	// 3. I2C�� ���� GPIO ����  
    I2C_GPIO_init();

	// 4. ���ͷ�Ʈ ��� �ʱ�ȭ
    Interrupt_initModule();

	// 5. ���ͷ�Ʈ ���� ���̺� �ʱ�ȭ
    Interrupt_initVectorTable();

}

// ��ɼ��� :  ��ġ�� Ÿ�̸� �� HI 3587 ���� ��Ű�� �Լ�
static void mrInit(void)
{

	// 1.��ġ�� Ÿ�̸Ӹ� Ŭ����
    GPIO_writePin(CLR_WDT, 1);
	// HI 3587 ���� 
	GPIO_writePin(MR, 1);
    delay_uS(100.0L);
    GPIO_writePin(MR, 0);

}

// ��� ���� : �ý����� �پ��� �ֺ� ��ġ�� ���� ���ͷ�Ʈ �����ϴ� �Լ�
static void init_interrupt(void)
{

	// 1. I2C, ADC, ePWM �ʱ�ȭ
    I2Cinit();
    initADCSOC();
    initEPWM();

	// 2. ���ͷ�Ʈ �ڵ鷯 ���
    Interrupt_register(INT_ADCA1, &adcA1ISR);
    Interrupt_register(INT_I2CA_FIFO, &i2cFIFO_isr);
    Interrupt_register(INT_I2CA, &i2c_isr);
    Interrupt_register(INT_SCIC_RX, &INT_mySCI0_RX_ISR);

	// 3. ADC �ػ� ����
    configureADC(ADCA_BASE, ADC_RESOLUTION_12_BIT);		// volt.	
    configureADC(ADCB_BASE, ADC_RESOLUTION_16_BIT);		// accel
    configureADC(ADCD_BASE, ADC_RESOLUTION_16_BIT);		// flux

    CPUTimer_enableInterrupt(myCPUTIMER0_BASE);

	// 4. ���ͷ�Ʈ Ȱ��ȭ �� ��Ȱ��ȭ
    Interrupt_enable(INT_I2CA_FIFO);
    Interrupt_enable(INT_I2CA);
    Interrupt_enable(INT_SCIC_RX);
    Interrupt_enable(INT_TIMER0);
    Interrupt_disable(mySCI0_BASE);
    Interrupt_enable(INT_ADCA1);

	// 5. ���ͷ�Ʈ ACK �׷� �ʱ�ȭ
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);

}
