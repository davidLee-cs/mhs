/*
 * lib_mhsStd.c
 *
 * 기능 :문자열을 기능별 변환 기능
 		정수변환, 문자열 분리, 문자열 길이, 두 문자열 비교, 정수를 ASCII 로 변환 , 정수를 문자열로 변환하는 기능
 * 구성요소 : 
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */


#include <lib_mhsStd.h>
#include "Typedef.h"
#include <string.h>

/*
기능설명
 문자열의 길이를 계산하는 함수

입출력 변수
char8_t c : 1바이트 정수 입력
*/
static char8_t ToDigitNum(char8_t c)
{
    char8_t r = c;
    char8_t z = (char8_t)0;
    char8_t n = (char8_t)9;

	// 1. c가 0~9 범위인지 체크
    if((r >= z)  && (r <= n))
    {
    	//1.1  c가 숫자라면, ASCII로 변환
        r = (char8_t)(c + 48);
    }

	// 2. 변환된 값을 반환
    return r;
}

/*
기능설명
  버퍼를 아스키코드 ‘0’ 으로 저장
입출력 변수
char8_t *str : 아스키코드‘0’ 으로 변환한 문자열을 str 버퍼에 저장
int16_t str_size : 변환할 문자열 자릿수
*/
void StrZeroFill( char8_t *str, uint16_t iLength )
{
    uint16_t i=0;
    uint16_t iLen = (uint16_t)strlen(str);
    uint16_t iMLen;

    if(iLength >= iLen)
    {
        iMLen =  iLength - iLen;
        char8_t szBuf[20+1];
        (void)memset(szBuf, 0x00, sizeof(szBuf));

        for( i=0U; i<iMLen; i++ )
        {
            szBuf[i] = '0';
        }

        szBuf[iMLen] = '\0';
        (void)strcat(szBuf, str);
        (void)strcpy(str, szBuf);

    }

}

/*
기능설명
  32bit 정수를 문자열로 변환하는 함수

입출력 변수
int32_t lval : 변환할 32 bit 정수를 입력
char8_t *str : 변환한 문자열을 str 버퍼에 저장
int16_t str_size : 변환할 문자열 자릿수
*/
int16_t LToStr(int32_t lval, char8_t *str, int16_t str_size)
{
    int16_t dval, index=0;
    int32_t maxcolumn = 100000000;
    uint16_t maxcol = 10;
    int16_t r;

	// 1. 음수이면 1을 반환하여 종료 
    if(lval < 0)
    {
        r =  1;
    }
    else
    {
    	// 1.1 최대 자릿수를 찾기 위한 루프
        while(maxcolumn != 0)
        {
        	//1.1.1 lval이 maxcolumn보다 크면 중단
            if(lval >= maxcolumn)
            {
                break;
            }

			// 1.1.2 lval보다 작아질 때까지 반복하여 자릿수를 줄임
            if(maxcol > 0U)
            {
                maxcol--;
            }

            maxcolumn /=10;
        }

		//1.2 lval을 문자열로 변환 
        while(maxcolumn != 0)
        {
        	// 1.2.1 lval을 자릿수로 나눔
            int32_t v = lval/maxcolumn;
            dval = (int16_t)v;

			// 1.2.2. ToDigitNum 함수를 사용하여 문자로 변환, 변환된 문자는 str 배열에 저장
            if ((index >= 0) && (index < (str_size - 1)))
            {
                str[index++] = ToDigitNum((char8_t)dval);
            }

			//1.2.3 현재 자릿수 제거하고 자릿수를 줄임.
            lval -= dval *maxcolumn;
            maxcolumn /= 10;
        }

		//1.3 index가 유효한 범위인지 확인하고 문자열 종료
        if ((index >= 0) && (index < str_size)) 
        {
            str[index] = '\0';
        }
        else
        {
            // 1.3.1 index가 유효하지 않으면 마지막 문자에 안전하게 null 추가
            str[str_size - 1] = '\0';
        }

        r = 0;
    }

	// 2. 변환 결과값 리턴 , 0: 정상 변환 , 1: 변환 안됨 
    return r;
}

