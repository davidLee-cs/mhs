/*
 * csu_calibrationData.c
 *
 * 기능 :
 * 구성요소 : Data 저장 기능 CSU (D-MHS-SFR-007)
 * 이력 :
 *    2024.05.23 : 이충순 : 초기 작성
 */

#include "mhs_project.h"

#define FLASH_BASE_ADDR        (0x080002U)


uint16_t  bParameterError;		// eeprom 으로부터 파라미터 읽는 상태 확인 : 1 : error, 0 : No error
uint16_t gCheckSum_code;        // 초기값  f124
uint16_t gversion_code;
uint16_t eepromcrc;

struct _mhsensor_calibration_Data mhsensor_calibration_Data;
struct _mhsensor_fluxrightAngle_Data mhsensor_fluxrightAngle_Data;
struct _mhsensor_accelrightAngle_Data mhsensor_accelrightAngle_Data;
struct _mhsensor_sensor_Data mhsensor_sensor_Data;


static uint16_t data_read_from_eeprom(uint32_t address, int16_t * pdata);
static uint16_t crc16_modbus_little_endian(const int16_t* data, size_t length);
static uint16_t FLASH_READ_u16(uint32_t offset);


//기능설명
// eeprom에서 저장된 calibration 값 읽어 오는 함수
// eeprom 에 저장된 가속도 이득(x,y,z), 가속도 옵셋(x,y,z), 자기장 이득(x,y,z), 자기장 옵셋(x,y,z) 읽어 각 변수에 저장하는 함수
// 출력 전역변수
// float64_t mhsensor_calibration_Data.Gain_Ax  // 가속도 x축 이득
// float64_t mhsensor_calibration_Data.Gain_Ay  // 가속도 y축 이득
// float64_t mhsensor_calibration_Data.Gain_Az  // 가속도 z축 이득
// int16_t  mhsensor_calibration_Data.Offset_Ax  // 가속도 x축 옵셋
// int16_t  mhsensor_calibration_Data.Offset_Ay  // 가속도 y축 옵셋
// int16_t  mhsensor_calibration_Data.Offset_Az  // 가속도 z축 옵셋
// float64_t mhsensor_calibration_Data.Gain_Bx   // 자기장 x축 이득
// float64_t mhsensor_calibration_Data.Gain_By     // 자기장 y축 이득
// float64_t mhsensor_calibration_Data.Gain_Bz   // 자기장 z축 이득
// int16_t  mhsensor_calibration_Data.Offset_Bx  // 자기장 x축 옵셋
// int16_t  mhsensor_calibration_Data.Offset_By  // 자기장 y축 옵셋
// int16_t  mhsensor_calibration_Data.Offset_Bz  // 자기장 z축 옵셋
// int16_t mhsensor_calibration_Data.calOffsetBx  //  자기장 x축 calibration 모드 옵셋
// int16_t mhsensor_calibration_Data.calOffsetBy  // 자기장 y축 calibration 모드 옵셋
// int16_t mhsensor_calibration_Data.calOffsetBz   // 자기장 z축 calibration 모드 옵셋
//mhsensor_fluxrightAngle_Data.matrix_x00  //자기장 x축 행렬 00
//mhsensor_fluxrightAngle_Data.matrix_x01  //자기장 x축 행렬 01
//mhsensor_fluxrightAngle_Data.matrix_x02  //자기장 x축 행렬 02
//mhsensor_fluxrightAngle_Data.matrix_y10  //자기장 y축 행렬 10
//mhsensor_fluxrightAngle_Data.matrix_y11  //자기장 y축 행렬 11
//mhsensor_fluxrightAngle_Data.matrix_y12  //자기장 y축 행렬 12
//mhsensor_fluxrightAngle_Data.matrix_z20  //자기장 z축 행렬 20
//mhsensor_fluxrightAngle_Data.matrix_z21  //자기장 z축 행렬 21
//mhsensor_fluxrightAngle_Data.matrix_z22  //자기장 z축 행렬 22
//mhsensor_accelrightAngle_Data.matrix_x00 //가속도 x축 행렬 00
//mhsensor_accelrightAngle_Data.matrix_x01 //가속도 x축 행렬 01
//mhsensor_accelrightAngle_Data.matrix_y10 //가속도 y축 행렬 10
//mhsensor_accelrightAngle_Data.matrix_y11 //가속도 y축 행렬 11
// uint16_t  bParameterError // eeprom 으로부터 파라미터 읽는 상태 확인(1 : error, 0 : No error)
// mhsensor_calibration_Data.ssm : eeprom error 체크
// eepromcrc : eeprom CRC16
// gversion_code : 소프트웨어 버전
// gCheckSum_code : 전체코드 check sum
void read_parameter(void)
{
    uint16_t error = 0U;
    int16_t eepromBuffer[30] = {0,};
    int16_t resultcrc = 0;
    uint32_t i=0U;

	// 1. EEPROM 주소에서 대상의 임시 변수에 로딩
	//  EEPROM_GAIN_AX_ADDRESS : Gain_Ax
	//  EEPROM_GAIN_AY_ADDRESS : Gain_Ay
	//  EEPROM_GAIN_AZ_ADDRESS : Gain_Az
	//  EEPROM_OFFSET_AX_ADDRESS : Offset_Ax
	//  EEPROM_OFFSET_AY_ADDRESS : Offset_Ay
	//  EEPROM_OFFSET_AZ_ADDRESS : Offset_Az
	//  EEPROM_GAIN_BX_ADDRESS : Gain_Bx
	//  EEPROM_GAIN_BY_ADDRESS : Gain_By
	//  EEPROM_GAIN_BZ_ADDRESS : Gain_Bz
	//  EEPROM_OFFSET_BX_ADDRESS : Offset_Bx
	//  EEPROM_OFFSET_BY_ADDRESS : Offset_By
	//  EEPROM_OFFSET_BZ_ADDRESS : Offset_Bz
	//  EEPROM_BX_CAL_OFFSET_ADDRESS : calOffsetBx
	//  EEPROM_BY_CAL_OFFSET_ADDRESS : calOffsetBy
	//  EEPROM_BZ_CAL_OFFSET_ADDRESS : calOffsetBz
    //  EEPROM_BX00_RA_ADDRESS  : x축 행렬 00
    //  EEPROM_BX01_RA_ADDRESS  : x축 행렬 01
    //  EEPROM_BX02_RA_ADDRESS  : x축 행렬 02
    //  EEPROM_BY10_RA_ADDRESS  : y축 행렬 10
    //  EEPROM_BY11_RA_ADDRESS  : y축 행렬 11
    //  EEPROM_BY12_RA_ADDRESS  : y축 행렬 12
    //  EEPROM_BZ20_RA_ADDRESS  : z축 행렬 20
    //  EEPROM_BZ21_RA_ADDRESS  : z축 행렬 21
    //  EEPROM_BZ22_RA_ADDRESS  : z축 행렬 22
    //  EEPROM_AX00_RA_ADDRESS  : x축 행렬 00 가속도
    //  EEPROM_AX01_RA_ADDRESS  : x축 행렬 01 가속도
    //  EEPROM_AY10_RA_ADDRESS  : y축 행렬 10 가속도
    //  EEPROM_AY11_RA_ADDRESS  : y축 행렬 11 가속도
    //  EEPROM_VERSION_ADDRESS  : 소프트웨어 버전
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

    // 2. eeprom crc16 확인 후 저장된 crc16와 다르면 에러 설정
    eepromcrc = crc16_modbus_little_endian(eepromBuffer, data_len);

    if((uint16_t)resultcrc != eepromcrc)
    {
        error = 1;
    }

    arinc429_error.fluxError = 0;
    arinc429_error.eepromError = 0;
    arinc429_error.tempError = 0;
    arinc429_error.voltageError = 0;


	// 2. 읽기 오류가 없으면  임시 변수에서 대상 변수에 가속도, 자기장 파라메터에 변수에 저장
	if (error==0U)
	{

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


        // 2.1 코드 전체 checksum을 확인
        gCheckSum_code = (uint16_t)eepromBuffer[28];

        gCheckSum_code = 0U;
        for(i=0;i<0x00017FFEU;i++)
        {
            gCheckSum_code += FLASH_READ_u16(i);
        }

        // 코드 버전
        gversion_code = (uint16_t)eepromBuffer[29];

        // 2.2 eeprom에서 읽은 가속도 및 자기장 행렬 값을 변수에 저장
        accel_matrix_init();
        flux_matrix_init();

	}
	else
	{
        //2.3 eeprom 데어터 읽기 에러 발생 시 dufault 값으로 설정
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


        gCheckSum_code = 0xEEEEU;
        gversion_code = 0x0001U;

	}

	// 3. caloffset x,y,z 값이 모두 0 이면 항공기 보정이 안되어 있는 상태이기 때문에 항공기 보정 완려 전까지 mhsensor_calibration_Data.ssm = 1 로 설정하여 STATUS_NO_COMPUT_DATA 상태로 유지
	if((mhsensor_calibration_Data.calOffsetBx == 0) && (mhsensor_calibration_Data.calOffsetBy == 0) && (mhsensor_calibration_Data.calOffsetBz == 0))
	{
	    mhsensor_calibration_Data.ssm = 0x01;
	}
	else
	{
	    mhsensor_calibration_Data.ssm = 0x00;
	}


	// 3. bParameterError 에 읽기 도중 오류 발생여부 기록 (1:발생, 0 : 미발생)
    bParameterError = error;
}

//기능설명
//eeprom 주소에 특정 데이터를 저장하는 함수
//
//입출력 변수 설명
//uint32_t address : eeprom에서 저장할 주소
//uint16_t data :  eeprom에 기록할 데이터
//return 값 : eeprom 쓰기 오류 발생 상황 (1: 오류 발생, 0 : 정상 작동)
//
//입출력 전역 변수 설명
//uint32_t ControlAddr; // Read/Write 할 EEPROM 의 주소. (i2c address 아님)
//uint16_t TX_MsgBuffer[64];// EEPROM 에 기록할 데이터를 ISR과 다른 csu와 공유하는 버퍼
//struct I2CHandle EEPROM;     // eeprom 의 I2C 인터페이스를 정보
uint16_t data_write_to_eeprom(uint32_t address, uint16_t data)
{
    uint16_t error = 0U;
    uint16_t wrstatus;
    //Example 3: EEPROM word (16-bit) write
    //EEPROM address 0x1 = 22 &  0x2 = 33

	// 1. 기록할 주소와 데이터를 전송 
    ControlAddr = address;   //EEPROM address to write
    EEPROM.NumOfDataBytes  = 3;
    TX_MsgBuffer[0]        = data & 0x00FFU ;        // low
    TX_MsgBuffer[1]        = (data >> 8) & 0x00FFU ; // high
    TX_MsgBuffer[2]        = 0;
    TX_MsgBuffer[2]        ^= TX_MsgBuffer[0];
    TX_MsgBuffer[2]        ^= TX_MsgBuffer[1];
    EEPROM.pTX_MsgBuffer   = TX_MsgBuffer;
    wrstatus = I2C_MasterTransmitter(&EEPROM);

	// 2. eeprom chip 안정화까지 대기 
    delay_uS((float64_t)EEPROM.WriteCycleTime_in_us);

	// 3. 기록한 주소의 데이터를 다시 읽기
	ControlAddr = address;
    EEPROM.pControlAddr   = &ControlAddr;
    EEPROM.pRX_MsgBuffer  = RX_MsgBuffer;
    EEPROM.NumOfDataBytes = 3;

    wrstatus = I2C_MasterReceiver(&EEPROM);

    if(wrstatus != 0U)
    {
    	error = 1U;
    }

	// 5. 전송한 데이터와 새로 읽은 데이터를 비교하여 정상 기록되었는지 확인.
    error |= verifyEEPROMRead();

    return error;
}


//기능설명
//calibration mode엣 eeprom 주소에 특정 데이터를 저장하는 함수
//
//입출력 변수 설명
//uint32_t address : eeprom에서 저장할 주소
//uint16_t data :  eeprom에 기록할 데이터
//return 값 : eeprom 쓰기 오류 발생 상황 (1: 오류 발생, 0 : 정상 작동)
//
//입출력 전역 변수 설명
//uint32_t ControlAddr; // Read/Write 할 EEPROM 의 주소. (i2c address 아님)
//uint16_t TX_MsgBuffer[64];// EEPROM 에 기록할 데이터를 ISR과 다른 csu와 공유하는 버퍼
//struct I2CHandle EEPROM;     // eeprom 의 I2C 인터페이스를 정보
uint16_t data_write_to_eeprom_calibraion(uint32_t address, uint16_t data)
{
    uint16_t error = 0U;
    uint16_t wrstatus;
    //Example 3: EEPROM word (16-bit) write
    //EEPROM address 0x1 = 22 &  0x2 = 33

    // 1. 기록할 주소와 데이터를 전송
    ControlAddr = address;   //EEPROM address to write
    EEPROM.NumOfDataBytes  = 3;
    TX_MsgBuffer[0]        = data & 0x00FFU ;        // low
    TX_MsgBuffer[1]        = (data >> 8) & 0x00FFU ; // high
    TX_MsgBuffer[2]        = 0;
    TX_MsgBuffer[2]        ^= TX_MsgBuffer[0];
    TX_MsgBuffer[2]        ^= TX_MsgBuffer[1];
    EEPROM.pTX_MsgBuffer   = TX_MsgBuffer;
    wrstatus = I2C_MasterTransmitter(&EEPROM);

    // 2. eeprom chip 안정화까지 대기
    delay_uS((float64_t)EEPROM.WriteCycleTime_in_us);

    // 3. 기록한 주소의 데이터를 다시 읽기
    ControlAddr = address;
    EEPROM.pControlAddr   = &ControlAddr;
    EEPROM.pRX_MsgBuffer  = RX_MsgBuffer;
    EEPROM.NumOfDataBytes = 3;

    wrstatus = I2C_MasterReceiver(&EEPROM);

    if(wrstatus != 0U)
    {
        error = 1U;
    }

    // 5. 전송한 데이터와 새로 읽은 데이터를 비교하여 정상 기록되었는지 확인.
    error |= verifyEEPROMRead();

    return error;
}

//기능 설명
// eeprom  특정주소의 에서 word data 를 읽어오는 함수.
// 읽기 성공이면 1, 실패이면 0을 리턴
//
//입출력 변수 설명
//uint32_t address : eeprom에서 읽어 올 주소
//uint16_t * pdata :  읽은 주소에서 기록할 word 데이터를 저장할 포인터
//return 값 : eeprom 읽어 데이터의 오류 발생 상황 (1: 오류 발생, 0 : 정상 작동)
//
//전역 변수 설명
//RX_MsgBuffer[] : i2c isr 과 읽은 데이터 공유를 위한 변수
static uint16_t data_read_from_eeprom(uint32_t address, int16_t * pdata)
{
	uint16_t ret_status = 0U;

	// 1. eeprom 에서 address 의 주소 데이터를 읽어 온다.
	if (data_Read(address) == SUCCESS)
	{
		// 1.1 읽는 것이 성공하면 pdata에 읽어온 데이터를 저장한다.
		uint16_t read_High = RX_MsgBuffer[1] << 8;
		uint16_t read_Low = RX_MsgBuffer[0] & 0x00FFU;
		uint16_t read_val = read_High | read_Low;
		int16_t  eepData = (int16_t)(read_val);

		*pdata = eepData;
	}
	else
	{
		// 1.2 읽을 때 오류가 있으면 return 값에 오류가 있음을 설정한다. 
		//     단. 읽기 오류가 있는경우 대상 변수의 데이터를 변경하지 않음.
			
		ret_status = 1U; 	// error
	}

	return ret_status;
}

//기능 설명
// CRC-16-MODBUS 계산 함수 (리틀엔디안 방식)
//
//입출력 변수 설명
//data: int16_t 타입의 데이터 배열 (16비트 정수)
//length: 배열의 길이 (int16_t 단위 개수)
//return 값 : crc 16bit

static uint16_t crc16_modbus_little_endian(const int16_t* data, size_t length) {
    uint16_t crc = 0xFFFFU;
    const uint16_t polynomial = 0xA001U;
    size_t i;
    int16_t b,j;
    uint8_t crcbytes[2];

    for (i = 0U; i < length; i++) {
        // 1. Little-endian: LSB 먼저 처리
        uint16_t crcdata = (uint16_t)data[i];
        uint16_t low = crcdata & 0x00FFU;
        uint16_t high = (crcdata >> 8) & 0x00FFU;

        crcbytes[0] = (uint8_t)low;         // 하위 바이트
        crcbytes[1] = (uint8_t)high;   // 상위 바이트

        // 2. 각 바이트에 대해 8비트씩 CRC 연산 수행, XOR 연산 후, CRC의 LSB가 1이면 다항식과 XOR 아니면 그냥 우측 쉬프트
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

    // 3. CRC-16 결과값 리턴
    return crc;
}


//기능 설명
// DSP 전체 플래시 메모리 check sum 하기 위해 특정 메모리에서 데이터 읽어오는 함수
//입출력 변수 설명
//uint32_t offset : 전체 플래시 메모리 사이즈
//return 값 : 플래시 메모리 데이터 리턴

static uint16_t FLASH_READ_u16(uint32_t offset)
{
    // 1. Flash 메모리 offset 주소의 데이터를 읽어 리턴한다.
    uint16_t *addr = (uint16_t *)((uint32_t)FLASH_BASE_ADDR + offset);
    return *addr;
}


