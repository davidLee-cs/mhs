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

#if 0
/*
기능설명
  uart 송신 인터럽트 서비스 루틴 함수

입출력 변수
char8_t *str : 문자열을 입력
return : 문자열을 정수로 변환된 값을 리턴
*/
int16_t my_atoi(const char8_t *str)
{
    int16_t   sign;
    int16_t   value;

    sign = 1;
    value = 0;

	// 1. 입력 문자가 NULL 인지 확인 
    if(str != NULL)
    {
    	// 1.1 입력 문자열이 NULL이 아닌 경우, 문자열의 시작 부분에서 공백 문자(탭, 줄 바꿈, 수직 탭, 폼 피드, 캐리지 리턴 및 일반 공백)를 무시함 
        while ((*str == '\t') || (*str == '\n') || (*str == '\v') || (*str == '\f') || (*str == '\r') || (*str == ' '))
        {
            str++;
        }

		// 1.2 문자열에서 + 또는 - 기호를 확인하여, - 기호가 있을 경우 sign을 -1로 변경
        if ((*str == '+') || (*str == '-'))
        {
            if (*str == '-')
            {
                sign *= -1;
            }
            str++;
        }

		// 1.3 *str이 '0'과 '9' 사이에 있을 경우, 해당 문자를 정수로 변환
        while ((*str >= '0') && (*str <= '9'))
        {
        	// 1.3.1 ASCII 코드에서 '0'의 값을 빼서 실제 숫자 값을 입력
            char8_t v = *str - '0';
			//1.3.2 value에 10을 곱하고 현재 읽은 숫자를 더하여 정수를 형성
            value = (value * 10) + (int16_t)v;  
            str++;
        }
    }

	//2. 문자열을 정수로 변환된 값을 리턴
    return (value * sign);
}

#if 0
// 구분자(delimiter)를 기반으로 주어진 문자열을 잘라서 각각의 토큰을 반환하는 함수 
char8_t *my_strtok(char8_t *str, const char8_t *deli)
{
    uint16_t i;
    uint16_t deli_len = my_strlen(deli); //deli가 몇개있는지 알아야 for문 케이스 검사가능
    static char8_t* tmp = NULL; //
    char8_t* chk = NULL;
    char8_t* idx = tmp;

	// 1. 문자열 입력에서  tmp에 값이 비어 있는 경우 tmp에 str 대입
    if((str != NULL) && !tmp){
        tmp = str;
    }
	// 2.만약 str이 NULL이고 tmp도 비어 있다면, chk를 NULL로 설정하여 종료 조건을 처리
    if((str == NULL) && !tmp){ //
        chk = NULL;
    }
    else
    {
		while(1){
        	//2.1 idx가 가리키는 위치에서 구분자와 비교하여, 구분자를 만나면 idx를 이동	
            for(i = 0; i < deli_len; i++) {
                if(*idx == deli[i]) {
                    idx++;
                    break;
                }
            }

			// 2.2 구분자를 모두 확인한 후, 구분자가 아닌 문자를 만나면 그 위치를 tmp에 저장하고 루프를 종료
            if(i == deli_len) { 
                tmp = idx; 
                break;
            }
        }
    }

	// 3. tmp가 가리키는 문자가 문자열의 끝('\0')인 경우, tmp를 NULL로 설정하고 chk에도 NULL을 대입
    if(*tmp == '\0') {
        tmp = NULL;
        chk = tmp;
    }
    else
    {
        // 3.1 구분자를 만나면 NULL로 변환 후 종료 
        while(*tmp != '\0') {
            for(i = 0; i < deli_len; i++)
			{
				//3.1.1 tmp에 구분자를 만난 경우 해당 위치에 '\0'을 대입하여 문자열 저장
                if(*tmp == deli[i])
				{
                    *tmp = '\0'; 
                    break;
                }
            }

			// 3.1.2 구분자를 만나면 반복 종료
            tmp++;
            if (i < deli_len){
                break;
            }
        }
        chk =  idx;
    }

    return chk;
}
#endif

/*
기능설명
 deli에 주어진 구분자를 기준으로 문자열을 나누고, 다음 토큰을 요청할 때마다 NULL이 아닌 부분을 반환

입출력 변수
char8_t *str : 문자열 포인터 입력
const char8_t *deli : 문자열 구분할 특정 구분자
*/
char8_t* my_strtok(char8_t* str, const char8_t* delimiters) {
    static char8_t* pCurrent;
    const char8_t* pDelimit; // const 유지
    char8_t* result = NULL; // 반환할 값을 저장할 변수

    // 1. pCurrent 초기화
    if (str != NULL) {
        pCurrent = str;
    } else {
        str = pCurrent;
    }

    // 2. 현재 위치(pCurrent)가 문자열 중 NULL이면 함수는 NULL을 반환하여 문자열이 없음을 알림.
    if (*pCurrent == '\0') {
        result = NULL;
    } else {
        // 3. 각 문자를 delimiters와 비교하여 구분자인지 확인.
        while (*pCurrent != '\0') { // 반복문의 제어식은 bool 타입 결과
            pDelimit = delimiters; // const 포인터 사용

            while (*pDelimit != '\0') { // 내부 반복문의 제어식도 bool 타입 결과
                if (*pCurrent == *pDelimit) {
                    *pCurrent = '\0';
                    ++pCurrent;
                    result = str;
                    break;
                }
                ++pDelimit;
            }

            if (result != NULL) {
                break;
            }

            ++pCurrent;
        }

        // 4. 구분자가 없고 문자열이 끝난 경우 처리
        if ((result == NULL) && (*pCurrent == '\0')) {
            result = str;
        }
    }

    return result; // return 문은 단 한 번만 사용
}
#if 0
char8_t* my_strtok(char8_t* str, const char8_t* delimiters) {
    static char8_t* pCurrent;
    const char8_t* pDelimit; // const 유지
    const char8_t* result = NULL; // 반환할 값을 저장할 변수

    // 1. pCurrent 초기화
    if (str != NULL) {
        pCurrent = str;
    }

    // 2. 현재 위치(pCurrent)가 문자열 중 NULL이면 함수는 NULL을 반환하여 문자열이 없음을 알림.
    if (*pCurrent == '\0') {
        result = NULL;
    } else {
        // 3. 각 문자를 delimiters와 비교하여 구분자인지 확인.
        while (*pCurrent != '\0') { // 반복문의 제어식은 bool 타입 결과
            pDelimit = delimiters; // const 포인터 사용

            while (*pDelimit != '\0') { // 내부 반복문의 제어식도 bool 타입 결과
                if (*pCurrent == *pDelimit) {
                    *pCurrent = '\0';
                    ++pCurrent;
                    result = str;
                    break;
                }
                ++pDelimit;
            }

            if (result != NULL) {
                break;
            }

            ++pCurrent;
        }

        // 4. 구분자가 없고 문자열이 끝난 경우 처리
        if ((result == NULL) && (*pCurrent == '\0')) {
            result = str;
        }
    }

    return result; // return 문은 단 한 번만 사용
}
#endif
/*
기능설명
 문자열의 길이를 계산하는 함수

입출력 변수
const char8_t *str : 문자열 포인터 입력
*/
uint16_t my_strlen(const char8_t *str)
{
    uint16_t cnt=0;
	// 1. str[cnt]가 널 문자('\0')가 아닐 때까지 계속 반복합니다. 각 반복에서 카운터 cnt를 증가 시킴.
    while(str[cnt] !='\0'){
        ++cnt;
    }

	//2. 문자열의 길이  cnt 값을  리턴 
    return cnt;
}

/*
기능설명
 문자열의 길이를 계산하는 함수

입출력 변수
const char8_t *str : 문자열 포인터 입력
*/
int16_t my_strncmp(const char8_t *s1, const char8_t *s2, uint16_t n)
{
    uint16_t  i=0;
    char8_t   r=0;

	// 1.  주어진 n만큼 문자열을 비교합니다. 두 문자열 모두 널 문자가 아닐 경우 계속 비교 
    while ((i < n) && ((s1[i] != '\0') && (s2[i] != '\0'))) {
		// 1.1 서로 다른 문자가 있으면 정지하고 그 차이를 반환
        if (s1[i] != s2[i]) {
            r = s1[i] - s2[i];  // 서로 다른 문자가 있으면 그 차이를 반환
            break;
        }
        i++;
    }

	// 2. 두 문자열 차이값 리턴, 두 문자열이 동일하면 0으로 반환  
    return (int16_t)r;
}


static char8_t *my_strcpy(char8_t *dest, const char8_t *src)
{
    int16_t i=0;
    char8_t *done = NULL;

    if((src == NULL) || (dest == NULL))
    {
        done = NULL; // NULL 포인터 방어
    }
    else
    {
        do {
            *(dest + i) = *(src + i);
        } while (*(src + i++) != '\0');

        done = dest;
    }

    return done;
}
#endif

#if 1

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


#if 0
//  문자열 버퍼의 내용을 초기화하는 함수 
void bufclear(char8_t *p, int16_t n)
{
    int16_t i;

	//1. 입력으로 받은 포인터 위치부터 n개의 문자를 null 문자('\0')로 설정하여 초기화
    for(i=0; i <n; i++){
        *p++= '\0';
    }
}

void *my_memset(void *src, char8_t value, uint16_t num)
{
        uint16_t i;
        char8_t* p = (char8_t*) src;

        for (i = 0U; i < num; i++) {
            p[i] = (char8_t) value;
        }

        return src;  // 원래 포인터를 반환
}


void *my_memcpy(void *dest,const void *src, uint16_t len)
{
    char8_t *pdest = (char8_t*)dest;
    const char8_t *psrc = (char8_t*)src;
    while(len--)
    {
        *pdest++ = *psrc++;
    }
    return dest;
}

int16_t my_strcmp(const char8_t *s1, const char8_t *s2)
{
    int16_t i=0;
    char8_t r=0;

    // 두 문자열을 하나씩 비교합니다.
    while ((s1[i] != '\0') && (s2[i] != '\0')) {
        if (s1[i] != s2[i]) {
            break;
        }
        i++;
    }

    r = s1[i] - s2[i];
    return (int16_t)r;

}


#endif


#endif
