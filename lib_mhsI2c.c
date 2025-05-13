/*
 * lib_mhsI2c.c
 *
 * 기능 :1. MCU의 I2C 인터페이스 제공, 
 		2. eeprom 읽기/쓰기를 하기위한 
 * 구성요소 : 
 * 이력 : 
 *    2024.05.23 : 이충순 : 초기 작성
 */

#include "driverlib.h"
#include "device.h"
#include "mhs_project.h"
#include "i2cLib_FIFO_master_interrupt.h"

#define EEPROM_SLAVE_ADDRESS        0x50

struct I2CHandle EEPROM;						// eeprom 의 I2C 인터페이스를 정보 
struct I2CHandle *currentResponderPtr;    // 현재 사용하는 I2C 인터페이스 정보
uint32_t ControlAddr;							// Read/Write 할 EEPROM 의 주소. (i2c address 아님)
uint16_t TX_MsgBuffer[MAX_BUFFER_SIZE];			// EEPROM 에 기록할 데이터를 ISR과 다른 csu와 공유하는 버퍼
uint16_t RX_MsgBuffer[MAX_BUFFER_SIZE];			// EEPROM 에서 읽은 데이터를 ISR과 다른 csu와 공유하는 버퍼
uint16_t status;

// 기능 : EEPROM을 사용하기 하기 위한 초기화 
//     : 인터페이스할 EEPROM의 eeprom 슬레이브 주소, eeprom 쓰기 사이클, eeprom에 데이털를 저장할 주소, 주소 사이즈를 설정하는 함수
// 입출력  전역 변수
// 		EEPROM : I2C 인터페이스를 위한 eeprom 정보 
// 		ControlAddr : Read/Write 할 EEPROM 의 주소. (i2c address 아님)
//      currentResponderPtr : 현재 사용하는 I2C 인터페이스 정보
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
void eepromInit(void)
{
    const uint16_t AvailableI2C_slaves[20]={0,};

	// 1. I2C 버스를 스캔하여 연결된 I2C 슬레이브 장치를 탐색하고, 결과를 pAvailableI2C_slaves에 저장
    uint16_t *pAvailableI2C_slaves = AvailableI2C_slaves;
    (void)I2CBusScan(I2CA_BASE, pAvailableI2C_slaves);


	// 2. EEPROM 구조체에 eeprom 슬레이브 주소, eeprom 쓰기 사이클, eeprom에 데이털를 저장할 주소, 주소 사이즈를 설정한다.  
    currentResponderPtr = &EEPROM;

    EEPROM.currentHandlePtr     = &EEPROM;
    EEPROM.SlaveAddr            = EEPROM_SLAVE_ADDRESS;
    EEPROM.WriteCycleTime_in_us = 10000;    //6ms for EEPROM this code was tested
    EEPROM.base                 = I2CA_BASE;
    EEPROM.pControlAddr         = &ControlAddr;
    EEPROM.NumOfAddrBytes       = 2;

}

// 기능 :address 에 해당하는 1word 데이터를 읽어 RX_MsgBuffer[] 에 저장하는 함수
// 입출력 변수 : 
//           address : 읽을 eeprom 의 주소 
// return 값 : 일기 설공시 (0), 실패시(1)
// 전역 입출력 변수
//			 ControlAddr : eeprom에서 읽을 주소 (ISR과 연동) 
// 			EEPROM		 : eeprom 의 I2C 인터페이스를 정보 
//          RX_MsgBuffer : EEPROM 에서 읽은 데이터를 ISR과 다른 csu와 공유하는 버퍼
// 이력 :
//      2024.05.23 : 이충선 : 초기 작성 
uint16_t data_Read(uint32_t address)
{
    uint16_t i2cstatus = 0;
    uint8_t chksum=0;
    uint8_t state = 1U;

	// 1. EEPROM에서 읽고자 하는 데이터의 address를 설정, 수신 데이터 버퍼 RX_MsgBuffer 설정 , 데이터 사이즈 3바이트 설정
	ControlAddr = address;
	EEPROM.pControlAddr   = &ControlAddr;
	EEPROM.pRX_MsgBuffer  = RX_MsgBuffer;
	EEPROM.NumOfDataBytes = 3;

	// 2.  EEPROM에서 데이터를 읽어 RX_MsgBuffer[]에 저장, I2C_MasterReceiver() 사용 
	i2cstatus = I2C_MasterReceiver(&EEPROM);

	// 3. 읽기 동작 완료시까지 대기, WaitUntilDone() 사용  
    while(state == 1U)
    {
        if((I2C_getStatus(EEPROM.base) & I2C_STS_BUS_BUSY) == I2C_STS_BUS_BUSY)
        {
            state = 1U;
        }
        else
        {
            state = 0U;
        }
    }

	// 4. 수신한 RX_MsgBuffer의 3번째 byte 를 이용하여 XOR 연산 checksum 확인. checkum 이 다른경우 return 값은 (1), 같으면 (0) 으로 설정 
	chksum ^= RX_MsgBuffer[0];
	chksum ^= RX_MsgBuffer[1];

	if(RX_MsgBuffer[2] != chksum)
	{	
	    i2cstatus = 1;
	}

	// 5. 읽은 데이터 상태 리턴 0(정상), 1(비정상)
    return i2cstatus;
}

// 기능 : EEPROM에서 읽은 데이터(RX_MsgBuffer)와 이전에 전송한 데이터(TX_MsgBuffer)를 비교하여, 데이터가 정확하게 기록되어 있는지 검증하는 함수 
// 출력값 : 데이터 비교 결과 리턴, 0: 데이터 일지, 1: 데이터 불일치
// 전역 입출력 변수 :
//				EEPROM : eeprom 의 I2C 인터페이스를 정보 
//				RX_MsgBuffer : EEPROM 에서 읽은 데이터를 ISR과 다른 csu와 공유하는 버퍼
//				TX_MsgBuffer : EEPROM 에 기록할 데이터를 ISR과 다른 csu와 공유하는 버퍼
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 

uint16_t verifyEEPROMRead(void)
{
    uint16_t i;
    uint16_t error = 0;
    uint8_t state = 1U;

    while(state == 1U)
    {
        if((I2C_getStatus(EEPROM.base) & I2C_STS_BUS_BUSY) == I2C_STS_BUS_BUSY)
        {
            state = 1U;
        }
        else
        {
            state = 0U;
        }
    }

    // 1. 요청한 데이터 길이만큼 수신된 데이터(RX_MsgBuffer[i])와 전송된 데이터(TX_MsgBuffer[i])가 일치하는지 비교하여 일치하면 0, 일치하지 않으면 1으로 리턴값을 설정한다.
    for(i=0;i<EEPROM.NumOfDataBytes;i++)
    {
        if(RX_MsgBuffer[i] != TX_MsgBuffer[i])
        {
            error = 1;
            break;
        }
    }

    // 2. 데이터 비교 결과 리턴, 0: 데이터 일지, 1: 데이터 불일치
    return error;
}
