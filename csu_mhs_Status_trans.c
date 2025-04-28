/*
 * csu_mhs_Status_trans.c
 *
 * 기능 : SFI로 MHS는 방위각, 자기장, 경사각 정보를 송신한다.
 * 구성요소 : MHS 상태 정보 송신 (D-MHS-SFR-014)
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */

#include "mhs_project.h"

#define DATA_6_LEN                  (6U)

#if 0
uint32_t RxBusWord[32];
#endif
uint32_t TxBusWord[32];		// HI3587에 데이터 전송을 위한 spi TX 버퍼 

struct _arinc429_error arinc429_error;

// 기능 : SFI에 Arinc429를 이용하여 방위각, 자기장 값을 전송하기 위한 함수.
// 입력 전역 변수
// 		_StatusMatrix : C_bit 수행중 에러 발생 상태. 1: Error State, 0: No Error
// 		mhsensor_data : 방위각, x,y,z축 자기장, x,y축 가속도와 각 축의 부호 저장을 위한 구조체 
// 출력 전역 변수 
//		TxBusWord[] : HI3587에 데이터 전송을 위한 spi TX 버퍼 
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
void mhs_status_trans(uint16_t paddingbit, uint16_t StatusMatrix)
{
    uint16_t rxbyte;
    struct _arinc429_word arinc429_word;

	// 1. hi3587의 상태 읽기를 통해 새로운 데이터 전송 할 준비기 되어 있는지 확인 , hi3587 tx fifo 가 비어 질때(readStatusReg()함수를 통하여 확인)까지 100us 주기로 검사하며 대기  
	//     리턴값 : 0x0008U(transmit fifo is empty.) 0x0000U(transmit fifo not empty)
    while (1)
    {    	
		rxbyte = readStatusReg() & 0x0008U;
		if(rxbyte == 0x0008U)
		{
			break;
		}
		else
		{
			rxbyte = 0;
		}
		
		delay_uS(100.0L);
    }

	// 2. mhsensor_data.MagHeading 값이 3599 값이 넘지 않도록 제한 (방어적 코드)
	if(mhsensor_data.MagHeading >3599U)
	{
		mhsensor_data.MagHeading = 3599U;
	}

	// 3. arinc429에 전송을 위한 방위각 값을 arinc429 프로토콜 형태로 변경 
	// 	  parity(1bit): 프로토콜의 parity 에서 0(deflult : Odd Parity)
	//    ssm   (2bit): status Matrix 입력 (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
	//    dig_4 (3bit): mhsensor_data.MagHeading의 1000의 자리값  
	//    dig_3 (4bit): mhsensor_data.MagHeading의 100의 자리값  
	//    dig_2 (4bit): mhsensor_data.MagHeading의 10의 자리값  
	//    dig_1 (4bit): mhsensor_data.MagHeading의 1의 자리값  
	//    pad   (2bit): padding (0000 : normal, 0001: calibration mode)
	//    label (8bit): Label octal (014)
    arinc429_word.parity = 0;
    arinc429_word.ssm = StatusMatrix;
    arinc429_word.dig_4 = mhsensor_data.MagHeading / 1000U;                // 100
    arinc429_word.dig_3 = (mhsensor_data.MagHeading % 1000U) / 100U;        // 20
    arinc429_word.dig_2 = (mhsensor_data.MagHeading % 100U) / 10U;          // 3
    arinc429_word.dig_1 = (mhsensor_data.MagHeading % 10U);                // 0.4
    arinc429_word.pad = paddingbit;
    arinc429_word.label = MH_LABEL_ADDR;    // 0ctal 014

	// 4. 방위각 데이터를 SFI_MHS01 포멧에 맞도록 전송할 hi3587 tx 버퍼(TxBusWord[0])에 저장
    TxBusWord[0] = 0;
    TxBusWord[0] |= (uint32_t)arinc429_word.parity << PARITY;
    TxBusWord[0] |= (uint32_t)arinc429_word.ssm << SSM;
    TxBusWord[0] |= (uint32_t)arinc429_word.dig_4 << DIG4;
    TxBusWord[0] |= (uint32_t)arinc429_word.dig_3 << DIG3;
    TxBusWord[0] |= (uint32_t)arinc429_word.dig_2 << DIG2;
    TxBusWord[0] |= (uint32_t)arinc429_word.dig_1 << DIG1;
    TxBusWord[0] |= (uint32_t)arinc429_word.pad << PAD;
    TxBusWord[0] |= (uint32_t)arinc429_word.label << LABEL;


	// 5. 자기장 x축 데이터를 SFI_MHS02 포멧에 맞도록 전송할 hi3587 tx 버퍼(TxBusWord[1])에 저장
		//	  parity(1bit): 프로토콜의 parity 에서 0(deflult : Odd Parity)
	//    ssm   (2bit): status Matrix 입력 (11 : normal, 01: No computed Data, 10: function Test, 00: Failure Warming )
	//    NULL  (3bit) : forced to 0
	//    data  (15bit) : Range/2^(15bit) , Range : ±100 μT
	// 	  signal(1bit)  : Sign Bit(0=Plus, 1=Minus, 2's complement)
	//    pad   (2bit): padding (0000 : normal, 0001: calibration mode)
	//    label (8bit): Label octal (300)
    if(StatusMatrix == STATUS_BCD_NORMAL)
    {
        arinc429_word.ssm = STATUS_BNR_NORMAL;
        arinc429_error.ssm = STATUS_BNR_NORMAL;
    }
    else if(StatusMatrix == STATUS_BCD_FW)
    {
        arinc429_word.ssm = STATUS_BNR_FW;
        arinc429_error.ssm = STATUS_BNR_FW;
    }
    else
    {
        arinc429_word.ssm = STATUS_NO_COMPUT_DATA;
        arinc429_error.ssm = STATUS_NO_COMPUT_DATA;
    }

    arinc429_word.parity = 0;
    arinc429_word.label = MAG_X_LABEL_ADDR;    // 0ctal 300
    TxBusWord[1] = 0;
    TxBusWord[1] |= (uint32_t)arinc429_word.parity << PARITY;
    TxBusWord[1] |= (uint32_t)arinc429_word.ssm << SSM;
    uint16_t magX = (uint16_t)mhsensor_data.Mag_x;
    TxBusWord[1] |= (uint32_t)magX << DATA;
    TxBusWord[1] |= (uint32_t)arinc429_word.pad << PAD;
    TxBusWord[1] |= (uint32_t)arinc429_word.label << LABEL;

	// 6. 자기장 y축 데이터를 SFI_MHS03 포멧에 맞도록 전송할 hi3587 tx 버퍼(TxBusWord[2])에 저장
	//	  parity(1bit): 프로토콜의 parity 에서 0(deflult : Odd Parity)
	//    ssm   (2bit): status Matrix 입력 (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
	//    NULL  (3bit) : forced to 0
	//    data  (15bit) : Range/2^(15bit) , Range : ±100 μT
	// 	  signal(1bit)  : Sign Bit(0=Plus, 1=Minus, 2's complement)
	//    pad   (2bit): padding (0000 : normal, 0001: calibration mode)
	//    label (8bit): Label octal (301)
    arinc429_word.parity = 0;
    arinc429_word.label = MAG_Y_LABEL_ADDR;    // 0ctal 301
    TxBusWord[2] = 0;
    TxBusWord[2] |= (uint32_t)arinc429_word.parity << PARITY;
    TxBusWord[2] |= (uint32_t)arinc429_word.ssm << SSM;
    uint16_t magY = (uint16_t)mhsensor_data.Mag_y;
    TxBusWord[2] |= (uint32_t)magY << DATA;
    TxBusWord[2] |= (uint32_t)arinc429_word.pad << PAD;
    TxBusWord[2] |= (uint32_t)arinc429_word.label << LABEL;


	// 7. 자기장 z축 데이터를 SFI_MHS04 포멧에 맞도록 전송할 hi3587 tx 버퍼(TxBusWord[3])에 저장
	//	  parity(1bit): 프로토콜의 parity 에서 0(deflult : Odd Parity)
	//    ssm   (2bit): status Matrix 입력 (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
	//    NULL  (3bit) : forced to 0
	//    data  (15bit) : Range/2^(15bit) , Range : ±100 μT
	// 	  signal(1bit)  : Sign Bit(0=Plus, 1=Minus, 2's complement)
	//    pad   (2bit): padding (0000 : normal, 0001: calibration mode)
	//    label (8bit): Label octal (302)
    arinc429_word.parity = 0;
    arinc429_word.label = MAG_Z_LABEL_ADDR;    // 0ctal 302
    TxBusWord[3] = 0;
    TxBusWord[3] |= (uint32_t)arinc429_word.parity << PARITY;
    TxBusWord[3] |= (uint32_t)arinc429_word.ssm << SSM;
    uint16_t magZ = (uint16_t)mhsensor_data.Mag_z;
    TxBusWord[3] |= (uint32_t)magZ << DATA;
    TxBusWord[3] |= (uint32_t)arinc429_word.pad << PAD;
    TxBusWord[3] |= (uint32_t)arinc429_word.label << LABEL;


    // 8. 코드 check sum 및 온도, 전압, eeprom 에러 상태를 SFI_MHS04 포멧에 맞도록 전송할 hi3587 tx 버퍼(TxBusWord[4])에 저장
    //    parity(1bit): 프로토콜의 parity 에서 0(deflult : Odd Parity)
    //    ssm   (2bit): status Matrix 입력 (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
    //    check Sum  (16bit) : compile out file checkSum
    //    temp error (1bit)  : temp error Bit(0= normal, 1= error)
    //    volt error (1bit)  : volt error Bit(0= normal, 1= error)
    //    eeprom error (1bit)  : eeprom error Bit(0= normal, 1= error)
    //    sdi   (2bit): not used, 00
    //    label (8bit): Label octal (331)

    uint32_t ssm = 0U;

    arinc429_error.parity = 0;
    arinc429_error.label = CHK_EER_LABEL_ADDR;    // 0ctal 331
    arinc429_error.checksum = gCheckSum_code;
    TxBusWord[4] = 0;
    TxBusWord[4] |= (uint32_t)arinc429_error.parity << PARITY;
    TxBusWord[4] |= ssm << SSM;
    uint16_t checksum = (uint16_t)arinc429_error.checksum;
    TxBusWord[4] |= (uint32_t)checksum << DATA;
    TxBusWord[4] |= (uint32_t)arinc429_error.tempError << TEMP_ERROR;
    TxBusWord[4] |= (uint32_t)arinc429_error.voltageError << VOLT_ERROR;
    TxBusWord[4] |= (uint32_t)arinc429_error.eepromError << EEP_ERROR;
    TxBusWord[4] |= (uint32_t)arinc429_error.label << LABEL;

    // 9. 소프트웨어 버전을 SFI_MHS04 포멧에 맞도록 전송할 hi3587 tx 버퍼(TxBusWord[4])에 저장
    //    parity(1bit): 프로토콜의 parity 에서 0(deflult : Odd Parity)
    //    ssm   (2bit): status Matrix 입력 (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
    //    version (16bit) : 4bit 4bit, 4bit, 4bit
    //    sdi   (2bit): not used, 00
    //    label (8bit): Label octal (332)
    arinc429_error.parity = 0;
    arinc429_error.label = VERSION_LABEL_ADDR;    // 0ctal 332
    TxBusWord[5] = 0;
    TxBusWord[5] |= (uint32_t)arinc429_error.parity << PARITY;
    TxBusWord[5] |= ssm << SSM;
//    uint16_t version = gversion_code;  // (16bit)
    uint16_t version = 0xA123;  // (16bit)
    TxBusWord[5] |= (uint32_t)version << VER;
    TxBusWord[5] |= (uint32_t)arinc429_error.label << LABEL;


	// 8. writeTxFIFO()를 이용하여 arinc429 프로토콜 TxBusWord[0..3]를 전송
    writeTxFIFO(DATA_6_LEN);

	// 9. readStatusReg()를 이용하여 전송 상태를 확인. (여기서는 완료까지 대기 하지 않는다. 대기는 다음번 본 함수가 실행시 추행함.)
    (void)readStatusReg();

	// 10. HI3579 를 쓰기모드로 변경하기 위하여 1 byte 의 dummy 데이터를 전송한다.
    txOpCode (0x12);


//    498             00000001001001100000000000001100b
//    7015    0x1B67  00000011011011001110000011000000b
//    8336    0x2090  00000100000100100000000011000001b
//    -7697   0xE1EF  00011100001111011110000011000010b


}

