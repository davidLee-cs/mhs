/*
 * lib_mhsEpwm.c
 *
 * 기능 : Epwm 인터럽트를 이용하여 ADC 인터럽트 설정 기능
 * 구성요소 :  (D- )
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */

#include <lib_mhsEpwm.h>
#include "init.h"


#if 0
/*기능설명
 EPWM 이벤트에 대한 인터럽트 서비스 루틴 함수 */
__interrupt void INT_myEPWM0_ISR(void)
{

	// 1. EPWM1 모듈의 이벤트 트리거 인터럽트 플래그를 클리어
    EPWM_clearEventTriggerInterruptFlag(EPWM1_BASE);
	// 2. 그룹3 대한 ACK 플래그를 클리어
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}
#endif
// 기능 설명 : EPWM1 모듈 초기화하는 함수
void initEPWM(void)
{
	//1. ADC 트리거 비활성화
    EPWM_disableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

	// 2. ADC 트리거 소스 설정
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
	//3. ADC 트리거 이벤트 프리스케일 설정
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1);
	// 4. EPWM1 카운터 비교 값 설정
    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 1000);
	// 5. EPWM1 타임베이스 주기 설정
    EPWM_setTimeBasePeriod(EPWM1_BASE, 1999);

	// 6. 클록 프리스케일러 설정
    EPWM_setClockPrescaler(EPWM1_BASE,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);

	// 7. 타임베이스 카운터 모드 설정
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);
}

