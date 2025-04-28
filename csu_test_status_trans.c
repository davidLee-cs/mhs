/*
 * test_status_trans.c
 *
 * 기능 : Factory_mode 상태에서 외부 시험장비로 자기장, 가속도 값을 RS323 시리얼통신을 통해 송신한다.
 * 구성요소 : 시험장비 상태 정보 송신 CSU (D-MHS-SFR-015)
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */


//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "mhs_project.h"

/*
기능 설명
 factory mode 에서 자기장, 가속도 센서의 측정된 데이터를 터미널로 전송하는 함수

입출력 전역변수
float64_t Box = 0.0L;  자기장 x축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
float64_t Boy = 0.0L;  자기장 y축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
float64_t Boz = 0.0L;  자기장 z축, 단위 gauss, 정밀도 100nT , 범위    -10000 ~ +10000
float64_t g0x = 0.0L;   가속도 x축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
float64_t g0y = 0.0L;   가속도 y축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
float64_t g0z = 0.0L;   가속도 z축, 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
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

		// 1. 자기장 x축값 부호 확인 
        if(Box < 0.0L){
			// 1.1 자기장 x축값이 0보다 작으면 minusFlag[]를 1로 설정, 자기장 x축값에 -1을 곱하여 양수로 변경 후 터미널에 전송할 버퍼에 저장 
            minusFlag[0]=1;
            float64_t x = Box * -1.0L;
            mydata[0] = (int32_t)x;
        }else{
			// 1.2 자기장 x축값이 0보다 크면 minusFlag[]를 0로 설정, 자기장 x축값을 터미널에 전송할 버퍼에 저장 
            minusFlag[0]=0;
            mydata[0] = (int32_t)Box;
        }

		// 2. 자기장 y축값 부호 확인 
        if(Boy < 0.0L){
			// 2.1 자기장 y축값이 0보다 작으면 minusFlag[]를 1로 설정, 자기장 y축값에 -1을 곱하여 양수로 변경 후 터미널에 전송할 버퍼에 저장 
            minusFlag[1]=1;
            float64_t y = Boy * -1.0L;
            mydata[1] = (int32_t)y;
        }else{
			// 2.2 자기장 y축값이 0보다 크면 minusFlag[]를 0로 설정, 자기장 y축값을 터미널에 전송할 버퍼에 저장 
            minusFlag[1]=0;
            mydata[1] = (int32_t)Boy;
        }

		// 3. 자기장 z축값 부호 확인 
        if(Boz < 0.0L){
			// 3.1 자기장 z축값이 0보다 작으면 minusFlag[]를 1로 설정, 자기장 z축값에 -1을 곱하여 양수로 변경 후 터미널에 전송할 버퍼에 저장 
            minusFlag[2]=1;
            float64_t z = Boz * -1.0L;
            mydata[2] = (int32_t)z;
        }else{
			// 3.2 자기장 z축값이 0보다 크면 minusFlag[]를 0로 설정, 자기장 z축값을 터미널에 전송할 버퍼에 저장 
            minusFlag[2]=0;
            mydata[2] = (int32_t)Boz;
        }

		// 4. 가속도 x축값 부호 확인 
        if(g0x < 0.0L){
			// 4.1 가속도 x축값이 0보다 작으면 minusFlag[]를 1로 설정, 가속도 x축값에 -1을 곱하여 양수로 변경 후 터미널에 전송할 버퍼에 저장 
            minusFlag[3]=1;
            float64_t ax = g0x *  -1.0L;
            faccx = ax * 10000.0L;
            mydata[3] = (int32_t)(faccx);
        }else{
			// 4.2 가속도 x축값이 0보다 크면 minusFlag[]를 0로 설정, 자기장 x축값을 터미널에 전송할 버퍼에 저장 
            minusFlag[3]=0;
            faccx = g0x * 10000.0L;
            mydata[3] = (int32_t)(faccx);
        }

		// 5. 가속도 y축값 부호 확인 
        if(g0y < 0.0L){
			// 5.1 가속도 y축값이 0보다 작으면 minusFlag[]를 1로 설정, 가속도 y축값에 -1을 곱하여 양수로 변경 후 터미널에 전송할 버퍼에 저장 
            minusFlag[4]=1;
            float64_t ay = g0y *  -1.0L;
            faccy = ay * 10000.0L;
            mydata[4] = (int32_t)(faccy);
        }else{
			// 5.2 가속도 y축값이 0보다 크면 minusFlag[]를 0로 설정, 자기장 y축값을 터미널에 전송할 버퍼에 저장 
            minusFlag[4]=0;
            faccy = g0y * 10000.0L;
            mydata[4] = (int32_t)(faccy);
        }

		// 6. 구분자 설정 후 터미널에 데이터 전송
        SCI_writeCharArray(SCIA_BASE, (const int8_t*)start, 2U);

		// 7. 자기장, 가속도 값을 터미널로 전송
        for(m=0; m<5; m++)
        {
        	// 7.1 자기장, 가속도값이 음수이면 터미널에 데이터 전송 시 '-' 전송 , 양수이면 '+' 전송 
            if(minusFlag[m]== 1)
            {
                SCI_writeCharArray(SCIA_BASE, (const int8_t*)minus, 1U);
            }
            else
            {
                SCI_writeCharArray(SCIA_BASE, (const int8_t*)plus, 1U);
            }

			// 7.2 전송할 데이터를 문자열로 변환
            (void)LToStr(mydata[m],msg2,6);
            StrZeroFill(msg2,6);
            SCI_writeCharArray(SCIA_BASE, (const int8_t*)msg2, 6U);

			//7.3 데이터 전송 후 구분자 ',' 전송 
            SCI_writeCharArray(SCIA_BASE, (const int8_t*)comma, 1U);
        }

		// 8. 자기장, 가속도 데이터 모두 전송 후 END 구분자 전송
        SCI_writeCharArray(SCIA_BASE, (const int8_t*)end, 2U);


}
