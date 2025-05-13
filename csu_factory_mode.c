/*
 * factory_mode.c
 *
 * ��� : factory mode ��ȯ �� ������ ���� �ڱ���, ���ӵ� ������ ���, ������ ����, eeprom �� ����ϴ� ���
 * ������� : Factory mode CSU (D-MHS-SFR-010)
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 *    2024.12.13
 *     - eeprom ����� �κ� ����
 */

#if 1
#include <stdio.h>
#include <stdlib.h>
#endif
#include <stdbool.h>
#include <string.h>
#include "mhs_project.h"


static void sub_function_mode(void);
static void calibrationDataTransfortToTerminal(void);
static void sub_calibrationDataTransfortToTerminal(void);
static void matrixDataTransfortToTerminal(void);
static void sub_matrixDataTransfortToTerminal(void);
static void calAcceSet(void);
static void calFluxSet(void);
static void flux_rightAngleSet(void);
static void accel_rightAngleSet(void);
static void calModeFluxSet(void);
static void chk_ver_Set(void);
static void eepromDataReadCmd(void);

#if 0
static void crc16Set(void);
#endif


uint16_t gfirstOpen_factory;


/*
 ��� ����
 factory ��忡���� �������� �����͸� �����Ͽ� ������ �����͸� eeprom �� �����Ѵ�. (bit ������ ���� ����)

 ���� ���� ����
 Rx_done:     ��� ���� �Ϸ� �÷��� , 1 : ���ſϷ�, 0 : �̼��� �Ǵ� ������
 rDataPointA[100]:   �͹̳ηκ��� ��� ������ ����  ����

 ���ú���
 bSendUartData : ��� ���� ���� ���� (0: �̼۽�, 1: �۽�)
*/
void factory_mode(void)
{
	static uint16_t bSendUartData = 0U;

    const char8_t startCmd[] = {'$','S','T','A','R','T','\0'};
    const char8_t stopCmd[] = {'$','S','T','O','P','\0'};

    const char8_t end[] = {'\r', '\n','\r', '\n','\0'};
    const char8_t openFactory[] =   {'F','A','C','T','O','R','Y','_','M','O','D','E','!','!','!','\0' };
    const char8_t star[] =       {'*','*','*','*','*','*','*','*','*','*','*','*','*','*','*','*','*','*','\0' };

    int16_t cnt;

    // 1. ó�� ���丮 ���� ���� �� ���丮 ��帲�� �˷��ִ� ������ ��� �͹̳ο� �ƽ�Ű �ڵ� ����
    if(gfirstOpen_factory == 1U)
    {
        gfirstOpen_factory = 0U;

        SCI_writeCharArray(SCIA_BASE, (const char8_t*)star, (uint16_t)strlen(star));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, (uint16_t)strlen(end));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)openFactory, (uint16_t)strlen(openFactory));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, (uint16_t)strlen(end));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)star, (uint16_t)strlen(star));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, (uint16_t)strlen(end));
    }


	// 2. �͹̳ηκ��� ���� �Ϸ�� ����� ������
    if(gRx_done == 1U)
    {
    	// 2.1 ���ŵ����Ͱ� start ����̸�, ������ ���� �����͸� �͹̳η� �����ϴ� �Լ��� �۵��ϵ��� bSendUartData �÷��׸� true �� �����Ѵ�.
    	//     start �� �Ǹ� �ڱ���, ���ӵ� ���� �� �ɼ� ���� �ʱ�ȭ �� ��Ŵ.
        if(strncmp(rDataPointA, startCmd, 6) == 0)
        {

#if 0
			Gain_Ax = 0.0001L;
			Gain_Ay = 0.0001L;
			Offset_Ax = 0;
			Offset_Ay = 0;

			Gain_Bx = 1.0L;
			Gain_By = 1.0L;
			Gain_Bz = 1.0L;
			Offset_Bx = 0;
			Offset_By = 0;
			Offset_Bz = 0;
#endif

			// 2.1.1 eeprom ���� �ִ� �ڱ���, ���ӵ� ���� �� �ɼ� �� ���� ����.
            read_parameter();

            bSendUartData = 1U;
        }

		// 2.2 ���ŵ����Ͱ� stop ����̸�, ������ ���� �����͸� �͹̳η� �����ϴ� �Լ��� �۵����� �ʵ��ϵ��� bSendUartData �÷��׸� false �� �����Ѵ�.
        if(strncmp(rDataPointA, stopCmd, 5) == 0)
        {
            bSendUartData = 0U;
        }

        // 2.3 �߰� ��ɾ� ����Ȯ�� �Լ�
        eepromDataReadCmd();
        sub_function_mode();


		// 2.4 ���� ���� clear
        for(cnt=0; cnt<100; cnt++)
        {
            rDataPointA[cnt] = 0;
        }

		// 2.5 ���ŵ����� õ�� �Ϸ� ǥ��
        gRx_done = 0U;
        gRx_cnt = 0;

    }

	// 3. start ���� (bSendUartData True) �̸� �ڱ���� ���ӵ��� �����Ͽ� status_transfer_to_terminal() �� ȣ���Ͽ� ���� �����͸� �͹̳η� ���� �Ѵ�.
    if(bSendUartData == 1U)
    {
        MeasureFlux();
        MeasureAccelation();

        status_transfer_to_terminal();
    }

}

/*
 ��� ����
 factory ��忡���� eeprom ������ �б� ��ɾ� �Է� �� eeprom ���� ������ �б�

 ���� ���� ����
 rDataPointA[100]:   �͹̳ηκ��� ��� ������ ����  ����

*/
static void eepromDataReadCmd(void){

    const char8_t eepromReadCmd[] = {'$','R','E','A','D','\0'};
    // 1.3 ���ŵ����Ͱ� eeprom �� ����� calibarion  ���� ����̸�, calibrationDataTransfortToTerminal() �� �����Ͽ� eeprom �� ����� �����͸� �͹̳ʷ� �����Ѵ�.
    if(strncmp(rDataPointA, eepromReadCmd, 5) == 0)
    {
        calibrationDataTransfortToTerminal();
        sub_calibrationDataTransfortToTerminal();
        matrixDataTransfortToTerminal();
        sub_matrixDataTransfortToTerminal();
    }
}


/*
 ��� ����
 �Էµ� ��ɾ� Ȯ���ϴ� �Լ�

 ���� ���� ����
 rDataPointA[100]:   �͹̳ηκ��� ��� ������ ����  ����

*/
static void sub_function_mode(void)
{

    const char8_t calFluxCmd[] = {'$','F','L','U','X','\0'};
    const char8_t calAccelCmd[] = {'$','A','C','C','E','L','\0'};
    const char8_t calModeOffsetCmd[] = {'$','C','A','L','B','\0'};
    const char8_t accelrightAnglesetCmd[] = {'$','A','R','A','\0'};
    const char8_t fluxrightAnglesetCmd[] = {'$','F','R','A','\0'};
    const char8_t versionCmd[] = {'$','V','E','R','\0'};
#if 0
    const char8_t crc16Cmd[] = {'$','C','R','C','\0'};
#endif

    // 1. ���ŵ����Ͱ� ������ ���ӵ� ���� �Ķ���� �� ���� ����̸�, ������ ���ӵ� ���� �Ķ���� ����  x,y,z�� gain, offset ���� eeprom�� �����Ѵ�.
    if(strncmp(rDataPointA, calAccelCmd, 6) == 0)
    {
        calAcceSet();
    }

    // 2. ���ŵ����Ͱ� ������ �ڱ⼾ ���� �ĸ����� ���� ����̸�, ������ �ڱ��� ���� �Ķ���� ���� x,y,z�� gain, offset ���� eeprom�� �����Ѵ�.
    if(strncmp(rDataPointA, calFluxCmd, 5) == 0)
    {
        calFluxSet();
    }

    // 3. ���ŵ����Ͱ� �װ��� �ڱ��� ���� �ɼ� ������ ���� ����̸�, ������ ���� �ڱ��� ���� �Ķ���� ���� x,y,z�� ���� eeprom�� �����Ѵ�.
    if(strncmp(rDataPointA, calModeOffsetCmd, 5) == 0)
    {
        calModeFluxSet();
    }

    // 4. ���ŵ����Ͱ� ������ ���ӵ� �ຸ�� ����̸�, ��� 2x2 ���� eeprom�� �����Ѵ�.
    if(strncmp(rDataPointA, fluxrightAnglesetCmd, 4) == 0)
    {
        flux_rightAngleSet();
    }

    // 5. ���ŵ����Ͱ� ������ �ڱ��� �ຸ�� ����̸�, ��� 3x3 ���� eeprom�� �����Ѵ�.
    if(strncmp(rDataPointA, accelrightAnglesetCmd, 4) == 0)
    {
        accel_rightAngleSet();
    }

    // 6. ���ŵ����Ͱ� ������ ����Ʈ���� ���� ����̸�, ����Ʈ���� ��������  eeprom�� �����Ѵ�.
    if(strncmp(rDataPointA, versionCmd, 4) == 0)
    {
        chk_ver_Set();
    }

#if 0

    if(strncmp(rDataPointA, crc16Cmd, 4) == 0)
    {
        crc16Set();
    }
#endif


}

/*
��ɼ���
eeprom�� ����Ǿ� �ִ� �ڱ���, ��ӵ� ���� �����͸� �͹̳ο� �����ϴ� �Լ�

���� ���� ����
RX_MsgBuffer[MAX_BUFFER_SIZE];  EEPROM ���� ���� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
*/
static void calibrationDataTransfortToTerminal(void)
{
    char8_t msg[100];

    const char8_t plus[] = {'+'};
    const char8_t minus[] = {'-'};
    const char8_t end[] = {'\r', '\n','\r', '\n'};
    const char8_t error[] = {'e','r','r','o','r','\r','\n','\0'};

    const char8_t accelXGain[] =   {'A','C','C','E','L','_','X','_','G','A','I','N',':',' ',' ',' ','\0' };
    const char8_t accelYGain[] =   {'A','C','C','E','L','_','Y','_','G','A','I','N',':',' ',' ',' ','\0' };
    const char8_t accelXOffset[] = {'A','C','C','E','L','_','X','_','O','F','F','S','E','T',':',' ','\0' };
    const char8_t accelYOffset[] = {'A','C','C','E','L','_','Y','_','O','F','F','S','E','T',':',' ','\0' };

    const char8_t fluxlXGain[] =   {'F','L','U','X','_','_','X','_','G','A','I','N',':',' ',' ',' ','\0' };
    const char8_t fluxlYGain[] =   {'F','L','U','X','_','_','Y','_','G','A','I','N',':',' ',' ',' ','\0' };
    const char8_t fluxlZGain[] =   {'F','L','U','X','_','_','Z','_','G','A','I','N',':',' ',' ',' ','\0' };
    const char8_t fluxXOffset[] =  {'F','L','U','X','_','_','X','_','O','F','F','S','E','T',':',' ','\0' };
    const char8_t fluxYOffset[] =  {'F','L','U','X','_','_','Y','_','O','F','F','S','E','T',':',' ','\0' };
    const char8_t fluxZOffset[] =  {'F','L','U','X','_','_','Z','_','O','F','F','S','E','T',':',' ','\0' };

    int16_t readCalData[20] = {0,};
    uint16_t readCalAddr[20] = {0,};

    uint16_t epaddr;
    int16_t epread;
    int16_t myread;
    uint16_t index=0;
    int16_t readsize=0;

	// 1. eeprom ���� ��� �����͸��� eeprom ���� 1word �� �о� terminal �� �����Ѵ�.
    for(epaddr=EEPROM_GAIN_AX_ADDRESS; epaddr<EEPROM_OFFSET_BZ_ADDRESS+1U; epaddr+=3U)
    {
    	// 1.1 eeprom ���� ���� ������ �б� ���ϴ� ���
        if(data_Read((uint32_t)epaddr) == SUCCESS)
        {
        	// 1.1.1 RX_MsgBuffer[] ���ۿ� ����ִ� 1word �� ��ȯ�Ͽ�    read   ������ �����Ѵ�.
            uint16_t rxh = RX_MsgBuffer[1] << 8;
            uint16_t rxl = RX_MsgBuffer[0] & 0x00FFU;
            uint16_t rxhl = rxh | rxl;
            epread = (int16_t)(rxhl);

            readCalData[readsize] = epread;
            readCalAddr[readsize] = epaddr;
            readsize++;
        }
    }

    // 2. eeprom���� ���� �������� ��巹���� ���Ͽ� �� �ش� �ε��� ���ڿ� ����Ѵ�.
    for(index=0; index < 12U; index++)
    {
        //2.1 �� �ּҸ� ���Ͽ� ���ڿ� ���
        uint16_t eepromAddr = readCalAddr[index];

        switch(eepromAddr)
        {
            case EEPROM_GAIN_AX_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)accelXGain, (uint16_t)strlen((accelXGain)));
                break;

            case EEPROM_GAIN_AY_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)accelYGain, (uint16_t)strlen(accelYGain));
                break;

            case EEPROM_GAIN_AZ_ADDRESS:
                break;

            case EEPROM_OFFSET_AX_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)accelXOffset, (uint16_t)strlen(accelXOffset));
                break;

            case EEPROM_OFFSET_AY_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)accelYOffset, (uint16_t)strlen(accelYOffset));
                break;

            case EEPROM_OFFSET_AZ_ADDRESS:
                break;

            case EEPROM_GAIN_BX_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)fluxlXGain, (uint16_t)strlen(fluxlXGain));
                break;

            case EEPROM_GAIN_BY_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)fluxlYGain, (uint16_t)strlen(fluxlYGain));
                break;

            case EEPROM_GAIN_BZ_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)fluxlZGain, (uint16_t)strlen(fluxlZGain));
                break;

            case EEPROM_OFFSET_BX_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)fluxXOffset, (uint16_t)strlen(fluxXOffset));
                break;

            case EEPROM_OFFSET_BY_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)fluxYOffset, (uint16_t)strlen(fluxYOffset));
                break;

            case EEPROM_OFFSET_BZ_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)fluxZOffset, (uint16_t)strlen(fluxZOffset));
                break;

            default :
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)error, 7U);
                break;

        }

        // 2.2. read ���� ��ȣ�� ���밪�� ����, ���밪�� myRead ������ ����
        if(readCalData[index] < 0){
            myread = readCalData[index] * -1;
            // 2.2.1 ��ȣ�� �͹̳η� ����
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)minus, 1U);
        }else{
            myread = readCalData[index];
            // 2.2.2 ��ȣ�� �͹̳η� ����
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)plus, 1U);
        }

        // 2.3.  myRead ���밪�� �͹̳η� �����Ѵ�.
        int32_t mystr = (int32_t)myread;
        (void)LToStr(mystr,msg,6);
        StrZeroFill(msg,6);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)msg, 6U);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 4U);

    }
}

/*
��ɼ���
eeprom�� ����Ǿ� �ִ� �װ��� ���� �ɼ� �ڱ��� �����͸� �͹̳ο� �����ϴ� �Լ�

���� ���� ����
RX_MsgBuffer[MAX_BUFFER_SIZE];  EEPROM ���� ���� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
*/
static void sub_calibrationDataTransfortToTerminal(void)
{
    char8_t msg[100];

    const char8_t plus[] = {'+'};
    const char8_t minus[] = {'-'};
    const char8_t end[] = {'\r', '\n','\r', '\n'};
    const char8_t error[] = {'e','r','r','o','r','\r','\n','\0'};

    const char8_t calOffset_Bx[] = {'C','A','L','_','_','X','_','_','O','F','F','S','E','T',':',' ','\0' };
    const char8_t calOffset_By[] = {'C','A','L','_','_','Y','_','_','O','F','F','S','E','T',':',' ','\0' };
    const char8_t calOffset_Bz[] = {'C','A','L','_','_','Z','_','_','O','F','F','S','E','T',':',' ','\0' };

    int16_t readCalData[20] = {0,};
    uint16_t readCalAddr[20] = {0,};

    uint16_t epaddr;
    int16_t epread;
    int16_t myread;
    uint16_t index=0;
    int16_t readsize=0;

    // 1. eeprom ���� ��� �����͸��� eeprom ���� 1word �� �о� terminal �� �����Ѵ�.
    for(epaddr=EEPROM_BX_CAL_OFFSET_ADDRESS; epaddr<EEPROM_BZ_CAL_OFFSET_ADDRESS+1U; epaddr+=3U)
    {
        // 1.1 eeprom ���� ���� ������ �б� ���ϴ� ���
        if(data_Read((uint32_t)epaddr) == SUCCESS)
        {
            // 1.1.1 RX_MsgBuffer[] ���ۿ� ����ִ� 1word �� ��ȯ�Ͽ�    read   ������ �����Ѵ�.
            uint16_t rxh = RX_MsgBuffer[1] << 8;
            uint16_t rxl = RX_MsgBuffer[0] & 0x00FFU;
            uint16_t rxhl = rxh | rxl;
            epread = (int16_t)(rxhl);

            readCalData[readsize] = epread;
            readCalAddr[readsize] = epaddr;
            readsize++;
        }
    }

    // 2. eeprom���� ���� �������� ��巹���� ���Ͽ� �� �ش� �ε��� ���ڿ� ����Ѵ�.
    for(index=0; index < 3U; index++)
    {
        //2.1 �� �ּҸ� ���Ͽ� ���ڿ� ���
        uint16_t eepromAddr = readCalAddr[index];

        switch(eepromAddr)
        {
            case EEPROM_BX_CAL_OFFSET_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)calOffset_Bx, (uint16_t)strlen(calOffset_Bx));
                break;

            case EEPROM_BY_CAL_OFFSET_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)calOffset_By, (uint16_t)strlen(calOffset_By));
                break;

            case EEPROM_BZ_CAL_OFFSET_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)calOffset_Bz, (uint16_t)strlen(calOffset_Bz));
                break;
            default :
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)error, 7U);
                break;

        }

        // 2.2. read ���� ��ȣ�� ���밪�� ����, ���밪�� myRead ������ ����
        if(readCalData[index] < 0){
            myread = readCalData[index] * -1;
            // 2.2.1 ��ȣ�� �͹̳η� ����
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)minus, 1U);
        }else{
            myread = readCalData[index];
            // 2.2.2 ��ȣ�� �͹̳η� ����
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)plus, 1U);
        }

        // 2.3.  myRead ���밪�� �͹̳η� �����Ѵ�.
        int32_t mystr = (int32_t)myread;
        (void)LToStr(mystr,msg,6);
        StrZeroFill(msg,6);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)msg, 6U);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 4U);

    }
}


/*
��ɼ���
eeprom�� ����Ǿ� �ִ� �� �ຸ�� �����͸� �͹̳ο� �����ϴ� �Լ�

���� ���� ����
RX_MsgBuffer[MAX_BUFFER_SIZE];  EEPROM ���� ���� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
*/
static void matrixDataTransfortToTerminal(void)
{
    char8_t msg[100];

    const char8_t plus[] = {'+'};
    const char8_t minus[] = {'-'};
    const char8_t end[] = {'\r', '\n','\r', '\n'};
    const char8_t error[] = {'e','r','r','o','r','\r','\n','\0'};

    const char8_t matrix_Bx_00[] = {'M','A','T','R','I','X','_','B','X','0','0','\0' };
    const char8_t matrix_Bx_01[] = {'M','A','T','R','I','X','_','B','X','0','1','\0' };
    const char8_t matrix_Bx_02[] = {'M','A','T','R','I','X','_','B','X','0','2','\0' };

    const char8_t matrix_By_10[] = {'M','A','T','R','I','X','_','B','X','1','0','\0' };
    const char8_t matrix_By_11[] = {'M','A','T','R','I','X','_','B','X','1','1','\0' };
    const char8_t matrix_By_12[] = {'M','A','T','R','I','X','_','B','X','1','2','\0' };

    const char8_t matrix_Bz_20[] = {'M','A','T','R','I','X','_','B','X','2','0','\0' };
    const char8_t matrix_Bz_21[] = {'M','A','T','R','I','X','_','B','X','2','1','\0' };
    const char8_t matrix_Bz_22[] = {'M','A','T','R','I','X','_','B','X','2','2','\0' };

    const char8_t matrix_Ax_00[] = {'M','A','T','R','I','X','_','A','X','0','0','\0' };
    const char8_t matrix_Ax_01[] = {'M','A','T','R','I','X','_','A','X','0','1','\0' };

    const char8_t matrix_Ay_10[] = {'M','A','T','R','I','X','_','A','Y','1','0','\0' };
    const char8_t matrix_Ay_11[] = {'M','A','T','R','I','X','_','A','Y','1','1','\0' };

    int16_t readCalData[20] = {0,};
    uint16_t readCalAddr[20] = {0,};

    uint16_t epaddr;
    int16_t epread;
    int16_t myread;
    uint16_t index=0;
    int16_t readsize=0;

    // 1. eeprom ���� ��� �����͸��� eeprom ���� 1word �� �о� terminal �� �����Ѵ�.
    for(epaddr=EEPROM_BX00_RA_ADDRESS; epaddr<EEPROM_AY11_RA_ADDRESS+1U; epaddr+=3U)
    {
        // 1.1 eeprom ���� ���� ������ �б� ���ϴ� ���
        if(data_Read((uint32_t)epaddr) == SUCCESS)
        {
            // 1.1.1 RX_MsgBuffer[] ���ۿ� ����ִ� 1word �� ��ȯ�Ͽ�    read   ������ �����Ѵ�.
            uint16_t rxh = RX_MsgBuffer[1] << 8;
            uint16_t rxl = RX_MsgBuffer[0] & 0x00FFU;
            uint16_t rxhl = rxh | rxl;
            epread = (int16_t)(rxhl);

            readCalData[readsize] = epread;
            readCalAddr[readsize] = epaddr;
            readsize++;
        }
    }

    for(index=0; index < 13U; index++)
    {
        //2.1 �� �ּҸ� ���Ͽ� ���ڿ� ���
        uint16_t eepromAddr = readCalAddr[index];
        switch(eepromAddr)
        {
            case EEPROM_BX00_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Bx_00, (uint16_t)strlen((matrix_Bx_00)));
                break;

            case EEPROM_BX01_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Bx_01, (uint16_t)strlen(matrix_Bx_01));
                break;

            case EEPROM_BX02_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Bx_02, (uint16_t)strlen(matrix_Bx_02));
                break;

            case EEPROM_BY10_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_By_10, (uint16_t)strlen(matrix_By_10));
                break;

            case EEPROM_BY11_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_By_11, (uint16_t)strlen(matrix_By_11));
                break;

            case EEPROM_BY12_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_By_12, (uint16_t)strlen(matrix_By_12));
                break;

            case EEPROM_BZ20_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Bz_20, (uint16_t)strlen(matrix_Bz_20));
                break;

            case EEPROM_BZ21_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Bz_21, (uint16_t)strlen(matrix_Bz_21));
                break;

            case EEPROM_BZ22_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Bz_22, (uint16_t)strlen(matrix_Bz_22));
                break;

            case EEPROM_AX00_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Ax_00, (uint16_t)strlen(matrix_Ax_00));
                break;

            case EEPROM_AX01_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Ax_01, (uint16_t)strlen(matrix_Ax_01));
                break;

            case EEPROM_AY10_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Ay_10, (uint16_t)strlen(matrix_Ay_10));
                break;

            case EEPROM_AY11_RA_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)matrix_Ay_11, (uint16_t)strlen(matrix_Ay_11));
                break;

            default :
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)error, 7U);
                break;

        }

        // 2.2. read ���� ��ȣ�� ���밪�� ����, ���밪�� myRead ������ ����
        if(readCalData[index] < 0){
            myread = readCalData[index] * -1;
            // 2.2.1 ��ȣ�� �͹̳η� ����
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)minus, 1U);
        }else{
            myread = readCalData[index];
            // 2.2.2 ��ȣ�� �͹̳η� ����
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)plus, 1U);
        }

        // 2.3.  myRead ���밪�� �͹̳η� �����Ѵ�.
        int32_t mystr = (int32_t)myread;
        (void)LToStr(mystr,msg,6);
        StrZeroFill(msg,6);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)msg, 6U);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 4U);

    }
}

/*
��ɼ���
eeprom�� ����Ǿ� �ִ� ����Ʈ���� ���� �� CRC16 �����͸� �͹̳ο� �����ϴ� �Լ�

���� ���� ����
RX_MsgBuffer[MAX_BUFFER_SIZE];  EEPROM ���� ���� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
*/
static void sub_matrixDataTransfortToTerminal(void)
{
    char8_t msg[100];

    const char8_t plus[] = {'+'};
    const char8_t minus[] = {'-'};
    const char8_t end[] = {'\r', '\n','\r', '\n'};
    const char8_t error[] = {'e','r','r','o','r','\r','\n','\0'};

    const char8_t version[] = {'V','E','R','S','I','O','N',' ',':',' ',' ','\0' };
    const char8_t crcRead[] = {'C','R','C','R','E','A','D',' ',':',' ',' ','\0' };


    int16_t readCalData[20] = {0,};
    uint16_t readCalAddr[20] = {0,};

    uint16_t epaddr;
    int16_t epread;
    int16_t myread;
    uint16_t index=0;
    int16_t readsize=0;

    // 1. eeprom ���� ��� �����͸��� eeprom ���� 1word �� �о� terminal �� �����Ѵ�.
    for(epaddr=EEPROM_CHK_SUM_ADDRESS; epaddr<EEPROM_CHK_CRC_ADDRESS+1U; epaddr+=3U)
    {
        // 1.1 eeprom ���� ���� ������ �б� ���ϴ� ���
        if(data_Read((uint32_t)epaddr) == SUCCESS)
        {
            // 1.1.1 RX_MsgBuffer[] ���ۿ� ����ִ� 1word �� ��ȯ�Ͽ�    read   ������ �����Ѵ�.
            uint16_t rxh = RX_MsgBuffer[1] << 8;
            uint16_t rxl = RX_MsgBuffer[0] & 0x00FFU;
            uint16_t rxhl = rxh | rxl;
            epread = (int16_t)(rxhl);

            readCalData[readsize] = epread;
            readCalAddr[readsize] = epaddr;
            readsize++;
        }
    }

    for(index=0; index < 2U; index++)
    {
        //2.1 �� �ּҸ� ���Ͽ� ���ڿ� ���
        uint16_t eepromAddr = readCalAddr[index];
        switch(eepromAddr)
        {
            case EEPROM_VERSION_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)version, (uint16_t)strlen(version));
                break;
            case EEPROM_CHK_CRC_ADDRESS:
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)crcRead, (uint16_t)strlen(crcRead));
                break;
            default :
                SCI_writeCharArray(SCIA_BASE, (const char8_t*)error, 7U);
                break;

        }

        // 2.2. read ���� ��ȣ�� ���밪�� ����, ���밪�� myRead ������ ����
        if(readCalData[index] < 0){
            myread = readCalData[index] * -1;
            // 2.2.1 ��ȣ�� �͹̳η� ����
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)minus, 1U);
        }else{
            myread = readCalData[index];
            // 2.2.2 ��ȣ�� �͹̳η� ����
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)plus, 1U);
        }

        // 2.3.  myRead ���밪�� �͹̳η� �����Ѵ�.
        int32_t mystr = (int32_t)myread;
        (void)LToStr(mystr,msg,6);
        StrZeroFill(msg,6);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)msg, 6U);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 4U);

    }
}


/*
��ɼ���
�͹̳ο��� ���ӵ� �������� �Է� �޾� �� ���� �Ľ� �� gain, offset �� �����Ͽ� eeprom�� ���� �� �͹̳ο� �Ϸ� ack ���� �Լ�

�������� ����
uint16_t rDataPointA[100];  �͹̳ηκ��� ��� ������ ���� ����
*/
static void calAcceSet(void)
{
    const char8_t* comma = ",";
    const char8_t end[] = {'\r', '\n'};

    char8_t buffer[100] = {0,};

    int16_t i;
    for(i=0; i<100; i++)
    {
        buffer[i] = (char8_t)rDataPointA[i];
    }

	// 1. ���ŵ� ���ڿ��� ���� comma ���� ���ڿ��� �и�
    const char8_t* tempacc = strtok(&rDataPointA[7],comma);

    if( tempacc != NULL)
    {

        // 2. ù��° ���� ���ڿ��� x�� ���ӵ� �̵� ���ڿ��� gain_Ax ������ ��ȯ �Ѵ�.
        int16_t gain_Ax = atoi(tempacc) ;      // pc���� 1000000 ���� ���� �޴´�.
        tempacc = strtok(NULL, comma);

        // 3. �ι�° ���� ���ڿ��� y�� ���ӵ� �̵� ���ڿ��� gain_Ay ������ ��ȯ �Ѵ�.
        int16_t gain_Ay = atoi(tempacc);
        tempacc = strtok(NULL, comma);

        // 4. ����° ���� ���ڿ��� x�� ���ӵ� �ɼ� ���ڿ��� offset_Ax ������ ��ȯ �Ѵ�.
        int16_t offset_Ax = atoi(tempacc);
        tempacc = strtok(NULL, comma);

        // 5. �׹�° ���� ���ڿ��� y�� ���ӵ� �ɼ� ���ڿ��� offset_Ay ������ ��ȯ �Ѵ�.
        int16_t offset_Ay = atoi(tempacc);

        tempacc = strtok(NULL, comma);

        // 6. �ټ���° ���� ���ڿ��� y�� ���ӵ� �ɼ� eeprom ���� ������ ���� ���� ��ȯ.
        int16_t testSave = atoi(tempacc) ;

        // 7. ��ȯ�� gain_Ax, gain_Ay, offset_Ax, offset_Ay ���� eeprom�� �����Ѵ�.
        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_GAIN_AX_ADDRESS, (uint16_t)(gain_Ax));
            (void)data_write_to_eeprom(EEPROM_GAIN_AY_ADDRESS, (uint16_t)(gain_Ay));
            (void)data_write_to_eeprom(EEPROM_OFFSET_AX_ADDRESS, (uint16_t)(offset_Ax));
            (void)data_write_to_eeprom(EEPROM_OFFSET_AY_ADDRESS, (uint16_t)(offset_Ay));

            writeEepromCRC16();
        }

        // 8. ������ ���� �� �Ϸ� ack �� �͹̳ο� ���� �Ѵ�.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);

    }
    else
    {

    }
}

/*
��ɼ���
 �͹̳ο��� ���ӵ� �������� �Է� �޾� �� ���� �Ľ� �� gain, offset �� �����Ͽ� eeprom�� ���� ��
 �͹̳ο� �Ϸ� ack ���� �Լ�

�������� ����
uint16_t rDataPointA[100];   �͹̳ηκ��� ��� ������ ����  ����
*/
static void calFluxSet(void)
{
    const char8_t* comma = ",";
    const char8_t end[] = {'\r', '\n'};

    char8_t buffer[100] = {0,};

    int16_t i;
    for(i=0; i<100; i++)
    {
        buffer[i] = (char8_t)rDataPointA[i];
    }

        // 1. ���ŵ� ���ڿ��� ���� comma ���� ���ڿ��� �и�
    const char8_t* tempflux = strtok(&rDataPointA[6],comma);

    if( tempflux != NULL)
    {
        // 2. ù��° ���� ���ڿ��� x�� �ڱ��� �̵� ���ڿ��� gain_Bx ������ ��ȯ �Ѵ�.
        int16_t gain_Bx = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 3. �ι�° ���� ���ڿ��� y�� �ڱ��� �̵� ���ڿ��� gain_By ������ ��ȯ �Ѵ�.
        int16_t gain_By = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 4. ����° ���� ���ڿ��� z�� �ڱ��� �̵� ���ڿ��� gain_Bz ������ ��ȯ �Ѵ�.
        int16_t gain_Bz = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 5. �׹�° ���� ���ڿ��� x�� ���ӵ� �ɼ� ���ڿ��� offset_Bx ������ ��ȯ �Ѵ�.
        int16_t offset_Bx = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 6. �ټ���° ���� ���ڿ��� y�� ���ӵ� �ɼ� ���ڿ��� offset_By ������ ��ȯ �Ѵ�.
        int16_t offset_By = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 7. ������° ���� ���ڿ��� z�� ���ӵ� �ɼ� ���ڿ��� offset_Bz ������ ��ȯ �Ѵ�.
        int16_t offset_Bz = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 8. �ټ���° ���� ���ڿ��� y�� ���ӵ� �ɼ� eeprom ���� ������ ���� ���� ��ȯ.
        int16_t testSave = atoi(tempflux) ;

        // 9. ��ȯ�� gain_Bx, gain_By, gain_Bz, offset_Bx, offset_By, offset_Bz ���� eeprom�� �����Ѵ�.
        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_GAIN_BX_ADDRESS, (uint16_t)(gain_Bx));
            (void)data_write_to_eeprom(EEPROM_GAIN_BY_ADDRESS, (uint16_t)(gain_By));
            (void)data_write_to_eeprom(EEPROM_GAIN_BZ_ADDRESS, (uint16_t)(gain_Bz));
            (void)data_write_to_eeprom(EEPROM_OFFSET_BX_ADDRESS, (uint16_t)(offset_Bx));
            (void)data_write_to_eeprom(EEPROM_OFFSET_BY_ADDRESS, (uint16_t)(offset_By));
            (void)data_write_to_eeprom(EEPROM_OFFSET_BZ_ADDRESS, (uint16_t)(offset_Bz));

            writeEepromCRC16();

        }

        // 10. ������ ���� �� �Ϸ� ack �� �͹̳ο� ���� �Ѵ�.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }
}

/*
��ɼ���
 �͹̳ο��� �ڱ��� �ຸ������ �Է� �޾� �� ���� �Ľ� �� ��� 3x3 ���� eeprom�� ���� ��
 �͹̳ο� �Ϸ� ack ���� �Լ�

�������� ����
uint16_t rDataPointA[100];   �͹̳ηκ��� ��� ������ ����  ����
*/
static void flux_rightAngleSet(void)
{
    const char8_t* comma = ",";
    const char8_t end[] = {'\r', '\n'};

    char8_t buffer[100] = {0,};

    int16_t i;
    for(i=0; i<100; i++)
    {
        buffer[i] = (char8_t)rDataPointA[i];
    }

    // 1. ���ŵ� ���ڿ��� ���� comma ���� ���ڿ��� �и�
    const char8_t* right = strtok(&rDataPointA[5],comma);

    if( right != NULL)
    {
        // 2. ù��° ���� ���ڿ��� ������ x�� 00�� ��ȯ �Ѵ�.
        int16_t x00 = atoi(right) ;
        right = strtok(NULL, comma);

        // 3. �ι�° ���� ���ڿ��� ������ x�� 01�� ��ȯ �Ѵ�.
        int16_t x01 = atoi(right) ;
        right = strtok(NULL, comma);

        // 4. ����° ���� ���ڿ��� ������ x�� 02�� ��ȯ �Ѵ�.
        int16_t x02 = atoi(right) ;
        right = strtok(NULL, comma);

        // 5. �׹�° ���� ���ڿ��� ������ y�� 10�� ��ȯ �Ѵ�.
        int16_t y10 = atoi(right) ;
        right = strtok(NULL, comma);

        // 6. �ټ���° ���� ���ڿ��� ������ y�� 11�� ��ȯ �Ѵ�.
        int16_t y11 = atoi(right) ;
        right = strtok(NULL, comma);

        // 7. ������° ���� ���ڿ��� ������ z�� 12�� ��ȯ �Ѵ�.
        int16_t y12 = atoi(right) ;
        right = strtok(NULL, comma);

        // 8. �׹�° ���� ���ڿ��� ������ z�� 20�� ��ȯ �Ѵ�.
        int16_t z20 = atoi(right) ;
        right = strtok(NULL, comma);

        // 9. �ټ���° ���� ���ڿ��� ������ z�� 21�� ��ȯ �Ѵ�.
        int16_t z21 = atoi(right) ;
        right = strtok(NULL, comma);

        // 10. ������° ���� ���ڿ��� ������ z�� 22�� ��ȯ �Ѵ�.
        int16_t z22 = atoi(right) ;
        right = strtok(NULL, comma);

        // 8. �ټ���° ���� ���ڿ��� ������ eeprom ���� ������ ���� ���� ��ȯ.
        int16_t testSave = atoi(right) ;

        // 9. ��ȯ�� gain_Bx, gain_By, gain_Bz, offset_Bx, offset_By, offset_Bz ���� eeprom�� �����Ѵ�.
        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_BX00_RA_ADDRESS, (uint16_t)(x00));
            (void)data_write_to_eeprom(EEPROM_BX01_RA_ADDRESS, (uint16_t)(x01));
            (void)data_write_to_eeprom(EEPROM_BX02_RA_ADDRESS, (uint16_t)(x02));

            (void)data_write_to_eeprom(EEPROM_BY10_RA_ADDRESS, (uint16_t)(y10));
            (void)data_write_to_eeprom(EEPROM_BY11_RA_ADDRESS, (uint16_t)(y11));
            (void)data_write_to_eeprom(EEPROM_BY12_RA_ADDRESS, (uint16_t)(y12));

            (void)data_write_to_eeprom(EEPROM_BZ20_RA_ADDRESS, (uint16_t)(z20));
            (void)data_write_to_eeprom(EEPROM_BZ21_RA_ADDRESS, (uint16_t)(z21));
            (void)data_write_to_eeprom(EEPROM_BZ22_RA_ADDRESS, (uint16_t)(z22));

            writeEepromCRC16();

        }

        // 10. ������ ���� �� �Ϸ� ack �� �͹̳ο� ���� �Ѵ�.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }

}

/*
��ɼ���
 �͹̳ο��� ���ӵ� �ຸ������ �Է� �޾� �� ���� �Ľ� �� ��� 2x2 ���� eeprom�� ���� ��
 �͹̳ο� �Ϸ� ack ���� �Լ�

�������� ����
uint16_t rDataPointA[100];   �͹̳ηκ��� ��� ������ ����  ����
*/
static void accel_rightAngleSet(void)
{
    const char8_t* comma = ",";
    const char8_t end[] = {'\r', '\n'};

    char8_t buffer[100] = {0,};

    int16_t i;
    for(i=0; i<100; i++)
    {
        buffer[i] = (char8_t)rDataPointA[i];
    }

    // 1. ���ŵ� ���ڿ��� ���� comma ���� ���ڿ��� �и�
    const char8_t* right = strtok(&rDataPointA[5],comma);

    if( right != NULL)
    {
        // 2. ù��° ���� ���ڿ��� ������ x�� 00�� ��ȯ �Ѵ�.
        int16_t x00 = atoi(right) ;
        right = strtok(NULL, comma);

        // 3. �ι�° ���� ���ڿ��� ������ x�� 01�� ��ȯ �Ѵ�.
        int16_t x01 = atoi(right) ;
        right = strtok(NULL, comma);

        // 4. �׹�° ���� ���ڿ��� ������ y�� 10�� ��ȯ �Ѵ�.
        int16_t y10 = atoi(right) ;
        right = strtok(NULL, comma);

        // 5. �ټ���° ���� ���ڿ��� ������ y�� 11�� ��ȯ �Ѵ�.
        int16_t y11 = atoi(right) ;
        right = strtok(NULL, comma);

        // 6. �ټ���° ���� ���ڿ��� ������ eeprom ���� ������ ���� ���� ��ȯ.
        int16_t testSave = atoi(right) ;

        // 7. ��ȯ�� gain_Bx, gain_By, gain_Bz, offset_Bx, offset_By, offset_Bz ���� eeprom�� �����Ѵ�.
        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_AX00_RA_ADDRESS, (uint16_t)(x00));
            (void)data_write_to_eeprom(EEPROM_AX01_RA_ADDRESS, (uint16_t)(x01));

            (void)data_write_to_eeprom(EEPROM_AY10_RA_ADDRESS, (uint16_t)(y10));
            (void)data_write_to_eeprom(EEPROM_AY11_RA_ADDRESS, (uint16_t)(y11));

            writeEepromCRC16();
        }

        // 8. ������ ���� �� �Ϸ� ack �� �͹̳ο� ���� �Ѵ�.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }
}

/*
��ɼ���
 �͹̳ο��� �װ��� ���� �ڱ��� �ɼ°��� �Է� �޾� eeprom�� ���� ��
 �͹̳ο� �Ϸ� ack ���� �Լ�

�������� ����
uint16_t rDataPointA[100];   �͹̳ηκ��� ��� ������ ����  ����
*/
static void calModeFluxSet(void)
{
    const char8_t* comma = ",";
    const char8_t end[] = {'\r', '\n'};

    char8_t buffer[100] = {0,};

    int16_t i;
    for(i=0; i<100; i++)
    {
        buffer[i] = (char8_t)rDataPointA[i];
    }

    // 1. ���ŵ� ���ڿ��� ���� comma ���� ���ڿ��� �и�
    const char8_t *tempcal = strtok(&rDataPointA[6],comma);
    if( tempcal != NULL)
    {
        // 2. ù��° ���� ���ڿ��� x�� ���ӵ� �̵� ���ڿ��� gain_Ax ������ ��ȯ �Ѵ�.
        int16_t offset_CalBx = atoi(tempcal) ;      // pc���� 1000000 ���� ���� �޴´�.
        tempcal = strtok(NULL, comma);

        // 3. �ι�° ���� ���ڿ��� y�� ���ӵ� �̵� ���ڿ��� gain_Ay ������ ��ȯ �Ѵ�.
        int16_t offset_CalBy = atoi(tempcal);
        tempcal = strtok(NULL, comma);

        // 4. ����° ���� ���ڿ��� x�� ���ӵ� �ɼ� ���ڿ��� offset_Ax ������ ��ȯ �Ѵ�.
        int16_t offset_CalBz = atoi(tempcal) ;
        tempcal = strtok(NULL, comma);

        // 5. �ټ���° �ټ���° ���� ���ڿ��� y�� ���ӵ� �ɼ� eeprom ���� ������ ���� ���� ��ȯ.
        int16_t testSave = atoi(tempcal) ;

        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_BX_CAL_OFFSET_ADDRESS, (uint16_t)(offset_CalBx));
            (void)data_write_to_eeprom(EEPROM_BY_CAL_OFFSET_ADDRESS, (uint16_t)(offset_CalBy));
            (void)data_write_to_eeprom(EEPROM_BZ_CAL_OFFSET_ADDRESS, (uint16_t)(offset_CalBz));

            writeEepromCRC16();
        }

        // 6. ������ ���� �� �Ϸ� ack �� �͹̳ο� ���� �Ѵ�.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }
}

/*
��ɼ���
 �͹̳ο��� ����Ʈ���� ������ �Է� �޾� eeprom�� ���� ��  �͹̳ο� �Ϸ� ack ���� �Լ�

�������� ����
uint16_t rDataPointA[100];   �͹̳ηκ��� ��� ������ ����  ����
*/
static void chk_ver_Set(void)
{
    const char8_t* comma = ",";
    const char8_t end[] = {'\r', '\n'};

    char8_t buffer[100] = {0,};

    int16_t i;
    for(i=0; i<100; i++)
    {
        buffer[i] = (char8_t)rDataPointA[i];
    }

    // 1. ���ŵ� ���ڿ��� ���� comma ���� ���ڿ��� �и�
    const char8_t *chk = strtok(&rDataPointA[5],comma);
    if( chk != NULL)
    {
        int16_t version = atoi(chk);
        chk = strtok(NULL, comma);

        // 1.1 �ټ���° �ټ���° ���� ���ڿ��� y�� ���ӵ� �ɼ� eeprom ���� ������ ���� ���� ��ȯ.
        int16_t testSave = atoi(chk) ;

        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_VERSION_ADDRESS, (uint16_t)(version));

            writeEepromCRC16();
        }

        // 1.2 ������ ���� �� �Ϸ� ack �� �͹̳ο� ���� �Ѵ�.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }

}


/*
��ɼ���
 eeprom�� ����� �����͸� �а� crc �����͸� �ش� EEPROM_CHK_CRC_ADDRESS �ּҿ� CRC16 ���� �����Ѵ�.

�������� ����
uint16_t eepromcrc; eeprom CRC ����
*/
void writeEepromCRC16(void)
{
    read_parameter();
    (void)data_write_to_eeprom(EEPROM_CHK_CRC_ADDRESS, (uint16_t)eepromcrc);
    read_parameter();

}


#if 0
void sendUart(float64_t bx, float64_t by, float64_t bz, float64_t ax, float64_t ay, float64_t brx, float64_t bry, uint16_t angle)
{
    char8_t *msg2 = NULL;


    sprintf(msg2,"%d, %d, %d, ", (int)bx, (int)by, (int)bz);
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg2, strlen(msg2));

    sprintf(msg2,"%d, %d, ", (int)brx, (int)bry);
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg2, strlen(msg2));


    sprintf(msg2,"%d, %d, ", (int16_t)(ax*10000), (int16_t)(ay*10000));
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg2, strlen(msg2));

    sprintf(msg2,"%d\r\n", angle);
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg2, strlen(msg2));

}


void calsendUart(uint16_t cnt, uint16_t angle)
{
    char8_t *msg2 = NULL;

    sprintf(msg2,"rot:%d, calBx:%d \r\n", cnt, angle);
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg2, strlen(msg2));

}

#endif

