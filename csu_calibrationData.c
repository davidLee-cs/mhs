/*
 * csu_calibrationData.c
 *
 * ��� :
 * ������� : Data ���� ��� CSU (D-MHS-SFR-007)
 * �̷� :
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */

#include "mhs_project.h"

#define FLASH_BASE_ADDR        (0x080002U)


uint16_t  bParameterError;		// eeprom ���κ��� �Ķ���� �д� ���� Ȯ�� : 1 : error, 0 : No error
uint16_t gCheckSum_code;        // �ʱⰪ  f124
uint16_t gversion_code;
uint16_t eepromcrc;

struct _mhsensor_calibration_Data mhsensor_calibration_Data;
struct _mhsensor_fluxrightAngle_Data mhsensor_fluxrightAngle_Data;
struct _mhsensor_accelrightAngle_Data mhsensor_accelrightAngle_Data;
struct _mhsensor_sensor_Data mhsensor_sensor_Data;


static uint16_t data_read_from_eeprom(uint32_t address, int16_t * pdata);
static uint16_t crc16_modbus_little_endian(const int16_t* data, size_t length);
static uint16_t FLASH_READ_u16(uint32_t offset);


//��ɼ���
// eeprom���� ����� calibration �� �о� ���� �Լ�
// eeprom �� ����� ���ӵ� �̵�(x,y,z), ���ӵ� �ɼ�(x,y,z), �ڱ��� �̵�(x,y,z), �ڱ��� �ɼ�(x,y,z) �о� �� ������ �����ϴ� �Լ�
// ��� ��������
// float64_t mhsensor_calibration_Data.Gain_Ax  // ���ӵ� x�� �̵�
// float64_t mhsensor_calibration_Data.Gain_Ay  // ���ӵ� y�� �̵�
// float64_t mhsensor_calibration_Data.Gain_Az  // ���ӵ� z�� �̵�
// int16_t  mhsensor_calibration_Data.Offset_Ax  // ���ӵ� x�� �ɼ�
// int16_t  mhsensor_calibration_Data.Offset_Ay  // ���ӵ� y�� �ɼ�
// int16_t  mhsensor_calibration_Data.Offset_Az  // ���ӵ� z�� �ɼ�
// float64_t mhsensor_calibration_Data.Gain_Bx   // �ڱ��� x�� �̵�
// float64_t mhsensor_calibration_Data.Gain_By     // �ڱ��� y�� �̵�
// float64_t mhsensor_calibration_Data.Gain_Bz   // �ڱ��� z�� �̵�
// int16_t  mhsensor_calibration_Data.Offset_Bx  // �ڱ��� x�� �ɼ�
// int16_t  mhsensor_calibration_Data.Offset_By  // �ڱ��� y�� �ɼ�
// int16_t  mhsensor_calibration_Data.Offset_Bz  // �ڱ��� z�� �ɼ�
// int16_t mhsensor_calibration_Data.calOffsetBx  //  �ڱ��� x�� calibration ��� �ɼ�
// int16_t mhsensor_calibration_Data.calOffsetBy  // �ڱ��� y�� calibration ��� �ɼ�
// int16_t mhsensor_calibration_Data.calOffsetBz   // �ڱ��� z�� calibration ��� �ɼ�
// uint16_t  bParameterError // eeprom ���κ��� �Ķ���� �д� ���� Ȯ��(1 : error, 0 : No error)
void read_parameter(void)
{
    uint16_t error = 0U;
    int16_t eepromBuffer[30] = {0,};
    int16_t resultcrc = 0;
    uint32_t i=0U;

	// 1. EEPROM �ּҿ��� ����� �ӽ� ������ �ε�
	//    EEPROM_GAIN_AX_ADDRESS : Gain_Ax
	//    EEPROM_GAIN_AY_ADDRESS : Gain_Ay
	//    EEPROM_GAIN_AZ_ADDRESS : Gain_Az
	//    EEPROM_OFFSET_AX_ADDRESS : Offset_Ax
	//    EEPROM_OFFSET_AY_ADDRESS : Offset_Ay
	//    EEPROM_OFFSET_AZ_ADDRESS : Offset_Az
	//    EEPROM_GAIN_BX_ADDRESS : Gain_Bx
	//    EEPROM_GAIN_BY_ADDRESS : Gain_By
	//    EEPROM_GAIN_BZ_ADDRESS : Gain_Bz
	//    EEPROM_OFFSET_BX_ADDRESS : Offset_Bx
	//    EEPROM_OFFSET_BY_ADDRESS : Offset_By
	//    EEPROM_OFFSET_BZ_ADDRESS : Offset_Bz
	//    EEPROM_BX_CAL_OFFSET_ADDRESS : calOffsetBx
	//    EEPROM_BY_CAL_OFFSET_ADDRESS : calOffsetBy
	//    EEPROM_BZ_CAL_OFFSET_ADDRESS : calOffsetBz
	error = data_read_from_eeprom(EEPROM_GAIN_AX_ADDRESS, &eepromBuffer[0]);
	error |= data_read_from_eeprom(EEPROM_GAIN_AY_ADDRESS, &eepromBuffer[1]);
	eepromBuffer[2] = 0;
	error |= data_read_from_eeprom(EEPROM_OFFSET_AX_ADDRESS, &eepromBuffer[3]);
	error |= data_read_from_eeprom(EEPROM_OFFSET_AY_ADDRESS, &eepromBuffer[4]);
	eepromBuffer[5] = 0;
	error |= data_read_from_eeprom(EEPROM_GAIN_BX_ADDRESS, &eepromBuffer[6]);
	error |= data_read_from_eeprom(EEPROM_GAIN_BY_ADDRESS, &eepromBuffer[7]);
	error |= data_read_from_eeprom(EEPROM_GAIN_BZ_ADDRESS, &eepromBuffer[8]);
	error |= data_read_from_eeprom(EEPROM_OFFSET_BX_ADDRESS, &eepromBuffer[9]);
	error |= data_read_from_eeprom(EEPROM_OFFSET_BY_ADDRESS, &eepromBuffer[10]);
	error |= data_read_from_eeprom(EEPROM_OFFSET_BZ_ADDRESS, &eepromBuffer[11]);
	error |= data_read_from_eeprom(EEPROM_BX_CAL_OFFSET_ADDRESS, &eepromBuffer[12]);
	error |= data_read_from_eeprom(EEPROM_BY_CAL_OFFSET_ADDRESS, &eepromBuffer[13]);
	error |= data_read_from_eeprom(EEPROM_BZ_CAL_OFFSET_ADDRESS, &eepromBuffer[14]);

    error |= data_read_from_eeprom(EEPROM_BX00_RA_ADDRESS, &eepromBuffer[15]);
    error |= data_read_from_eeprom(EEPROM_BX01_RA_ADDRESS, &eepromBuffer[16]);
    error |= data_read_from_eeprom(EEPROM_BX02_RA_ADDRESS, &eepromBuffer[17]);
    error |= data_read_from_eeprom(EEPROM_BY10_RA_ADDRESS, &eepromBuffer[18]);
    error |= data_read_from_eeprom(EEPROM_BY11_RA_ADDRESS, &eepromBuffer[19]);
    error |= data_read_from_eeprom(EEPROM_BY12_RA_ADDRESS, &eepromBuffer[20]);
    error |= data_read_from_eeprom(EEPROM_BZ20_RA_ADDRESS, &eepromBuffer[21]);
    error |= data_read_from_eeprom(EEPROM_BZ21_RA_ADDRESS, &eepromBuffer[22]);
    error |= data_read_from_eeprom(EEPROM_BZ22_RA_ADDRESS, &eepromBuffer[23]);

    error |= data_read_from_eeprom(EEPROM_AX00_RA_ADDRESS, &eepromBuffer[24]);
    error |= data_read_from_eeprom(EEPROM_AX01_RA_ADDRESS, &eepromBuffer[25]);
    error |= data_read_from_eeprom(EEPROM_AY10_RA_ADDRESS, &eepromBuffer[26]);
    error |= data_read_from_eeprom(EEPROM_AY11_RA_ADDRESS, &eepromBuffer[27]);

    eepromBuffer[28] = 0;
    error |= data_read_from_eeprom(EEPROM_VERSION_ADDRESS, &eepromBuffer[29]);  // software version

    error |= data_read_from_eeprom(EEPROM_CHK_CRC_ADDRESS, &resultcrc);  // eeprom crc16

    size_t data_len = (size_t)(sizeof(eepromBuffer) / sizeof(eepromBuffer[0]));
    eepromcrc = crc16_modbus_little_endian(eepromBuffer, data_len);


    if((uint16_t)resultcrc != eepromcrc)
    {
        error = 1;
    }

    arinc429_error.fluxError = 0;
    arinc429_error.eepromError = 0;
    arinc429_error.tempError = 0;
    arinc429_error.voltageError = 0;


	// 2. �б� ������ ������  �ӽ� �������� ��� ������ ���ӵ�, �ڱ��� �Ķ���Ϳ� ������ ����
	if (error==0U)
	{

#if 0
	    //������
//	    ACCEL_X_GAIN:   +000183
//	    ACCEL_Y_GAIN:   +000218
//	    ACCEL_Z_GAIN:   +000000
//	    ACCEL_X_OFFSET: -000214
//	    ACCEL_Y_OFFSET: -000900
//	    ACCEL_Z_OFFSET: +000000
//	    FLUX__X_GAIN:   +010610
//	    FLUX__Y_GAIN:   +010580
//	    FLUX__Z_GAIN:   +010600
//	    FLUX__X_OFFSET: +000010
//	    FLUX__Y_OFFSET: +000130
//	    FLUX__Z_OFFSET: +000060
//	    CAL__X__OFFSET: +000000
//	    CAL__Y__OFFSET: +000000
//	    CAL__Z__OFFSET: +000000

        mhsensor_calibration_Data.Gain_Bx = 1.0517;
        mhsensor_calibration_Data.Gain_By = 1.0541;
        mhsensor_calibration_Data.Gain_Bz = 1.0550;
        mhsensor_calibration_Data.Offset_Bx   = 2;
        mhsensor_calibration_Data.Offset_By   = 198;
        mhsensor_calibration_Data.Offset_Bz   = 28;

        mhsensor_calibration_Data.Gain_Ax = 0.000179;
        mhsensor_calibration_Data.Gain_Ay = 0.000206;
        mhsensor_calibration_Data.Gain_Az = 0.0001;
        mhsensor_calibration_Data.Offset_Ax   = -195;
        mhsensor_calibration_Data.Offset_Ay   = -912;
        mhsensor_calibration_Data.Offset_Az   = 0;

        mhsensor_calibration_Data.calOffsetBx = 1;
        mhsensor_calibration_Data.calOffsetBy = 1;
        mhsensor_calibration_Data.calOffsetBz = 1;

#endif

	    mhsensor_calibration_Data.Gain_Ax = (float64_t)eepromBuffer[0] / 1000000.0L;
	    mhsensor_calibration_Data.Gain_Ay = (float64_t)eepromBuffer[1] / 1000000.0L;
	    mhsensor_calibration_Data.Gain_Az = (float64_t)eepromBuffer[2];
	    mhsensor_calibration_Data.Offset_Ax = (int16_t)eepromBuffer[3];
	    mhsensor_calibration_Data.Offset_Ay = (int16_t)eepromBuffer[4];
	    mhsensor_calibration_Data.Offset_Az = (int16_t)eepromBuffer[5];
	    mhsensor_calibration_Data.Gain_Bx = (float64_t)eepromBuffer[6] / 10000.0L;
	    mhsensor_calibration_Data.Gain_By = (float64_t)eepromBuffer[7] / 10000.0L;
	    mhsensor_calibration_Data.Gain_Bz = (float64_t)eepromBuffer[8] / 10000.0L;
	    mhsensor_calibration_Data.Offset_Bx = (int16_t)eepromBuffer[9];
	    mhsensor_calibration_Data.Offset_By = (int16_t)eepromBuffer[10];
	    mhsensor_calibration_Data.Offset_Bz = (int16_t)eepromBuffer[11];
	    mhsensor_calibration_Data.calOffsetBx = (int16_t)eepromBuffer[12];
	    mhsensor_calibration_Data.calOffsetBy = (int16_t)eepromBuffer[13];
	    mhsensor_calibration_Data.calOffsetBz = (int16_t)eepromBuffer[14];

	    mhsensor_fluxrightAngle_Data.matrix_x00 = (float64_t)eepromBuffer[15] * 0.0001L;
	    mhsensor_fluxrightAngle_Data.matrix_x01 = (float64_t)eepromBuffer[16] * 0.0001L;
	    mhsensor_fluxrightAngle_Data.matrix_x02 = (float64_t)eepromBuffer[17] * 0.0001L;
	    mhsensor_fluxrightAngle_Data.matrix_y10 = (float64_t)eepromBuffer[18] * 0.0001L;
	    mhsensor_fluxrightAngle_Data.matrix_y11 = (float64_t)eepromBuffer[19] * 0.0001L;
	    mhsensor_fluxrightAngle_Data.matrix_y12 = (float64_t)eepromBuffer[20] * 0.0001L;
	    mhsensor_fluxrightAngle_Data.matrix_z20 = (float64_t)eepromBuffer[21] * 0.0001L;
	    mhsensor_fluxrightAngle_Data.matrix_z21 = (float64_t)eepromBuffer[22] * 0.0001L;
	    mhsensor_fluxrightAngle_Data.matrix_z22 = (float64_t)eepromBuffer[23] * 0.0001L;

	    mhsensor_accelrightAngle_Data.matrix_x00 = (float64_t)eepromBuffer[24] * 0.0001L;
        mhsensor_accelrightAngle_Data.matrix_x01 = (float64_t)eepromBuffer[25] * 0.0001L;
        mhsensor_accelrightAngle_Data.matrix_y10 = (float64_t)eepromBuffer[26] * 0.0001L;
        mhsensor_accelrightAngle_Data.matrix_y11 = (float64_t)eepromBuffer[27] * 0.0001L;


        // �ڵ� ��ü checksum
        gCheckSum_code = (uint16_t)eepromBuffer[28];

        gCheckSum_code = 0U;
        for(i=0;i<0x00017FFEU;i++)
        {
            gCheckSum_code += FLASH_READ_u16(i);
        }

        // �ڵ� ����
        gversion_code = (uint16_t)eepromBuffer[29];


        accel_matrix_init();
        flux_matrix_init();

	}
	else
	{
        // ��õ����
        mhsensor_calibration_Data.Gain_Bx = -0.965251;
        mhsensor_calibration_Data.Gain_By = -0.949307;
        mhsensor_calibration_Data.Gain_Bz = -0.954563;
        mhsensor_calibration_Data.Offset_Bx   = -120;
        mhsensor_calibration_Data.Offset_By   = 80;
        mhsensor_calibration_Data.Offset_Bz   = -147;

        mhsensor_calibration_Data.Gain_Ax = 0.000169;
        mhsensor_calibration_Data.Gain_Ay = -0.00019;
        mhsensor_calibration_Data.Gain_Az = 0.0001;
        mhsensor_calibration_Data.Offset_Ax   = 160;
        mhsensor_calibration_Data.Offset_Ay   = -700;
        mhsensor_calibration_Data.Offset_Az   = 0;

        mhsensor_calibration_Data.calOffsetBx = 0;
        mhsensor_calibration_Data.calOffsetBy = 0;
        mhsensor_calibration_Data.calOffsetBz = 0;

        mhsensor_fluxrightAngle_Data.matrix_x00 = 1.0L;
        mhsensor_fluxrightAngle_Data.matrix_x01 = 0.0L;
        mhsensor_fluxrightAngle_Data.matrix_x02 = 0.0L;
        mhsensor_fluxrightAngle_Data.matrix_y10 = 0.0L;
        mhsensor_fluxrightAngle_Data.matrix_y11 = 1.0L;
        mhsensor_fluxrightAngle_Data.matrix_y12 = 0.0L;
        mhsensor_fluxrightAngle_Data.matrix_z20 = 0.0L;
        mhsensor_fluxrightAngle_Data.matrix_z21 = 0.0L;
        mhsensor_fluxrightAngle_Data.matrix_z22 = 1.0L;

        mhsensor_accelrightAngle_Data.matrix_x00 = 1.0L;
        mhsensor_accelrightAngle_Data.matrix_x01 = 0.0L;
        mhsensor_accelrightAngle_Data.matrix_y10 = 0.0L;
        mhsensor_accelrightAngle_Data.matrix_y11 = 1.0L;


        gCheckSum_code = 0U;
        gversion_code = 0xA000U;

	}


	if((mhsensor_calibration_Data.calOffsetBx == 0) && (mhsensor_calibration_Data.calOffsetBy == 0) && (mhsensor_calibration_Data.calOffsetBz == 0))
	{
	    mhsensor_calibration_Data.ssm = 0x01;
	}
	else
	{
	    mhsensor_calibration_Data.ssm = 0x00;
	}


	// 3. bParameterError �� �б� ���� ���� �߻����� ��� (1:�߻�, 0 : �̹߻�)
    bParameterError = error;
}

//��ɼ���
//eeprom �ּҿ� Ư�� �����͸� �����ϴ� �Լ�
//
//����� ���� ����
//uint32_t address : eeprom���� ������ �ּ�
//uint16_t data :  eeprom�� ����� ������
//return �� : eeprom ���� ���� �߻� ��Ȳ (1: ���� �߻�, 0 : ���� �۵�)
//
//����� ���� ���� ����
//uint32_t ControlAddr; // Read/Write �� EEPROM �� �ּ�. (i2c address �ƴ�)
//uint16_t TX_MsgBuffer[64];// EEPROM �� ����� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
//struct I2CHandle EEPROM;     // eeprom �� I2C �������̽��� ����
uint16_t data_write_to_eeprom(uint32_t address, uint16_t data)
{
    uint16_t error = 0U;
    uint16_t wrstatus;
    //Example 3: EEPROM word (16-bit) write
    //EEPROM address 0x1 = 22 &  0x2 = 33

	// 1. ����� �ּҿ� �����͸� ���� 
    ControlAddr = address;   //EEPROM address to write
    EEPROM.NumOfDataBytes  = 3;
    TX_MsgBuffer[0]        = data & 0x00FFU ;        // low
    TX_MsgBuffer[1]        = (data >> 8) & 0x00FFU ; // high
    TX_MsgBuffer[2]        = 0;
    TX_MsgBuffer[2]        ^= TX_MsgBuffer[0];
    TX_MsgBuffer[2]        ^= TX_MsgBuffer[1];
    EEPROM.pTX_MsgBuffer   = TX_MsgBuffer;
    wrstatus = I2C_MasterTransmitter(&EEPROM);

	// 2. eeprom chip ����ȭ���� ��� 
    delay_uS((float64_t)EEPROM.WriteCycleTime_in_us);

	// 3. ����� �ּ��� �����͸� �ٽ� �б�
	ControlAddr = address;
    EEPROM.pControlAddr   = &ControlAddr;
    EEPROM.pRX_MsgBuffer  = RX_MsgBuffer;
    EEPROM.NumOfDataBytes = 3;

    wrstatus = I2C_MasterReceiver(&EEPROM);

    if(wrstatus != 0U)
    {
    	error = 1U;
    }

	// 5. ������ �����Ϳ� ���� ���� �����͸� ���Ͽ� ���� ��ϵǾ����� Ȯ��.
    error |= verifyEEPROMRead();

    return error;
}

uint16_t data_write_to_eeprom_calibraion(uint32_t address, uint16_t data)
{
    uint16_t error = 0U;
    uint16_t wrstatus;
    //Example 3: EEPROM word (16-bit) write
    //EEPROM address 0x1 = 22 &  0x2 = 33

    // 1. ����� �ּҿ� �����͸� ����
    ControlAddr = address;   //EEPROM address to write
    EEPROM.NumOfDataBytes  = 3;
    TX_MsgBuffer[0]        = data & 0x00FFU ;        // low
    TX_MsgBuffer[1]        = (data >> 8) & 0x00FFU ; // high
    TX_MsgBuffer[2]        = 0;
    TX_MsgBuffer[2]        ^= TX_MsgBuffer[0];
    TX_MsgBuffer[2]        ^= TX_MsgBuffer[1];
    EEPROM.pTX_MsgBuffer   = TX_MsgBuffer;
    wrstatus = I2C_MasterTransmitter(&EEPROM);

    // 2. eeprom chip ����ȭ���� ���
    delay_uS((float64_t)EEPROM.WriteCycleTime_in_us);

    // 3. ����� �ּ��� �����͸� �ٽ� �б�
    ControlAddr = address;
    EEPROM.pControlAddr   = &ControlAddr;
    EEPROM.pRX_MsgBuffer  = RX_MsgBuffer;
    EEPROM.NumOfDataBytes = 3;

    wrstatus = I2C_MasterReceiver(&EEPROM);

    if(wrstatus != 0U)
    {
        error = 1U;
    }

    // 5. ������ �����Ϳ� ���� ���� �����͸� ���Ͽ� ���� ��ϵǾ����� Ȯ��.
    error |= verifyEEPROMRead();

    return error;
}

//��� ����
// eeprom  Ư���ּ��� ���� word data �� �о���� �Լ�.
// �б� �����̸� 1, �����̸� 0�� ����
//
//����� ���� ����
//uint32_t address : eeprom���� �о� �� �ּ�
//uint16_t * pdata :  ���� �ּҿ��� ����� word �����͸� ������ ������
//return �� : eeprom �о� �������� ���� �߻� ��Ȳ (1: ���� �߻�, 0 : ���� �۵�)
//
//���� ���� ����
//RX_MsgBuffer[] : i2c isr �� ���� ������ ������ ���� ����
static uint16_t data_read_from_eeprom(uint32_t address, int16_t * pdata)
{
	uint16_t ret_status = 0U;

	// 1. eeprom ���� address �� �ּ� �����͸� �о� �´�.
	if (data_Read(address) == SUCCESS)
	{
		// 1.1 �д� ���� �����ϸ� pdata�� �о�� �����͸� �����Ѵ�.
		uint16_t read_High = RX_MsgBuffer[1] << 8;
		uint16_t read_Low = RX_MsgBuffer[0] & 0x00FFU;
		uint16_t read_val = read_High | read_Low;
		int16_t  eepData = (int16_t)(read_val);

		*pdata = eepData;
	}
	else
	{
		// 1.2 ���� �� ������ ������ return ���� ������ ������ �����Ѵ�. 
		//     ��. �б� ������ �ִ°�� ��� ������ �����͸� �������� ����.
			
		ret_status = 1U; 	// error
	}

	return ret_status;
}

// CRC-16-MODBUS ��� �Լ� (��Ʋ����� ���)
static uint16_t crc16_modbus_little_endian(const int16_t* data, size_t length) {
    uint16_t crc = 0xFFFFU;
    const uint16_t polynomial = 0xA001U;
    size_t i;
    int16_t b,j;
    uint8_t crcbytes[2];

    for (i = 0U; i < length; i++) {
        // Little-endian: LSB ���� ó��
        uint16_t crcdata = (uint16_t)data[i];
        uint16_t low = crcdata & 0x00FFU;
        uint16_t high = (crcdata >> 8) & 0x00FFU;

        crcbytes[0] = (uint8_t)low;         // ���� ����Ʈ
        crcbytes[1] = (uint8_t)high;   // ���� ����Ʈ

        for (b = 0; b < 2; ++b) {
            crc ^= crcbytes[b];
            for (j = 0; j < 8; ++j) {
                if ((crc & 0x0001U) == 0x0001U){
                    crc = (crc >> 1) ^ polynomial;
                }
                else{
                    crc >>= 1;
                }
            }
        }
    }

    return crc;
}



static uint16_t FLASH_READ_u16(uint32_t offset)
{
    uint16_t *addr = (uint16_t *)((uint32_t)FLASH_BASE_ADDR + offset);
    return *addr;
}


