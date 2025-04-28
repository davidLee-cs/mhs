/*
 * lib_mhsStd.c
 *
 * ��� :���ڿ��� ��ɺ� ��ȯ ���
 		������ȯ, ���ڿ� �и�, ���ڿ� ����, �� ���ڿ� ��, ������ ASCII �� ��ȯ , ������ ���ڿ��� ��ȯ�ϴ� ���
 * ������� : 
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */


#include <lib_mhsStd.h>
#include "Typedef.h"
#include <string.h>

#if 0
/*
��ɼ���
  uart �۽� ���ͷ�Ʈ ���� ��ƾ �Լ�

����� ����
char_t *str : ���ڿ��� �Է�
return : ���ڿ��� ������ ��ȯ�� ���� ����
*/
int16_t my_atoi(const char_t *str)
{
    int16_t   sign;
    int16_t   value;

    sign = 1;
    value = 0;

	// 1. �Է� ���ڰ� NULL ���� Ȯ�� 
    if(str != NULL)
    {
    	// 1.1 �Է� ���ڿ��� NULL�� �ƴ� ���, ���ڿ��� ���� �κп��� ���� ����(��, �� �ٲ�, ���� ��, �� �ǵ�, ĳ���� ���� �� �Ϲ� ����)�� ������ 
        while ((*str == '\t') || (*str == '\n') || (*str == '\v') || (*str == '\f') || (*str == '\r') || (*str == ' '))
        {
            str++;
        }

		// 1.2 ���ڿ����� + �Ǵ� - ��ȣ�� Ȯ���Ͽ�, - ��ȣ�� ���� ��� sign�� -1�� ����
        if ((*str == '+') || (*str == '-'))
        {
            if (*str == '-')
            {
                sign *= -1;
            }
            str++;
        }

		// 1.3 *str�� '0'�� '9' ���̿� ���� ���, �ش� ���ڸ� ������ ��ȯ
        while ((*str >= '0') && (*str <= '9'))
        {
        	// 1.3.1 ASCII �ڵ忡�� '0'�� ���� ���� ���� ���� ���� �Է�
            char_t v = *str - '0';
			//1.3.2 value�� 10�� ���ϰ� ���� ���� ���ڸ� ���Ͽ� ������ ����
            value = (value * 10) + (int16_t)v;  
            str++;
        }
    }

	//2. ���ڿ��� ������ ��ȯ�� ���� ����
    return (value * sign);
}

#if 0
// ������(delimiter)�� ������� �־��� ���ڿ��� �߶� ������ ��ū�� ��ȯ�ϴ� �Լ� 
char_t *my_strtok(char_t *str, const char_t *deli)
{
    uint16_t i;
    uint16_t deli_len = my_strlen(deli); //deli�� ��ִ��� �˾ƾ� for�� ���̽� �˻簡��
    static char_t* tmp = NULL; //
    char_t* chk = NULL;
    char_t* idx = tmp;

	// 1. ���ڿ� �Է¿���  tmp�� ���� ��� �ִ� ��� tmp�� str ����
    if((str != NULL) && !tmp){
        tmp = str;
    }
	// 2.���� str�� NULL�̰� tmp�� ��� �ִٸ�, chk�� NULL�� �����Ͽ� ���� ������ ó��
    if((str == NULL) && !tmp){ //
        chk = NULL;
    }
    else
    {
		while(1){
        	//2.1 idx�� ����Ű�� ��ġ���� �����ڿ� ���Ͽ�, �����ڸ� ������ idx�� �̵�	
            for(i = 0; i < deli_len; i++) {
                if(*idx == deli[i]) {
                    idx++;
                    break;
                }
            }

			// 2.2 �����ڸ� ��� Ȯ���� ��, �����ڰ� �ƴ� ���ڸ� ������ �� ��ġ�� tmp�� �����ϰ� ������ ����
            if(i == deli_len) { 
                tmp = idx; 
                break;
            }
        }
    }

	// 3. tmp�� ����Ű�� ���ڰ� ���ڿ��� ��('\0')�� ���, tmp�� NULL�� �����ϰ� chk���� NULL�� ����
    if(*tmp == '\0') {
        tmp = NULL;
        chk = tmp;
    }
    else
    {
        // 3.1 �����ڸ� ������ NULL�� ��ȯ �� ���� 
        while(*tmp != '\0') {
            for(i = 0; i < deli_len; i++)
			{
				//3.1.1 tmp�� �����ڸ� ���� ��� �ش� ��ġ�� '\0'�� �����Ͽ� ���ڿ� ����
                if(*tmp == deli[i])
				{
                    *tmp = '\0'; 
                    break;
                }
            }

			// 3.1.2 �����ڸ� ������ �ݺ� ����
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
��ɼ���
 deli�� �־��� �����ڸ� �������� ���ڿ��� ������, ���� ��ū�� ��û�� ������ NULL�� �ƴ� �κ��� ��ȯ

����� ����
char_t *str : ���ڿ� ������ �Է�
const char_t *deli : ���ڿ� ������ Ư�� ������
*/
char_t* my_strtok(char_t* str, const char_t* delimiters) {
    static char_t* pCurrent;
    const char_t* pDelimit; // const ����
    char_t* result = NULL; // ��ȯ�� ���� ������ ����

    // 1. pCurrent �ʱ�ȭ
    if (str != NULL) {
        pCurrent = str;
    } else {
        str = pCurrent;
    }

    // 2. ���� ��ġ(pCurrent)�� ���ڿ� �� NULL�̸� �Լ��� NULL�� ��ȯ�Ͽ� ���ڿ��� ������ �˸�.
    if (*pCurrent == '\0') {
        result = NULL;
    } else {
        // 3. �� ���ڸ� delimiters�� ���Ͽ� ���������� Ȯ��.
        while (*pCurrent != '\0') { // �ݺ����� ������� bool Ÿ�� ���
            pDelimit = delimiters; // const ������ ���

            while (*pDelimit != '\0') { // ���� �ݺ����� ����ĵ� bool Ÿ�� ���
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

        // 4. �����ڰ� ���� ���ڿ��� ���� ��� ó��
        if ((result == NULL) && (*pCurrent == '\0')) {
            result = str;
        }
    }

    return result; // return ���� �� �� ���� ���
}
#if 0
char_t* my_strtok(char_t* str, const char_t* delimiters) {
    static char_t* pCurrent;
    const char_t* pDelimit; // const ����
    const char_t* result = NULL; // ��ȯ�� ���� ������ ����

    // 1. pCurrent �ʱ�ȭ
    if (str != NULL) {
        pCurrent = str;
    }

    // 2. ���� ��ġ(pCurrent)�� ���ڿ� �� NULL�̸� �Լ��� NULL�� ��ȯ�Ͽ� ���ڿ��� ������ �˸�.
    if (*pCurrent == '\0') {
        result = NULL;
    } else {
        // 3. �� ���ڸ� delimiters�� ���Ͽ� ���������� Ȯ��.
        while (*pCurrent != '\0') { // �ݺ����� ������� bool Ÿ�� ���
            pDelimit = delimiters; // const ������ ���

            while (*pDelimit != '\0') { // ���� �ݺ����� ����ĵ� bool Ÿ�� ���
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

        // 4. �����ڰ� ���� ���ڿ��� ���� ��� ó��
        if ((result == NULL) && (*pCurrent == '\0')) {
            result = str;
        }
    }

    return result; // return ���� �� �� ���� ���
}
#endif
/*
��ɼ���
 ���ڿ��� ���̸� ����ϴ� �Լ�

����� ����
const char_t *str : ���ڿ� ������ �Է�
*/
uint16_t my_strlen(const char_t *str)
{
    uint16_t cnt=0;
	// 1. str[cnt]�� �� ����('\0')�� �ƴ� ������ ��� �ݺ��մϴ�. �� �ݺ����� ī���� cnt�� ���� ��Ŵ.
    while(str[cnt] !='\0'){
        ++cnt;
    }

	//2. ���ڿ��� ����  cnt ����  ���� 
    return cnt;
}

/*
��ɼ���
 ���ڿ��� ���̸� ����ϴ� �Լ�

����� ����
const char_t *str : ���ڿ� ������ �Է�
*/
int16_t my_strncmp(const char_t *s1, const char_t *s2, uint16_t n)
{
    uint16_t  i=0;
    char_t   r=0;

	// 1.  �־��� n��ŭ ���ڿ��� ���մϴ�. �� ���ڿ� ��� �� ���ڰ� �ƴ� ��� ��� �� 
    while ((i < n) && ((s1[i] != '\0') && (s2[i] != '\0'))) {
		// 1.1 ���� �ٸ� ���ڰ� ������ �����ϰ� �� ���̸� ��ȯ
        if (s1[i] != s2[i]) {
            r = s1[i] - s2[i];  // ���� �ٸ� ���ڰ� ������ �� ���̸� ��ȯ
            break;
        }
        i++;
    }

	// 2. �� ���ڿ� ���̰� ����, �� ���ڿ��� �����ϸ� 0���� ��ȯ  
    return (int16_t)r;
}


static char_t *my_strcpy(char_t *dest, const char_t *src)
{
    int16_t i=0;
    char_t *done = NULL;

    if((src == NULL) || (dest == NULL))
    {
        done = NULL; // NULL ������ ���
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
��ɼ���
 ���ڿ��� ���̸� ����ϴ� �Լ�

����� ����
char_t c : 1����Ʈ ���� �Է�
*/
static char_t ToDigitNum(char_t c)
{
    char_t r = c;
    char_t z = (char_t)0;
    char_t n = (char_t)9;

	// 1. c�� 0~9 �������� üũ
    if((r >= z)  && (r <= n))
    {
    	//1.1  c�� ���ڶ��, ASCII�� ��ȯ
        r = (char_t)(c + 48);
    }

	// 2. ��ȯ�� ���� ��ȯ
    return r;
}

/*
��ɼ���
  ���۸� �ƽ�Ű�ڵ� ��0�� ���� ����
����� ����
char_t *str : �ƽ�Ű�ڵ塮0�� ���� ��ȯ�� ���ڿ��� str ���ۿ� ����
int16_t str_size : ��ȯ�� ���ڿ� �ڸ���
*/
void StrZeroFill( char_t *str, uint16_t iLength )
{
    uint16_t i=0;
    uint16_t iLen = (uint16_t)strlen(str);
    uint16_t iMLen;

    if(iLength >= iLen)
    {
        iMLen =  iLength - iLen;
        char_t szBuf[20+1];
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
��ɼ���
  32bit ������ ���ڿ��� ��ȯ�ϴ� �Լ�

����� ����
int32_t lval : ��ȯ�� 32 bit ������ �Է�
char_t *str : ��ȯ�� ���ڿ��� str ���ۿ� ����
int16_t str_size : ��ȯ�� ���ڿ� �ڸ���
*/
int16_t LToStr(int32_t lval, char_t *str, int16_t str_size)
{
    int16_t dval, index=0;
    int32_t maxcolumn = 100000000;
    uint16_t maxcol = 10;
    int16_t r;

	// 1. �����̸� 1�� ��ȯ�Ͽ� ���� 
    if(lval < 0)
    {
        r =  1;
    }
    else
    {
    	// 1.1 �ִ� �ڸ����� ã�� ���� ����
        while(maxcolumn != 0)
        {
        	//1.1.1 lval�� maxcolumn���� ũ�� �ߴ�
            if(lval >= maxcolumn)
            {
                break;
            }

			// 1.1.2 lval���� �۾��� ������ �ݺ��Ͽ� �ڸ����� ����
            if(maxcol > 0U)
            {
                maxcol--;
            }

            maxcolumn /=10;
        }

		//1.2 lval�� ���ڿ��� ��ȯ 
        while(maxcolumn != 0)
        {
        	// 1.2.1 lval�� �ڸ����� ����
            int32_t v = lval/maxcolumn;
            dval = (int16_t)v;

			// 1.2.2. ToDigitNum �Լ��� ����Ͽ� ���ڷ� ��ȯ, ��ȯ�� ���ڴ� str �迭�� ����
            if ((index >= 0) && (index < (str_size - 1)))
            {
                str[index++] = ToDigitNum((char_t)dval);
            }

			//1.2.3 ���� �ڸ��� �����ϰ� �ڸ����� ����.
            lval -= dval *maxcolumn;
            maxcolumn /= 10;
        }

		//1.3 index�� ��ȿ�� �������� Ȯ���ϰ� ���ڿ� ����
        if ((index >= 0) && (index < str_size)) 
        {
            str[index] = '\0';
        }
        else
        {
            // 1.3.1 index�� ��ȿ���� ������ ������ ���ڿ� �����ϰ� null �߰�
            str[str_size - 1] = '\0';
        }

        r = 0;
    }

	// 2. ��ȯ ����� ���� , 0: ���� ��ȯ , 1: ��ȯ �ȵ� 
    return r;
}


#if 0
//  ���ڿ� ������ ������ �ʱ�ȭ�ϴ� �Լ� 
void bufclear(char_t *p, int16_t n)
{
    int16_t i;

	//1. �Է����� ���� ������ ��ġ���� n���� ���ڸ� null ����('\0')�� �����Ͽ� �ʱ�ȭ
    for(i=0; i <n; i++){
        *p++= '\0';
    }
}

void *my_memset(void *src, char_t value, uint16_t num)
{
        uint16_t i;
        char_t* p = (char_t*) src;

        for (i = 0U; i < num; i++) {
            p[i] = (char_t) value;
        }

        return src;  // ���� �����͸� ��ȯ
}


void *my_memcpy(void *dest,const void *src, uint16_t len)
{
    char_t *pdest = (char_t*)dest;
    const char_t *psrc = (char_t*)src;
    while(len--)
    {
        *pdest++ = *psrc++;
    }
    return dest;
}

int16_t my_strcmp(const char_t *s1, const char_t *s2)
{
    int16_t i=0;
    char_t r=0;

    // �� ���ڿ��� �ϳ��� ���մϴ�.
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
