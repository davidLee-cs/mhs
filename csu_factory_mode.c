/*
 * factory_mode.c
 *
 * 기능 : factory mode 전환 후 보정을 위한 자기장, 가속도 데이터 출력, 보정값 저장, eeprom 값 출력하는 기능
 * 구성요소 : Factory mode CSU (D-MHS-SFR-010)
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 *    2024.12.13
 *     - eeprom 입출력 부분 수정
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
 기능 설명
 factory 모드에서는 센서에서 데이터를 수집하여 보정한 데이터를 eeprom 에 저장한다. (bit 수행은 하지 않음)

 전역 변수 설명
 Rx_done:     명령 수신 완료 플레그 , 1 : 수신완료, 0 : 미수신 또는 진행중
 rDataPointA[100]:   터미널로부터 명령 수신을 위한  버퍼

 로컬변수
 bSendUartData : 통신 전송 유무 설정 (0: 미송신, 1: 송신)
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

    // 1. 처음 팩토리 모드로 동작 시 팩토리 모드림을 알려주는 문장을 통신 터미널에 아스키 코드 전송
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


	// 2. 터미널로부터 수신 완료된 명령이 있으면
    if(gRx_done == 1U)
    {
    	// 2.1 수신데이터가 start 명령이면, 측정된 센서 데이터를 터미널로 전송하는 함수가 작동하도록 bSendUartData 플레그를 true 로 변경한다.
    	//     start 가 되면 자기장, 가속도 게인 및 옵셋 값을 초기화 한 시킴.
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

			// 2.1.1 eeprom 에서 있는 자기장, 가속도 게인 및 옵셋 값 등을 읽음.
            read_parameter();

            bSendUartData = 1U;
        }

		// 2.2 수신데이터가 stop 명령이면, 측정된 센서 데이터를 터미널로 전송하는 함수가 작동하지 않도록도록 bSendUartData 플레그를 false 로 변경한다.
        if(strncmp(rDataPointA, stopCmd, 5) == 0)
        {
            bSendUartData = 0U;
        }

        // 2.3 추가 명령어 수신확인 함수
        eepromDataReadCmd();
        sub_function_mode();


		// 2.4 수신 버퍼 clear
        for(cnt=0; cnt<100; cnt++)
        {
            rDataPointA[cnt] = 0;
        }

		// 2.5 수신데이터 천리 완료 표시
        gRx_done = 0U;
        gRx_cnt = 0;

    }

	// 3. start 상태 (bSendUartData True) 이면 자기장과 가속도를 측정하여 status_transfer_to_terminal() 를 호출하여 측정 데이터를 터미널로 전송 한다.
    if(bSendUartData == 1U)
    {
        MeasureFlux();
        MeasureAccelation();

        status_transfer_to_terminal();
    }

}

/*
 기능 설명
 factory 모드에서는 eeprom 데이터 읽기 명령어 입력 시 eeprom 내부 데이터 읽기

 전역 변수 설명
 rDataPointA[100]:   터미널로부터 명령 수신을 위한  버퍼

*/
static void eepromDataReadCmd(void){

    const char8_t eepromReadCmd[] = {'$','R','E','A','D','\0'};
    // 1.3 수신데이터가 eeprom 에 저장된 calibarion  전송 명령이면, calibrationDataTransfortToTerminal() 를 수행하여 eeprom 에 저장된 데이터를 터미너로 전송한다.
    if(strncmp(rDataPointA, eepromReadCmd, 5) == 0)
    {
        calibrationDataTransfortToTerminal();
        sub_calibrationDataTransfortToTerminal();
        matrixDataTransfortToTerminal();
        sub_matrixDataTransfortToTerminal();
    }
}


/*
 기능 설명
 입력된 명령어 확인하는 함수

 전역 변수 설명
 rDataPointA[100]:   터미널로부터 명령 수신을 위한  버퍼

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

    // 1. 수신데이터가 보정한 가속도 센서 파라미터 값 저장 명령이면, 보정한 가속도 센서 파라미터 값인  x,y,z축 gain, offset 값을 eeprom에 저장한다.
    if(strncmp(rDataPointA, calAccelCmd, 6) == 0)
    {
        calAcceSet();
    }

    // 2. 수신데이터가 보정한 자기센 센서 파리메터 저장 명령이면, 보정한 자기장 센서 파라미터 값인 x,y,z축 gain, offset 값을 eeprom에 저장한다.
    if(strncmp(rDataPointA, calFluxCmd, 5) == 0)
    {
        calFluxSet();
    }

    // 3. 수신데이터가 항공기 자기장 보정 옵셋 데이터 저장 명령이면, 보정한 보정 자기장 센서 파라미터 값인 x,y,z축 값을 eeprom에 저장한다.
    if(strncmp(rDataPointA, calModeOffsetCmd, 5) == 0)
    {
        calModeFluxSet();
    }

    // 4. 수신데이터가 보정한 가속도 축보정 명령이면, 행력 2x2 값을 eeprom에 저장한다.
    if(strncmp(rDataPointA, fluxrightAnglesetCmd, 4) == 0)
    {
        flux_rightAngleSet();
    }

    // 5. 수신데이터가 보정한 자기장 축보정 명령이면, 행력 3x3 값을 eeprom에 저장한다.
    if(strncmp(rDataPointA, accelrightAnglesetCmd, 4) == 0)
    {
        accel_rightAngleSet();
    }

    // 6. 수신데이터가 보정한 소프트웨어 버전 명령이면, 소프트웨어 버전값을  eeprom에 저장한다.
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
기능설명
eeprom에 저장되어 있는 자기장, 기속도 보정 데이터를 터미널에 전송하는 함수

전역 변수 설명
RX_MsgBuffer[MAX_BUFFER_SIZE];  EEPROM 에서 읽은 데이터를 ISR과 다른 csu와 공유하는 버퍼
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

	// 1. eeprom 내의 모든 데이터마다 eeprom 에서 1word 씩 읽어 terminal 로 전송한다.
    for(epaddr=EEPROM_GAIN_AX_ADDRESS; epaddr<EEPROM_OFFSET_BZ_ADDRESS+1U; epaddr+=3U)
    {
    	// 1.1 eeprom 으로 부터 데이터 읽기 공하는 경우
        if(data_Read((uint32_t)epaddr) == SUCCESS)
        {
        	// 1.1.1 RX_MsgBuffer[] 버퍼에 들어있는 1word 로 변환하여    read   변수에 저장한다.
            uint16_t rxh = RX_MsgBuffer[1] << 8;
            uint16_t rxl = RX_MsgBuffer[0] & 0x00FFU;
            uint16_t rxhl = rxh | rxl;
            epread = (int16_t)(rxhl);

            readCalData[readsize] = epread;
            readCalAddr[readsize] = epaddr;
            readsize++;
        }
    }

    // 2. eeprom에서 읽은 데이터의 어드레스를 비교하여 각 해당 인덱스 문자열 출력한다.
    for(index=0; index < 12U; index++)
    {
        //2.1 각 주소를 비교하여 문자열 출력
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

        // 2.2. read 에서 부호와 절대값을 추출, 절대값은 myRead 변수에 저장
        if(readCalData[index] < 0){
            myread = readCalData[index] * -1;
            // 2.2.1 부호를 터미널로 전송
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)minus, 1U);
        }else{
            myread = readCalData[index];
            // 2.2.2 부호를 터미널로 전송
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)plus, 1U);
        }

        // 2.3.  myRead 절대값을 터미널로 전송한다.
        int32_t mystr = (int32_t)myread;
        (void)LToStr(mystr,msg,6);
        StrZeroFill(msg,6);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)msg, 6U);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 4U);

    }
}

/*
기능설명
eeprom에 저장되어 있는 항공기 보정 옵셋 자기장 데이터를 터미널에 전송하는 함수

전역 변수 설명
RX_MsgBuffer[MAX_BUFFER_SIZE];  EEPROM 에서 읽은 데이터를 ISR과 다른 csu와 공유하는 버퍼
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

    // 1. eeprom 내의 모든 데이터마다 eeprom 에서 1word 씩 읽어 terminal 로 전송한다.
    for(epaddr=EEPROM_BX_CAL_OFFSET_ADDRESS; epaddr<EEPROM_BZ_CAL_OFFSET_ADDRESS+1U; epaddr+=3U)
    {
        // 1.1 eeprom 으로 부터 데이터 읽기 공하는 경우
        if(data_Read((uint32_t)epaddr) == SUCCESS)
        {
            // 1.1.1 RX_MsgBuffer[] 버퍼에 들어있는 1word 로 변환하여    read   변수에 저장한다.
            uint16_t rxh = RX_MsgBuffer[1] << 8;
            uint16_t rxl = RX_MsgBuffer[0] & 0x00FFU;
            uint16_t rxhl = rxh | rxl;
            epread = (int16_t)(rxhl);

            readCalData[readsize] = epread;
            readCalAddr[readsize] = epaddr;
            readsize++;
        }
    }

    // 2. eeprom에서 읽은 데이터의 어드레스를 비교하여 각 해당 인덱스 문자열 출력한다.
    for(index=0; index < 3U; index++)
    {
        //2.1 각 주소를 비교하여 문자열 출력
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

        // 2.2. read 에서 부호와 절대값을 추출, 절대값은 myRead 변수에 저장
        if(readCalData[index] < 0){
            myread = readCalData[index] * -1;
            // 2.2.1 부호를 터미널로 전송
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)minus, 1U);
        }else{
            myread = readCalData[index];
            // 2.2.2 부호를 터미널로 전송
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)plus, 1U);
        }

        // 2.3.  myRead 절대값을 터미널로 전송한다.
        int32_t mystr = (int32_t)myread;
        (void)LToStr(mystr,msg,6);
        StrZeroFill(msg,6);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)msg, 6U);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 4U);

    }
}


/*
기능설명
eeprom에 저장되어 있는 각 축보정 데이터를 터미널에 전송하는 함수

전역 변수 설명
RX_MsgBuffer[MAX_BUFFER_SIZE];  EEPROM 에서 읽은 데이터를 ISR과 다른 csu와 공유하는 버퍼
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

    // 1. eeprom 내의 모든 데이터마다 eeprom 에서 1word 씩 읽어 terminal 로 전송한다.
    for(epaddr=EEPROM_BX00_RA_ADDRESS; epaddr<EEPROM_AY11_RA_ADDRESS+1U; epaddr+=3U)
    {
        // 1.1 eeprom 으로 부터 데이터 읽기 공하는 경우
        if(data_Read((uint32_t)epaddr) == SUCCESS)
        {
            // 1.1.1 RX_MsgBuffer[] 버퍼에 들어있는 1word 로 변환하여    read   변수에 저장한다.
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
        //2.1 각 주소를 비교하여 문자열 출력
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

        // 2.2. read 에서 부호와 절대값을 추출, 절대값은 myRead 변수에 저장
        if(readCalData[index] < 0){
            myread = readCalData[index] * -1;
            // 2.2.1 부호를 터미널로 전송
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)minus, 1U);
        }else{
            myread = readCalData[index];
            // 2.2.2 부호를 터미널로 전송
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)plus, 1U);
        }

        // 2.3.  myRead 절대값을 터미널로 전송한다.
        int32_t mystr = (int32_t)myread;
        (void)LToStr(mystr,msg,6);
        StrZeroFill(msg,6);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)msg, 6U);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 4U);

    }
}

/*
기능설명
eeprom에 저장되어 있는 소프트웨어 버전 및 CRC16 데이터를 터미널에 전송하는 함수

전역 변수 설명
RX_MsgBuffer[MAX_BUFFER_SIZE];  EEPROM 에서 읽은 데이터를 ISR과 다른 csu와 공유하는 버퍼
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

    // 1. eeprom 내의 모든 데이터마다 eeprom 에서 1word 씩 읽어 terminal 로 전송한다.
    for(epaddr=EEPROM_CHK_SUM_ADDRESS; epaddr<EEPROM_CHK_CRC_ADDRESS+1U; epaddr+=3U)
    {
        // 1.1 eeprom 으로 부터 데이터 읽기 공하는 경우
        if(data_Read((uint32_t)epaddr) == SUCCESS)
        {
            // 1.1.1 RX_MsgBuffer[] 버퍼에 들어있는 1word 로 변환하여    read   변수에 저장한다.
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
        //2.1 각 주소를 비교하여 문자열 출력
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

        // 2.2. read 에서 부호와 절대값을 추출, 절대값은 myRead 변수에 저장
        if(readCalData[index] < 0){
            myread = readCalData[index] * -1;
            // 2.2.1 부호를 터미널로 전송
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)minus, 1U);
        }else{
            myread = readCalData[index];
            // 2.2.2 부호를 터미널로 전송
            SCI_writeCharArray(SCIA_BASE, (const char8_t*)plus, 1U);
        }

        // 2.3.  myRead 절대값을 터미널로 전송한다.
        int32_t mystr = (int32_t)myread;
        (void)LToStr(mystr,msg,6);
        StrZeroFill(msg,6);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)msg, 6U);
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 4U);

    }
}


/*
기능설명
터미널에서 가속도 보정값을 입력 받아 각 값을 파싱 후 gain, offset 로 저장하여 eeprom에 저장 후 터미널에 완료 ack 전송 함수

전역변수 설명
uint16_t rDataPointA[100];  터미널로부터 명령 수신을 위한 버퍼
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

	// 1. 수신된 문자열로 부터 comma 단위 분자열로 분리
    const char8_t* tempacc = strtok(&rDataPointA[7],comma);

    if( tempacc != NULL)
    {

        // 2. 첫번째 단위 문자열은 x축 가속도 이득 문자열을 gain_Ax 정수로 변환 한다.
        int16_t gain_Ax = atoi(tempacc) ;      // pc에서 1000000 곱한 값을 받는다.
        tempacc = strtok(NULL, comma);

        // 3. 두번째 단위 문자열은 y축 가속도 이득 문자열을 gain_Ay 정수로 변환 한다.
        int16_t gain_Ay = atoi(tempacc);
        tempacc = strtok(NULL, comma);

        // 4. 세번째 단위 문자열은 x축 가속도 옵셋 문자열을 offset_Ax 정수로 변환 한다.
        int16_t offset_Ax = atoi(tempacc);
        tempacc = strtok(NULL, comma);

        // 5. 네번째 단위 문자열은 y축 가속도 옵셋 문자열을 offset_Ay 정수로 변환 한다.
        int16_t offset_Ay = atoi(tempacc);

        tempacc = strtok(NULL, comma);

        // 6. 다섯전째 단위 문자열은 y축 가속도 옵셋 eeprom 저장 유무를 위한 정수 변환.
        int16_t testSave = atoi(tempacc) ;

        // 7. 변환된 gain_Ax, gain_Ay, offset_Ax, offset_Ay 값을 eeprom에 저장한다.
        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_GAIN_AX_ADDRESS, (uint16_t)(gain_Ax));
            (void)data_write_to_eeprom(EEPROM_GAIN_AY_ADDRESS, (uint16_t)(gain_Ay));
            (void)data_write_to_eeprom(EEPROM_OFFSET_AX_ADDRESS, (uint16_t)(offset_Ax));
            (void)data_write_to_eeprom(EEPROM_OFFSET_AY_ADDRESS, (uint16_t)(offset_Ay));

            writeEepromCRC16();
        }

        // 8. 데이터 저장 후 완료 ack 를 터미널에 전송 한다.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);

    }
    else
    {

    }
}

/*
기능설명
 터미널에서 가속도 보정값을 입력 받아 각 값을 파싱 후 gain, offset 로 저장하여 eeprom에 저장 후
 터미널에 완료 ack 전송 함수

전역변수 설명
uint16_t rDataPointA[100];   터미널로부터 명령 수신을 위한  버퍼
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

        // 1. 수신된 문자열로 부터 comma 단위 분자열로 분리
    const char8_t* tempflux = strtok(&rDataPointA[6],comma);

    if( tempflux != NULL)
    {
        // 2. 첫번째 단위 문자열은 x축 자기장 이득 문자열을 gain_Bx 정수로 변환 한다.
        int16_t gain_Bx = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 3. 두번째 단위 문자열은 y축 자기장 이득 문자열을 gain_By 정수로 변환 한다.
        int16_t gain_By = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 4. 세번째 단위 문자열은 z축 자기장 이득 문자열을 gain_Bz 정수로 변환 한다.
        int16_t gain_Bz = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 5. 네번째 단위 문자열은 x축 가속도 옵셋 문자열을 offset_Bx 정수로 변환 한다.
        int16_t offset_Bx = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 6. 다섯번째 단위 문자열은 y축 가속도 옵셋 문자열을 offset_By 정수로 변환 한다.
        int16_t offset_By = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 7. 여섯번째 단위 문자열은 z축 가속도 옵셋 문자열을 offset_Bz 정수로 변환 한다.
        int16_t offset_Bz = atoi(tempflux) ;
        tempflux = strtok(NULL, comma);

        // 8. 다섯전째 단위 문자열은 y축 가속도 옵셋 eeprom 저장 유무를 위한 정수 변환.
        int16_t testSave = atoi(tempflux) ;

        // 9. 변환된 gain_Bx, gain_By, gain_Bz, offset_Bx, offset_By, offset_Bz 값을 eeprom에 저장한다.
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

        // 10. 데이터 저장 후 완료 ack 를 터미널에 전송 한다.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }
}

/*
기능설명
 터미널에서 자기장 축보정값을 입력 받아 각 값을 파싱 후 행력 3x3 값을 eeprom에 저장 후
 터미널에 완료 ack 전송 함수

전역변수 설명
uint16_t rDataPointA[100];   터미널로부터 명령 수신을 위한  버퍼
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

    // 1. 수신된 문자열로 부터 comma 단위 분자열로 분리
    const char8_t* right = strtok(&rDataPointA[5],comma);

    if( right != NULL)
    {
        // 2. 첫번째 단위 문자열은 직각도 x축 00를 변환 한다.
        int16_t x00 = atoi(right) ;
        right = strtok(NULL, comma);

        // 3. 두번째 단위 문자열은 직각도 x축 01를 변환 한다.
        int16_t x01 = atoi(right) ;
        right = strtok(NULL, comma);

        // 4. 세번째 단위 문자열은 직각도 x축 02를 변환 한다.
        int16_t x02 = atoi(right) ;
        right = strtok(NULL, comma);

        // 5. 네번째 단위 문자열은 직각도 y축 10를 변환 한다.
        int16_t y10 = atoi(right) ;
        right = strtok(NULL, comma);

        // 6. 다섯번째 단위 문자열은 직각도 y축 11를 변환 한다.
        int16_t y11 = atoi(right) ;
        right = strtok(NULL, comma);

        // 7. 여섯번째 단위 문자열은 직각도 z축 12를 변환 한다.
        int16_t y12 = atoi(right) ;
        right = strtok(NULL, comma);

        // 8. 네번째 단위 문자열은 직각도 z축 20를 변환 한다.
        int16_t z20 = atoi(right) ;
        right = strtok(NULL, comma);

        // 9. 다섯번째 단위 문자열은 직각도 z축 21를 변환 한다.
        int16_t z21 = atoi(right) ;
        right = strtok(NULL, comma);

        // 10. 여섯번째 단위 문자열은 직각도 z축 22를 변환 한다.
        int16_t z22 = atoi(right) ;
        right = strtok(NULL, comma);

        // 8. 다섯전째 단위 문자열은 직각도 eeprom 저장 유무를 위한 정수 변환.
        int16_t testSave = atoi(right) ;

        // 9. 변환된 gain_Bx, gain_By, gain_Bz, offset_Bx, offset_By, offset_Bz 값을 eeprom에 저장한다.
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

        // 10. 데이터 저장 후 완료 ack 를 터미널에 전송 한다.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }

}

/*
기능설명
 터미널에서 가속도 축보정값을 입력 받아 각 값을 파싱 후 행력 2x2 값을 eeprom에 저장 후
 터미널에 완료 ack 전송 함수

전역변수 설명
uint16_t rDataPointA[100];   터미널로부터 명령 수신을 위한  버퍼
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

    // 1. 수신된 문자열로 부터 comma 단위 분자열로 분리
    const char8_t* right = strtok(&rDataPointA[5],comma);

    if( right != NULL)
    {
        // 2. 첫번째 단위 문자열은 직각도 x축 00를 변환 한다.
        int16_t x00 = atoi(right) ;
        right = strtok(NULL, comma);

        // 3. 두번째 단위 문자열은 직각도 x축 01를 변환 한다.
        int16_t x01 = atoi(right) ;
        right = strtok(NULL, comma);

        // 4. 네번째 단위 문자열은 직각도 y축 10를 변환 한다.
        int16_t y10 = atoi(right) ;
        right = strtok(NULL, comma);

        // 5. 다섯번째 단위 문자열은 직각도 y축 11를 변환 한다.
        int16_t y11 = atoi(right) ;
        right = strtok(NULL, comma);

        // 6. 다섯전째 단위 문자열은 직각도 eeprom 저장 유무를 위한 정수 변환.
        int16_t testSave = atoi(right) ;

        // 7. 변환된 gain_Bx, gain_By, gain_Bz, offset_Bx, offset_By, offset_Bz 값을 eeprom에 저장한다.
        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_AX00_RA_ADDRESS, (uint16_t)(x00));
            (void)data_write_to_eeprom(EEPROM_AX01_RA_ADDRESS, (uint16_t)(x01));

            (void)data_write_to_eeprom(EEPROM_AY10_RA_ADDRESS, (uint16_t)(y10));
            (void)data_write_to_eeprom(EEPROM_AY11_RA_ADDRESS, (uint16_t)(y11));

            writeEepromCRC16();
        }

        // 8. 데이터 저장 후 완료 ack 를 터미널에 전송 한다.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }
}

/*
기능설명
 터미널에서 항공기 고유 자기장 옵셋값을 입력 받아 eeprom에 저장 후
 터미널에 완료 ack 전송 함수

전역변수 설명
uint16_t rDataPointA[100];   터미널로부터 명령 수신을 위한  버퍼
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

    // 1. 수신된 문자열로 부터 comma 단위 분자열로 분리
    const char8_t *tempcal = strtok(&rDataPointA[6],comma);
    if( tempcal != NULL)
    {
        // 2. 첫번째 단위 문자열은 x축 가속도 이득 문자열을 gain_Ax 정수로 변환 한다.
        int16_t offset_CalBx = atoi(tempcal) ;      // pc에서 1000000 곱한 값을 받는다.
        tempcal = strtok(NULL, comma);

        // 3. 두번째 단위 문자열은 y축 가속도 이득 문자열을 gain_Ay 정수로 변환 한다.
        int16_t offset_CalBy = atoi(tempcal);
        tempcal = strtok(NULL, comma);

        // 4. 세번째 단위 문자열은 x축 가속도 옵셋 문자열을 offset_Ax 정수로 변환 한다.
        int16_t offset_CalBz = atoi(tempcal) ;
        tempcal = strtok(NULL, comma);

        // 5. 다섯전째 다섯전째 단위 문자열은 y축 가속도 옵셋 eeprom 저장 유무를 위한 정수 변환.
        int16_t testSave = atoi(tempcal) ;

        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_BX_CAL_OFFSET_ADDRESS, (uint16_t)(offset_CalBx));
            (void)data_write_to_eeprom(EEPROM_BY_CAL_OFFSET_ADDRESS, (uint16_t)(offset_CalBy));
            (void)data_write_to_eeprom(EEPROM_BZ_CAL_OFFSET_ADDRESS, (uint16_t)(offset_CalBz));

            writeEepromCRC16();
        }

        // 6. 데이터 저장 후 완료 ack 를 터미널에 전송 한다.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }
}

/*
기능설명
 터미널에서 소프트웨어 버전을 입력 받아 eeprom에 저장 후  터미널에 완료 ack 전송 함수

전역변수 설명
uint16_t rDataPointA[100];   터미널로부터 명령 수신을 위한  버퍼
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

    // 1. 수신된 문자열로 부터 comma 단위 분자열로 분리
    const char8_t *chk = strtok(&rDataPointA[5],comma);
    if( chk != NULL)
    {
        int16_t version = atoi(chk);
        chk = strtok(NULL, comma);

        // 1.1 다섯전째 다섯전째 단위 문자열은 y축 가속도 옵셋 eeprom 저장 유무를 위한 정수 변환.
        int16_t testSave = atoi(chk) ;

        if(testSave == 1)
        {
            (void)data_write_to_eeprom(EEPROM_VERSION_ADDRESS, (uint16_t)(version));

            writeEepromCRC16();
        }

        // 1.2 데이터 저장 후 완료 ack 를 터미널에 전송 한다.
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)buffer, (uint16_t)strlen(buffer));
        SCI_writeCharArray(SCIA_BASE, (const char8_t*)end, 2U);
    }

}


/*
기능설명
 eeprom에 저장된 데이터를 읽고 crc 데이터를 해당 EEPROM_CHK_CRC_ADDRESS 주소에 CRC16 값을 저장한다.

전역변수 설명
uint16_t eepromcrc; eeprom CRC 저장
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

