/*
 * lib_mhsI2c.c
 *
 * ��� :1. MCU�� I2C �������̽� ����, 
 		2. eeprom �б�/���⸦ �ϱ����� 
 * ������� : 
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */

#include "driverlib.h"
#include "device.h"
#include "mhs_project.h"
#include "i2cLib_FIFO_master_interrupt.h"

#define EEPROM_SLAVE_ADDRESS        0x50

struct I2CHandle EEPROM;						// eeprom �� I2C �������̽��� ���� 
struct I2CHandle *currentResponderPtr;    // ���� ����ϴ� I2C �������̽� ����
uint32_t ControlAddr;							// Read/Write �� EEPROM �� �ּ�. (i2c address �ƴ�)
uint16_t TX_MsgBuffer[MAX_BUFFER_SIZE];			// EEPROM �� ����� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
uint16_t RX_MsgBuffer[MAX_BUFFER_SIZE];			// EEPROM ���� ���� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
uint16_t status;

// ��� : EEPROM�� ����ϱ� �ϱ� ���� �ʱ�ȭ 
//     : �������̽��� EEPROM�� eeprom �����̺� �ּ�, eeprom ���� ����Ŭ, eeprom�� �����и� ������ �ּ�, �ּ� ����� �����ϴ� �Լ�
// �����  ���� ����
// 		EEPROM : I2C �������̽��� ���� eeprom ���� 
// 		ControlAddr : Read/Write �� EEPROM �� �ּ�. (i2c address �ƴ�)
//      currentResponderPtr : ���� ����ϴ� I2C �������̽� ����
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
void eepromInit(void)
{
    const uint16_t AvailableI2C_slaves[20]={0,};

	// 1. I2C ������ ��ĵ�Ͽ� ����� I2C �����̺� ��ġ�� Ž���ϰ�, ����� pAvailableI2C_slaves�� ����
    uint16_t *pAvailableI2C_slaves = AvailableI2C_slaves;
    (void)I2CBusScan(I2CA_BASE, pAvailableI2C_slaves);


	// 2. EEPROM ����ü�� eeprom �����̺� �ּ�, eeprom ���� ����Ŭ, eeprom�� �����и� ������ �ּ�, �ּ� ����� �����Ѵ�.  
    currentResponderPtr = &EEPROM;

    EEPROM.currentHandlePtr     = &EEPROM;
    EEPROM.SlaveAddr            = EEPROM_SLAVE_ADDRESS;
    EEPROM.WriteCycleTime_in_us = 10000;    //6ms for EEPROM this code was tested
    EEPROM.base                 = I2CA_BASE;
    EEPROM.pControlAddr         = &ControlAddr;
    EEPROM.NumOfAddrBytes       = 2;

}

// ��� :address �� �ش��ϴ� 1word �����͸� �о� RX_MsgBuffer[] �� �����ϴ� �Լ�
// ����� ���� : 
//           address : ���� eeprom �� �ּ� 
// return �� : �ϱ� ������ (0), ���н�(1)
// ���� ����� ����
//			 ControlAddr : eeprom���� ���� �ּ� (ISR�� ����) 
// 			EEPROM		 : eeprom �� I2C �������̽��� ���� 
//          RX_MsgBuffer : EEPROM ���� ���� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
// �̷� :
//      2024.05.23 : ���漱 : �ʱ� �ۼ� 
uint16_t data_Read(uint32_t address)
{
    uint16_t i2cstatus = 0;
    uint8_t chksum=0;
    uint8_t state = 1U;

	// 1. EEPROM���� �а��� �ϴ� �������� address�� ����, ���� ������ ���� RX_MsgBuffer ���� , ������ ������ 3����Ʈ ����
	ControlAddr = address;
	EEPROM.pControlAddr   = &ControlAddr;
	EEPROM.pRX_MsgBuffer  = RX_MsgBuffer;
	EEPROM.NumOfDataBytes = 3;

	// 2.  EEPROM���� �����͸� �о� RX_MsgBuffer[]�� ����, I2C_MasterReceiver() ��� 
	i2cstatus = I2C_MasterReceiver(&EEPROM);

	// 3. �б� ���� �Ϸ�ñ��� ���, WaitUntilDone() ���  
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

	// 4. ������ RX_MsgBuffer�� 3��° byte �� �̿��Ͽ� XOR ���� checksum Ȯ��. checkum �� �ٸ���� return ���� (1), ������ (0) ���� ���� 
	chksum ^= RX_MsgBuffer[0];
	chksum ^= RX_MsgBuffer[1];

	if(RX_MsgBuffer[2] != chksum)
	{	
	    i2cstatus = 1;
	}

	// 5. ���� ������ ���� ���� 0(����), 1(������)
    return i2cstatus;
}

// ��� : EEPROM���� ���� ������(RX_MsgBuffer)�� ������ ������ ������(TX_MsgBuffer)�� ���Ͽ�, �����Ͱ� ��Ȯ�ϰ� ��ϵǾ� �ִ��� �����ϴ� �Լ� 
// ��°� : ������ �� ��� ����, 0: ������ ����, 1: ������ ����ġ
// ���� ����� ���� :
//				EEPROM : eeprom �� I2C �������̽��� ���� 
//				RX_MsgBuffer : EEPROM ���� ���� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
//				TX_MsgBuffer : EEPROM �� ����� �����͸� ISR�� �ٸ� csu�� �����ϴ� ����
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 

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

    // 1. ��û�� ������ ���̸�ŭ ���ŵ� ������(RX_MsgBuffer[i])�� ���۵� ������(TX_MsgBuffer[i])�� ��ġ�ϴ��� ���Ͽ� ��ġ�ϸ� 0, ��ġ���� ������ 1���� ���ϰ��� �����Ѵ�.
    for(i=0;i<EEPROM.NumOfDataBytes;i++)
    {
        if(RX_MsgBuffer[i] != TX_MsgBuffer[i])
        {
            error = 1;
            break;
        }
    }

    // 2. ������ �� ��� ����, 0: ������ ����, 1: ������ ����ġ
    return error;
}
