/*
* ��� : �ۼ��� ������ ���ͷ�Ʈ ó��
 * �̷� :
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 * �̷� :
*/
#include <lib_mhsAdc.h>
#include <lib_mhsUart.h>
#include "init.h"

uint16_t rDataPointA[100] = {0}; 	// �͹̳ηκ��� ��� ������ ����  ���� 
uint16_t gRx_done = 0U;	// ��� ���� �Ϸ� �÷��� , 1 : ���ſϷ�, 0 : �̼��� �Ǵ� ������
int16_t gRx_cnt	= 0;		// ���ŵǾ� ó������ ���� ������ ����


//  uart ���� ���ͷ�Ʈ ���� ��ƾ �Լ�
// ��� : uart ���� ���ͷ�Ʈ ���� ��ƾ �Լ�
// ���� ����� ���� :
//     uint16_t  rDataPointA[] : ���� ������ ����
//     int16_t gRx_cnt : ���� ������ ������ ī����
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ�
__interrupt void INT_mySCI0_RX_ISR(void)
{
    uint16_t rDataA[2];

	//1. ���� ���ۿ��� �� ���ڸ� �о�� rDataA[0] ��ġ�� ����
    SCI_readCharArray(SCIA_BASE, rDataA, 1);

	// 2. ���ŵ� ���ڰ� 0x0D���� Ȯ��
    if((rDataA[0] & 0x00FFU) == 0x0DU)
    {
		// 2.1 ���� ���ڰ� 0x0D �̸� �޼��� �Է� �Ϸ�, gRx_done = 1�� �����Ͽ� �޼��� �Ϸ� ����, ���� ���� ī���ʹ� -1�� �ʱ�ȭ 
        gRx_done = 1U;
        gRx_cnt = -1;
    }
    else
    {
    	// 2.2 ���ŵ� ���ڰ� ĳ���� ����(0x0D)�� �ƴ� ���, rDataPointA ������ gRx_cnt ��ġ�� �ش� ���ڸ� ����
        rDataPointA[gRx_cnt] = rDataA[0];
    }

	// 3. SCI ����� �����÷� ���¸� �ʱ�ȭ
    SCI_clearOverflowStatus(SCIA_BASE);

	// 4. SCI ���� FIFO ����(RXFF)�� ���� ���ͷ�Ʈ ���� �÷��׸� �ʱ�ȭ
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXFF);

	// 5. ���ͷ�Ʈ �׷� 9�� ACK �÷��׸� �ʱ�ȭ
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);

	// 6. ī���� gRx_cnt�� 1 �������� ���� ���ŵ� ���ڰ� ���� �� rDataPointA �迭�� ���� ��ġ�� ����
    gRx_cnt++;
}

//  uart ���� ���ͷ�Ʈ ���� ��ƾ �Լ�
// ��� : uart �۽� ���ͷ�Ʈ ���� ��ƾ �Լ�
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ�
__interrupt void INT_mySCI0_TX_ISR(void)
{

    Interrupt_disable(INT_mySCI0_TX);
	// 1.  SCI �۽� FIFO ����(TXFF)�� ���ͷ�Ʈ ���� �÷��׸� �ʱ�ȭ
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_TXFF);

	// 2. ���ͷ�Ʈ �׷� 9�� ACK �÷��׸� �ʱ�ȭ
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}


