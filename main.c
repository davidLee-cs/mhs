/*
* ��� : DSP �ʱ�ȭ �� Ÿ�̸� ���ͷ�Ʈ �Լ�
 * �̷� :
 *    2024.05.23 : ����� : �ʱ� �ۼ�
*/

//#include <stdlib.h>
//#include <string.h>
#include <math.h>
#include <file.h>
#include "mhs_project.h"

#define CLR_WDT_TIME            (1200U)      // 1��(60) / 0.05s = 3600

static void delay_10uS(void);

static uint16_t cputimer0Flag;		// 50 mSec �ֱ������� Ÿ�̸� ���ͷ�Ʈ �÷���
static uint32_t wdtclrcnt;			// �ϵ���� ��ġ���� �ֱ������� ���� ��Ű������ ī���� 

// DSP �ʱ�ȭ �� 50mS���� ��带 ȣ���ϴ� �Լ�
void main(void)
{
	// 1. DSP ���� �ʱ�ȭ
    dspinit();

	// 2. ����� CAL parameter loading
    read_parameter();

	// 3. HI 3587 TX ���� �ʱ�ȭ �� ���� Ȯ��
    writeControlReg();
    uint16_t rxword = readControlReg();
    if(rxword == 0U)
    {

    }

	// 4. ��带 status_mode �� �ʱ�ȭ ��Ŵ.
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

		//5. 50mS �ֱ�� ��� ���� ��ȯ
        if(cputimer0Flag == 1U)//50ms Timer flag
        {
        	//5.1 �ֱ������� ��ġ�� ���� ��Ŵ
            if(wdtclrcnt > CLR_WDT_TIME)
            {
                GPIO_writePin(CLR_WDT, 0);
                delay_10uS();
                GPIO_writePin(CLR_WDT, 1);
                wdtclrcnt = 0U;
            }

			// 5.2 ������ ��� �Լ��� ȣ��
            (*Can_State_Ptr)();

			// 5.3 �ٸ� �ֱ⸦ ���� cputimer0Flag�� �ʱ�ȭ ��Ŵ
            cputimer0Flag = 0U;
        }
    }
}

// Ÿ�̸�0 ���ͷ�Ʈ ���� ��ƾ �Լ�
__interrupt void cpuTimer0ISR(void)
{

	// 1. �ֱ������� cputimer0Flag true�� �����Ѵ�.
    wdtclrcnt++;
    cputimer0Flag = 1U;

	// 2. �׷�1 ���ͷ�Ʈ Ŭ����
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


// 1 us ���� ���� �Լ�
void delay_uS(float64_t x)
{

    float64_t delay = ((((float64_t)(x)) / (1000000.0L / (float64_t)DEVICE_SYSCLK_FREQ)) - 9.0L) / 5.0L;
    SysCtl_delay((uint32_t)delay);

}
// 10 us ���� ���� �Լ�
static void delay_10uS(void)
{
    float64_t x = 10.0L;
    float64_t delay = ((((float64_t)(x)) / (1000000.0L / (float64_t)DEVICE_SYSCLK_FREQ)) - 9.0L) / 5.0L;
    SysCtl_delay((uint32_t)delay);

}
