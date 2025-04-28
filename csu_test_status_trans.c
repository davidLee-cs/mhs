/*
 * test_status_trans.c
 *
 * ��� : Factory_mode ���¿��� �ܺ� �������� �ڱ���, ���ӵ� ���� RS323 �ø�������� ���� �۽��Ѵ�.
 * ������� : ������� ���� ���� �۽� CSU (D-MHS-SFR-015)
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */


//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "mhs_project.h"

/*
��� ����
 factory mode ���� �ڱ���, ���ӵ� ������ ������ �����͸� �͹̳η� �����ϴ� �Լ�

����� ��������
float64_t Box = 0.0L;  �ڱ��� x��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
float64_t Boy = 0.0L;  �ڱ��� y��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
float64_t Boz = 0.0L;  �ڱ��� z��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
float64_t g0x = 0.0L;   ���ӵ� x��, ���� g, ���е�  , ����    0 ~  +/- 1.6g
float64_t g0y = 0.0L;   ���ӵ� y��, ���� g, ���е�  , ����    0 ~  +/- 1.6g
float64_t g0z = 0.0L;   ���ӵ� z��, ���� g, ���е�  , ����    0 ~  +/- 1.6g
*/
void status_transfer_to_terminal(void)
{
        int8_t msg2[100]={0,} ;
        int16_t minusFlag[5];
        int32_t mydata[5];
        int16_t m;
        float64_t faccx;
        float64_t faccy;

        const int8_t comma[] = {','};
        const int8_t start[] = {'$',','};
        const int8_t plus[] = {'+'};
        const int8_t minus[] = {'-'};
        const int8_t end[] = {'\r', '\n'};

		// 1. �ڱ��� x�ప ��ȣ Ȯ�� 
        if(Box < 0.0L){
			// 1.1 �ڱ��� x�ప�� 0���� ������ minusFlag[]�� 1�� ����, �ڱ��� x�ప�� -1�� ���Ͽ� ����� ���� �� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[0]=1;
            float64_t x = Box * -1.0L;
            mydata[0] = (int32_t)x;
        }else{
			// 1.2 �ڱ��� x�ప�� 0���� ũ�� minusFlag[]�� 0�� ����, �ڱ��� x�ప�� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[0]=0;
            mydata[0] = (int32_t)Box;
        }

		// 2. �ڱ��� y�ప ��ȣ Ȯ�� 
        if(Boy < 0.0L){
			// 2.1 �ڱ��� y�ప�� 0���� ������ minusFlag[]�� 1�� ����, �ڱ��� y�ప�� -1�� ���Ͽ� ����� ���� �� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[1]=1;
            float64_t y = Boy * -1.0L;
            mydata[1] = (int32_t)y;
        }else{
			// 2.2 �ڱ��� y�ప�� 0���� ũ�� minusFlag[]�� 0�� ����, �ڱ��� y�ప�� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[1]=0;
            mydata[1] = (int32_t)Boy;
        }

		// 3. �ڱ��� z�ప ��ȣ Ȯ�� 
        if(Boz < 0.0L){
			// 3.1 �ڱ��� z�ప�� 0���� ������ minusFlag[]�� 1�� ����, �ڱ��� z�ప�� -1�� ���Ͽ� ����� ���� �� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[2]=1;
            float64_t z = Boz * -1.0L;
            mydata[2] = (int32_t)z;
        }else{
			// 3.2 �ڱ��� z�ప�� 0���� ũ�� minusFlag[]�� 0�� ����, �ڱ��� z�ప�� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[2]=0;
            mydata[2] = (int32_t)Boz;
        }

		// 4. ���ӵ� x�ప ��ȣ Ȯ�� 
        if(g0x < 0.0L){
			// 4.1 ���ӵ� x�ప�� 0���� ������ minusFlag[]�� 1�� ����, ���ӵ� x�ప�� -1�� ���Ͽ� ����� ���� �� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[3]=1;
            float64_t ax = g0x *  -1.0L;
            faccx = ax * 10000.0L;
            mydata[3] = (int32_t)(faccx);
        }else{
			// 4.2 ���ӵ� x�ప�� 0���� ũ�� minusFlag[]�� 0�� ����, �ڱ��� x�ప�� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[3]=0;
            faccx = g0x * 10000.0L;
            mydata[3] = (int32_t)(faccx);
        }

		// 5. ���ӵ� y�ప ��ȣ Ȯ�� 
        if(g0y < 0.0L){
			// 5.1 ���ӵ� y�ప�� 0���� ������ minusFlag[]�� 1�� ����, ���ӵ� y�ప�� -1�� ���Ͽ� ����� ���� �� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[4]=1;
            float64_t ay = g0y *  -1.0L;
            faccy = ay * 10000.0L;
            mydata[4] = (int32_t)(faccy);
        }else{
			// 5.2 ���ӵ� y�ప�� 0���� ũ�� minusFlag[]�� 0�� ����, �ڱ��� y�ప�� �͹̳ο� ������ ���ۿ� ���� 
            minusFlag[4]=0;
            faccy = g0y * 10000.0L;
            mydata[4] = (int32_t)(faccy);
        }

		// 6. ������ ���� �� �͹̳ο� ������ ����
        SCI_writeCharArray(SCIA_BASE, (const int8_t*)start, 2U);

		// 7. �ڱ���, ���ӵ� ���� �͹̳η� ����
        for(m=0; m<5; m++)
        {
        	// 7.1 �ڱ���, ���ӵ����� �����̸� �͹̳ο� ������ ���� �� '-' ���� , ����̸� '+' ���� 
            if(minusFlag[m]== 1)
            {
                SCI_writeCharArray(SCIA_BASE, (const int8_t*)minus, 1U);
            }
            else
            {
                SCI_writeCharArray(SCIA_BASE, (const int8_t*)plus, 1U);
            }

			// 7.2 ������ �����͸� ���ڿ��� ��ȯ
            (void)LToStr(mydata[m],msg2,6);
            StrZeroFill(msg2,6);
            SCI_writeCharArray(SCIA_BASE, (const int8_t*)msg2, 6U);

			//7.3 ������ ���� �� ������ ',' ���� 
            SCI_writeCharArray(SCIA_BASE, (const int8_t*)comma, 1U);
        }

		// 8. �ڱ���, ���ӵ� ������ ��� ���� �� END ������ ����
        SCI_writeCharArray(SCIA_BASE, (const int8_t*)end, 2U);


}
