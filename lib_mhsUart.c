/*
* 기능 : 송수신 데이터 인터럽트 처리
 * 이력 :
 *    2024.05.23 : 이충순 : 초기 작성
 * 이력 :
*/
#include <lib_mhsAdc.h>
#include <lib_mhsUart.h>
#include "init.h"

uint16_t rDataPointA[100] = {0}; 	// 터미널로부터 명령 수신을 위한  버퍼 
uint16_t gRx_done = 0U;	// 명령 수신 완료 플레그 , 1 : 수신완료, 0 : 미수신 또는 진행중
int16_t gRx_cnt	= 0;		// 수신되어 처리하지 않은 데이터 갯수


//  uart 수신 인터럽트 서비스 루틴 함수
// 기능 : uart 수신 인터럽트 서비스 루틴 함수
// 전역 입출력 변수 :
//     uint16_t  rDataPointA[] : 수신 데이터 저장
//     int16_t gRx_cnt : 수신 데이터 사이즈 카운터
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성
__interrupt void INT_mySCI0_RX_ISR(void)
{
    uint16_t rDataA[2];

	//1. 수신 버퍼에서 한 문자를 읽어와 rDataA[0] 위치에 저장
    SCI_readCharArray(SCIA_BASE, rDataA, 1);

	// 2. 수신된 문자가 0x0D인지 확인
    if((rDataA[0] & 0x00FFU) == 0x0DU)
    {
		// 2.1 수신 문자가 0x0D 이면 메세지 입력 완료, gRx_done = 1로 설정하여 메세지 완료 전달, 수신 버퍼 카운터는 -1로 초기화 
        gRx_done = 1U;
        gRx_cnt = -1;
    }
    else
    {
    	// 2.2 수신된 문자가 캐리지 리턴(0x0D)이 아닐 경우, rDataPointA 버퍼의 gRx_cnt 위치에 해당 문자를 저장
        rDataPointA[gRx_cnt] = rDataA[0];
    }

	// 3. SCI 모듈의 오버플로 상태를 초기화
    SCI_clearOverflowStatus(SCIA_BASE);

	// 4. SCI 수신 FIFO 버퍼(RXFF)에 대한 인터럽트 상태 플래그를 초기화
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXFF);

	// 5. 인터럽트 그룹 9의 ACK 플래그를 초기화
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);

	// 6. 카운터 gRx_cnt를 1 증가시켜 다음 수신된 문자가 있을 때 rDataPointA 배열의 다음 위치에 저장
    gRx_cnt++;
}

//  uart 수신 인터럽트 서비스 루틴 함수
// 기능 : uart 송신 인터럽트 서비스 루틴 함수
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성
__interrupt void INT_mySCI0_TX_ISR(void)
{

    Interrupt_disable(INT_mySCI0_TX);
	// 1.  SCI 송신 FIFO 버퍼(TXFF)의 인터럽트 상태 플래그를 초기화
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_TXFF);

	// 2. 인터럽트 그룹 9의 ACK 플래그를 초기화
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}


