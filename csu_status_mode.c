/*
 * status_mode.c
 *
 * ��� : ��� ü�迡 ���� Operation mode, Calibration mode, Factory mode�� �����Ͽ� �ش��� ����
 * ������� : ���¿� ��� (D-MHS-SFR-001)
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */

#include "mhs_project.h"



#define DELAY_100MS      (100000.0L)
#define DELAY_10MS       (10000.0L)
#define LOOP_COUNT_LIMIT (100U)		// Chattering ���� ��Ȳ�� 100 msec  �� �ʰ����� �ʵ��� �����ϴ� ����

void (*Can_State_Ptr)(void);        // ���� ����� ��带 ����Ű�� �Լ� ������
									// calibrationMode() / status_mode() / factory_mode() / IdleMode() / operation_mode() �� ���� �� ����� ���� �Լ���������Ŵ.
									// ���� cycle  ����� Can_State_Ptr �� ����Ű�� ��� �Լ��� �����.

uint16_t calibration_mode;


// ��� : Discrete SW 1,2 �� ���� ��� �����ϴ� �Լ�
//       SW1,2 (H/H) : Operation
//		 SW1,2 (L/x) : calibrationMode
//		 SW1,2 (H/L) : factory_mode
// �̷� : 
//	 2024.05.23 : ����� : �ʱ� �ۼ�
void status_mode(void)
{
	uint32_t Discrete_1_val=1, Discrete_2_val=1, Discrete_1_val_new=1, Discrete_2_val_new=1;
	uint32_t loop_count = 0U;

	// 1. ��ȣ�� ����ȭ�� ���Ͽ� 100 msec  ���
    delay_uS(DELAY_100MS);

    do{

        Discrete_1_val = Discrete_1_val_new;
        Discrete_2_val = Discrete_2_val_new;

        delay_uS(DELAY_10MS);

        Discrete_1_val_new = GPIO_readPin(DISCRETE_1);
        Discrete_2_val_new = GPIO_readPin(DISCRETE_2);
        loop_count++;

#ifdef NEW_BOARD
        if(((Discrete_1_val == Discrete_1_val_new) && (Discrete_2_val == Discrete_2_val_new)) || (loop_count > LOOP_COUNT_LIMIT))
#else
        if((Discrete_1_val == Discrete_1_val_new)|| (loop_count > LOOP_COUNT_LIMIT))
#endif
        {
            break;
        }

    } while(1);


	// 3 ��ȣ�Է¿� ���� ��� ����
	// 3.1 chattering �� 100 msec ���� ����ȭ ���� ������ status_mode �� �۵��ϵ��� Can_State_Ptr �� status_mode �� ����
	// 3.2 (SW1 �� LOW �̸� calibration ���� �۵��ϵ��� Can_State_Ptr �� calibration �� ����
	// 3.3 (SW1 �� HIGH �̰� SW1 �� LOW �̸� factory_mode ���� �۵��ϵ��� Can_State_Ptr �� factory_mode �� ����
	// 3.4 (SW1 �� HIGH �̰� SW1 �� HIGH �̸� operation_mode ���� �۵��ϵ��� Can_State_Ptr �� operation_mode �� ����
	if (loop_count >= LOOP_COUNT_LIMIT)
	{
		Can_State_Ptr = &status_mode;
	}
    else if((Discrete_1_val == 1U) && (Discrete_2_val == 0U))
    {
        calibration_mode = 1U;
        Can_State_Ptr = &calibrationMode;
    } 
    else if ((Discrete_1_val == 0U) && (Discrete_2_val == 1U))
    {
        calibration_mode = 0U;
        gfirstOpen_factory = 1U;
        Interrupt_enable(INT_mySCI0_TX);
        Can_State_Ptr = &factory_mode;
    }
    else 
    {
        calibration_mode = 0U;
		Can_State_Ptr = &operation_mode;
    };

}


