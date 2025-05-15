/*
* 기능 : DSP 초기화 및 타이머 인터럽트 함수
 * 이력 :
 *    2024.05.23 : 이충순 : 초기 작성
*/

//#include <stdlib.h>
//#include <string.h>
#include <math.h>
#include <file.h>
#include "mhs_project.h"

#define CLR_WDT_TIME            (1200U)      // 1분(60) / 0.05s = 3600

static void delay_10uS(void);

static uint16_t cputimer0Flag;		// 50 mSec 주기적으로 타이머 인터럽트 플래그
static uint32_t wdtclrcnt;			// 하드웨어 와치독을 주기적으로 리셋 시키기위하 카운터 

// DSP 초기화 및 50mS마다 모드를 호출하는 함수
void main(void)
{
	// 1. DSP 관련 초기화
    dspinit();

	// 2. 정장된 CAL parameter loading
    read_parameter();

	// 3. HI 3587 TX 모드로 초기화 및 상태 확인
    writeControlReg();
    uint16_t rxword = readControlReg();
    if(rxword == 0U)
    {

    }

	// 4. 모드를 status_mode 로 초기화 시킴.
    Can_State_Ptr = &status_mode;///normal mode

    while(1)
    {

#if 1
		static uint16_t jump = 7;

        if(jump == 4U)      Can_State_Ptr = &status_mode;
        else if(jump == 5U)      Can_State_Ptr = &calibrationMode;
        else if(jump == 6U)      Can_State_Ptr = &factory_mode;
        else if(jump == 7U)      Can_State_Ptr = &operation_mode;
#endif

		//5. 50mS 주기로 모드 상태 전환
        if(cputimer0Flag == 1U)//50ms Timer flag
        {
        	//5.1 주기적으로 와치독 리셋 시킴
            if(wdtclrcnt > CLR_WDT_TIME)
            {
                GPIO_writePin(CLR_WDT, 0);
                delay_10uS();
                GPIO_writePin(CLR_WDT, 1);
                wdtclrcnt = 0U;
            }

			// 5.2 구동할 모드 함수를 호출
            (*Can_State_Ptr)();

			// 5.3 다름 주기를 위해 cputimer0Flag를 초기화 시킴
            cputimer0Flag = 0U;
        }
    }
}

// 타이머0 인터럽트 서비스 루틴 함수
__interrupt void cpuTimer0ISR(void)
{

	// 1. 주기적으로 cputimer0Flag true로 설정한다.
    wdtclrcnt++;
    cputimer0Flag = 1U;

	// 2. 그룹1 인터럽트 클리어
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


// 1 us 단위 지연 함수
void delay_uS(float64_t x)
{

    float64_t delay = ((((float64_t)(x)) / (1000000.0L / (float64_t)DEVICE_SYSCLK_FREQ)) - 9.0L) / 5.0L;
    SysCtl_delay((uint32_t)delay);

}
// 10 us 단위 지연 함수
static void delay_10uS(void)
{
    float64_t x = 10.0L;
    float64_t delay = ((((float64_t)(x)) / (1000000.0L / (float64_t)DEVICE_SYSCLK_FREQ)) - 9.0L) / 5.0L;
    SysCtl_delay((uint32_t)delay);

}
