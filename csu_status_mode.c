/*
 * status_mode.c
 *
 * 기능 : 운용 체계에 따른 Operation mode, Calibration mode, Factory mode로 구분하여 해당모드 구동
 * 구성요소 : 상태와 모드 (D-MHS-SFR-001)
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */

#include "mhs_project.h"



#define DELAY_100MS      (100000.0L)
#define DELAY_10MS       (10000.0L)
#define LOOP_COUNT_LIMIT (100U)		// Chattering 감지 상황이 100 msec  을 초과하지 않도록 제한하는 선언값

void (*Can_State_Ptr)(void);        // 다음 수행될 모드를 가르키는 함수 포인터
									// calibrationMode() / status_mode() / factory_mode() / IdleMode() / operation_mode() 와 같이 각 모드의 실행 함수명을가르킴.
									// 다음 cycle  수행시 Can_State_Ptr 가 가르키는 모드 함수가 실행됨.

uint16_t calibration_mode;


// 기능 : Discrete SW 1,2 를 통해 모드 선택하는 함수
//       SW1,2 (H/H) : Operation
//		 SW1,2 (L/x) : calibrationMode
//		 SW1,2 (H/L) : factory_mode
// 이력 : 
//	 2024.05.23 : 이충순 : 초기 작성
void status_mode(void)
{
	uint32_t Discrete_1_val=1, Discrete_2_val=1, Discrete_1_val_new=1, Discrete_2_val_new=1;
	uint32_t loop_count = 0U;

	// 1. 신호의 안정화를 이하여 100 msec  대기
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


	// 3 신호입력에 따라 모드 설정
	// 3.1 chattering 이 100 msec 까지 안정화 되지 않으면 status_mode 로 작동하도록 Can_State_Ptr 를 status_mode 로 변경
	// 3.2 (SW1 이 LOW 이면 calibration 모드로 작동하도록 Can_State_Ptr 를 calibration 로 변경
	// 3.3 (SW1 이 HIGH 이고 SW1 이 LOW 이면 factory_mode 모드로 작동하도록 Can_State_Ptr 를 factory_mode 로 변경
	// 3.4 (SW1 이 HIGH 이고 SW1 이 HIGH 이면 operation_mode 모드로 작동하도록 Can_State_Ptr 를 operation_mode 로 변경
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


