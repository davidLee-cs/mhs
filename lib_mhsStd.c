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

/*
��ɼ���
 ���ڿ��� ���̸� ����ϴ� �Լ�

����� ����
char8_t c : 1����Ʈ ���� �Է�
*/
static char8_t ToDigitNum(char8_t c)
{
    char8_t r = c;
    char8_t z = (char8_t)0;
    char8_t n = (char8_t)9;

	// 1. c�� 0~9 �������� üũ
    if((r >= z)  && (r <= n))
    {
    	//1.1  c�� ���ڶ��, ASCII�� ��ȯ
        r = (char8_t)(c + 48);
    }

	// 2. ��ȯ�� ���� ��ȯ
    return r;
}

/*
��ɼ���
  ���۸� �ƽ�Ű�ڵ� ��0�� ���� ����
����� ����
char8_t *str : �ƽ�Ű�ڵ塮0�� ���� ��ȯ�� ���ڿ��� str ���ۿ� ����
int16_t str_size : ��ȯ�� ���ڿ� �ڸ���
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
��ɼ���
  32bit ������ ���ڿ��� ��ȯ�ϴ� �Լ�

����� ����
int32_t lval : ��ȯ�� 32 bit ������ �Է�
char8_t *str : ��ȯ�� ���ڿ��� str ���ۿ� ����
int16_t str_size : ��ȯ�� ���ڿ� �ڸ���
*/
int16_t LToStr(int32_t lval, char8_t *str, int16_t str_size)
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
                str[index++] = ToDigitNum((char8_t)dval);
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

