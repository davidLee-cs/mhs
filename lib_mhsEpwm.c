/*
 * lib_mhsEpwm.c
 *
 * ��� : Epwm ���ͷ�Ʈ�� �̿��Ͽ� ADC ���ͷ�Ʈ ���� ���
 * ������� :  (D- )
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */

#include <lib_mhsEpwm.h>
#include "init.h"


#if 0
/*��ɼ���
 EPWM �̺�Ʈ�� ���� ���ͷ�Ʈ ���� ��ƾ �Լ� */
__interrupt void INT_myEPWM0_ISR(void)
{

	// 1. EPWM1 ����� �̺�Ʈ Ʈ���� ���ͷ�Ʈ �÷��׸� Ŭ����
    EPWM_clearEventTriggerInterruptFlag(EPWM1_BASE);
	// 2. �׷�3 ���� ACK �÷��׸� Ŭ����
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}
#endif
// ��� ���� : EPWM1 ��� �ʱ�ȭ�ϴ� �Լ�
void initEPWM(void)
{
	//1. ADC Ʈ���� ��Ȱ��ȭ
    EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

	// 2. ADC Ʈ���� �ҽ� ����
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
	//3. ADC Ʈ���� �̺�Ʈ ���������� ����
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1);
	// 4. EPWM1 ī���� �� �� ����
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 1000);
	// 5. EPWM1 Ÿ�Ӻ��̽� �ֱ� ����
    EPWM_setTimeBasePeriod(EPWM1_BASE, 1999);

	// 6. Ŭ�� ���������Ϸ� ����
    EPWM_setClockPrescaler(EPWM1_BASE,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);

	// 7. Ÿ�Ӻ��̽� ī���� ��� ����
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);
}

