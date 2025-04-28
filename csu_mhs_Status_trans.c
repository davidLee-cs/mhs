/*
 * csu_mhs_Status_trans.c
 *
 * ��� : SFI�� MHS�� ������, �ڱ���, ��簢 ������ �۽��Ѵ�.
 * ������� : MHS ���� ���� �۽� (D-MHS-SFR-014)
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */

#include "mhs_project.h"

#define DATA_6_LEN                  (6U)

#if 0
uint32_t RxBusWord[32];
#endif
uint32_t TxBusWord[32];		// HI3587�� ������ ������ ���� spi TX ���� 

struct _arinc429_error arinc429_error;

// ��� : SFI�� Arinc429�� �̿��Ͽ� ������, �ڱ��� ���� �����ϱ� ���� �Լ�.
// �Է� ���� ����
// 		_StatusMatrix : C_bit ������ ���� �߻� ����. 1: Error State, 0: No Error
// 		mhsensor_data : ������, x,y,z�� �ڱ���, x,y�� ���ӵ��� �� ���� ��ȣ ������ ���� ����ü 
// ��� ���� ���� 
//		TxBusWord[] : HI3587�� ������ ������ ���� spi TX ���� 
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
void mhs_status_trans(uint16_t paddingbit, uint16_t StatusMatrix)
{
    uint16_t rxbyte;
    struct _arinc429_word arinc429_word;

	// 1. hi3587�� ���� �б⸦ ���� ���ο� ������ ���� �� �غ�� �Ǿ� �ִ��� Ȯ�� , hi3587 tx fifo �� ��� ����(readStatusReg()�Լ��� ���Ͽ� Ȯ��)���� 100us �ֱ�� �˻��ϸ� ���  
	//     ���ϰ� : 0x0008U(transmit fifo is empty.) 0x0000U(transmit fifo not empty)
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

	// 2. mhsensor_data.MagHeading ���� 3599 ���� ���� �ʵ��� ���� (����� �ڵ�)
	if(mhsensor_data.MagHeading >3599U)
	{
		mhsensor_data.MagHeading = 3599U;
	}

	// 3. arinc429�� ������ ���� ������ ���� arinc429 �������� ���·� ���� 
	// 	  parity(1bit): ���������� parity ���� 0(deflult : Odd Parity)
	//    ssm   (2bit): status Matrix �Է� (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
	//    dig_4 (3bit): mhsensor_data.MagHeading�� 1000�� �ڸ���  
	//    dig_3 (4bit): mhsensor_data.MagHeading�� 100�� �ڸ���  
	//    dig_2 (4bit): mhsensor_data.MagHeading�� 10�� �ڸ���  
	//    dig_1 (4bit): mhsensor_data.MagHeading�� 1�� �ڸ���  
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

	// 4. ������ �����͸� SFI_MHS01 ���信 �µ��� ������ hi3587 tx ����(TxBusWord[0])�� ����
    TxBusWord[0] = 0;
    TxBusWord[0] |= (uint32_t)arinc429_word.parity << PARITY;
    TxBusWord[0] |= (uint32_t)arinc429_word.ssm << SSM;
    TxBusWord[0] |= (uint32_t)arinc429_word.dig_4 << DIG4;
    TxBusWord[0] |= (uint32_t)arinc429_word.dig_3 << DIG3;
    TxBusWord[0] |= (uint32_t)arinc429_word.dig_2 << DIG2;
    TxBusWord[0] |= (uint32_t)arinc429_word.dig_1 << DIG1;
    TxBusWord[0] |= (uint32_t)arinc429_word.pad << PAD;
    TxBusWord[0] |= (uint32_t)arinc429_word.label << LABEL;


	// 5. �ڱ��� x�� �����͸� SFI_MHS02 ���信 �µ��� ������ hi3587 tx ����(TxBusWord[1])�� ����
		//	  parity(1bit): ���������� parity ���� 0(deflult : Odd Parity)
	//    ssm   (2bit): status Matrix �Է� (11 : normal, 01: No computed Data, 10: function Test, 00: Failure Warming )
	//    NULL  (3bit) : forced to 0
	//    data  (15bit) : Range/2^(15bit) , Range : ��100 ��T
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

	// 6. �ڱ��� y�� �����͸� SFI_MHS03 ���信 �µ��� ������ hi3587 tx ����(TxBusWord[2])�� ����
	//	  parity(1bit): ���������� parity ���� 0(deflult : Odd Parity)
	//    ssm   (2bit): status Matrix �Է� (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
	//    NULL  (3bit) : forced to 0
	//    data  (15bit) : Range/2^(15bit) , Range : ��100 ��T
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


	// 7. �ڱ��� z�� �����͸� SFI_MHS04 ���信 �µ��� ������ hi3587 tx ����(TxBusWord[3])�� ����
	//	  parity(1bit): ���������� parity ���� 0(deflult : Odd Parity)
	//    ssm   (2bit): status Matrix �Է� (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
	//    NULL  (3bit) : forced to 0
	//    data  (15bit) : Range/2^(15bit) , Range : ��100 ��T
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


    // 8. �ڵ� check sum �� �µ�, ����, eeprom ���� ���¸� SFI_MHS04 ���信 �µ��� ������ hi3587 tx ����(TxBusWord[4])�� ����
    //    parity(1bit): ���������� parity ���� 0(deflult : Odd Parity)
    //    ssm   (2bit): status Matrix �Է� (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
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

    // 9. ����Ʈ���� ������ SFI_MHS04 ���信 �µ��� ������ hi3587 tx ����(TxBusWord[4])�� ����
    //    parity(1bit): ���������� parity ���� 0(deflult : Odd Parity)
    //    ssm   (2bit): status Matrix �Է� (00 : normal, 01: No computed Data, 10: function Test, 11: Not used)
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


	// 8. writeTxFIFO()�� �̿��Ͽ� arinc429 �������� TxBusWord[0..3]�� ����
    writeTxFIFO(DATA_6_LEN);

	// 9. readStatusReg()�� �̿��Ͽ� ���� ���¸� Ȯ��. (���⼭�� �Ϸ���� ��� ���� �ʴ´�. ���� ������ �� �Լ��� ����� ������.)
    (void)readStatusReg();

	// 10. HI3579 �� ������� �����ϱ� ���Ͽ� 1 byte �� dummy �����͸� �����Ѵ�.
    txOpCode (0x12);


//    498             00000001001001100000000000001100b
//    7015    0x1B67  00000011011011001110000011000000b
//    8336    0x2090  00000100000100100000000011000001b
//    -7697   0xE1EF  00011100001111011110000011000010b


}

