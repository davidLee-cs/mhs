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


#if 0
// ��� :  I2C ���ͷ�Ʈ ���� ��ƾ �Լ�
//       �߻��� I2C_INT_RXFF ���ͷ�Ʈ��  �߻��� �� �ֵ���  �߻��� INTERRUPT �� CLEAR �Ѵ�.
// ���� ����� ���� : 
//			 ControlAddr : eeprom���� ���� �ּ� (ISR�� ����) 
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
__interrupt void i2c_isr(void)
{
	// 1. ���� ��ġ�� ���������� �����̺����� Ȯ�� 
    uint16_t MasterSlave = HWREGH(currentResponderPtr->base + I2C_O_MDR);

	// 2. currentResponderPtr�� ����Ű�� ��ġ�� ���� ������ ó��
    handleI2C_ErrorCondition(currentResponderPtr);

	// 3.  I2C ��尡 �����͸�� ���� �����Ͽ� ������ ��尡 �ƴҽ� 3.1~ 3.2 ����
    if((MasterSlave & I2C_MDR_MST) != 0U)
    {
    	// 3.1���� FIFO ������ ���ͷ�Ʈ(I2C_INT_RXFF)�� Ȱ��ȭ�Ͽ� ���� ���ۿ� ���� ���ͷ�Ʈ �߻� Ȱ��ȭ 
        I2C_enableInterrupt(currentResponderPtr->base, I2C_INT_RXFF);

		// 3.2 I2C_INT_RXFF ���ͷ�Ʈ �÷��׸� �ʱ�ȭ
        I2C_clearInterruptStatus(currentResponderPtr->base,(I2C_INT_RXFF));
    }

	// 4. ���ͷ�Ʈ �׷� 8 ACK �÷��׸� clear �Ͽ� GROUP8 INTERRUPT �� ��� �߻��� �� �ֵ��� ��.
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP8);
}


// ��� : I2C FIFO ���ͷ�Ʈ�� �߻����� �� ����Ǵ� ���ͷ�Ʈ ���� ��ƾ �Լ�
//       ȣ��� ���ͷ�Ʈ �׷� 8 ACK �÷��׸� clear �Ͽ� GROUP8 INTERRUPT �� ��� �߻��� �� �ֵ��� ��.
// �����  ���� ����
//      currentResponderPtr : ���� ����ϴ� I2C �������̽� ����
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
__interrupt void i2cFIFO_isr(void)
{
	// 1. currentResponderPtr �����Ͱ� ����Ű�� ��ġ�� �۽�(TX) �� ����(RX) FIFO�� ó��
    Write_Read_TX_RX_FIFO(currentResponderPtr);

	// 2. ���ͷ�Ʈ �׷� 8 ACK �÷��׸� �ʱ�ȭ
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP8);
}

// ��� : MCU eeprom �������̽��� ����ϱ� ���� �� ���� 
//      SDA(104), SCL(105) �� ���� GPIO ����
// �̷� :
//      2024.05.23 : ���漱 : �ʱ� �ۼ� 
void   I2C_GPIO_init(void)
{
	// GPIO#  DIR  PULLUP/DOWN ASSING_CORE  ASYNC/SYNC
	//  104   IN   PULLUP         CPU1        ASYNC
	//  105   IN   PULLUP         CPU1        ASYNC

    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SDAA, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SDAA, GPIO_PIN_TYPE_PULLUP);
    GPIO_setMasterCore(DEVICE_GPIO_PIN_SDAA, GPIO_CORE_CPU1);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SDAA, GPIO_QUAL_ASYNC);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCLA, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCLA, GPIO_PIN_TYPE_PULLUP);
    GPIO_setMasterCore(DEVICE_GPIO_PIN_SCLA, GPIO_CORE_CPU1);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCLA, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_SDAA);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCLA);
}

// ��� : MCU�� I2C �������̽� ��� ����
// �̷� :
//      2024.05.23 : ���漱 : �ʱ� �ۼ� 
static void I2CsubInit(void)
{
	// I2C ��� ����
	// BitCount : 8��Ʈ ���, 
	// DataCount : 2 bytes
	// AddressMode : �ּҸ�� (7 bits)
	// FIFO(Enable)
	// INTERRUPT MASK : I2C_INT_ARB_LOST | I2C_INT_NO_ACK Enable
	// INTERRUPT LEVEL : I2C_FIFO_TXEMPTY, I2C_FIFO_RX2
	// INTERRUPT ENABLE : I2C_INT_ADDR_SLAVE | I2C_INT_ARB_LOST | I2C_INT_NO_ACK | I2C_INT_STOP_CONDITION
	// EmulationMode : I2C_EMULATION_FREE_RUN
	
    I2C_setBitCount(I2CA_BASE, I2C_BITCOUNT_8);
    I2C_setDataCount(I2CA_BASE, 2);
    I2C_setAddressMode(I2CA_BASE, I2C_ADDR_MODE_7BITS);
    I2C_enableFIFO(I2CA_BASE);
    I2C_clearInterruptStatus(I2CA_BASE, I2C_INT_ARB_LOST | I2C_INT_NO_ACK);
    I2C_setFIFOInterruptLevel(I2CA_BASE, I2C_FIFO_TXEMPTY, I2C_FIFO_RX2);
    I2C_enableInterrupt(I2CA_BASE, I2C_INT_ADDR_SLAVE | I2C_INT_ARB_LOST | I2C_INT_NO_ACK | I2C_INT_STOP_CONDITION);
    I2C_setEmulationMode(I2CA_BASE, I2C_EMULATION_FREE_RUN);

}

// ��� : I2C �������̽��� �ʱ� ���� 
//      MCU �� EEPROM �� I2C �ּ�, ��� �ӵ� �� ����
// �̷� :
//      2024.05.23 : ���漱 : �ʱ� �ۼ� 
void I2Cinit(void)
{
    //1. I2C ����� ��Ȱ��ȭ
    I2C_disableModule(I2CA_BASE);
	// 2. I2C ������ ��带 �ʱ�ȭ, I2C ����� Ŭ�� ���ļ��� ����, ��żӵ� 100kHz�� ����, Ŭ�� ��Ƽ 50% ���� 
    I2C_initMaster(I2CA_BASE, DEVICE_SYSCLK_FREQ, 100000U, I2C_DUTYCYCLE_50);
	// 3. I2C ������ �۽� ���� ����
    I2C_setConfig(I2CA_BASE, I2C_MASTER_SEND_MODE);
	// 4. EEPROM �� I2C �����̺��� �ּҸ� ����(80)
    I2C_setSlaveAddress(I2CA_BASE, 80); 
	// 5. MCU �� I2C ����� ��ü �����̺� �ּҸ� ����(96)
    I2C_setOwnSlaveAddress(I2CA_BASE, 96);
	// 6. ������ ��带 ��Ȱ��ȭ
    I2C_disableLoopback(I2CA_BASE);
	// 7. I2C �������̽��� �ʱ� ���� �Լ� ȣ��  
    I2CsubInit();
	// 8. ��� ������ �Ϸ�� ��, I2C ����� Ȱ��ȭ
    I2C_enableModule(I2CA_BASE);
}

// ���:  I2C bus�� busy ���°� �ƴҶ� ���� ��� �ϴ� �Լ�
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
static void I2C_wait(uint32_t base)
{
	// 1. I2C �������� ���� ����(Start Condition)�� ����
    I2C_sendStartCondition(base);

    // 2. I2C �������� ���� ����(Start Condition)�� ���� �Ϸ� ���
    while((I2C_getStatus(base) & I2C_STS_REG_ACCESS_RDY) == 0U)
    {
        ;
    }

    // 3. I2C �������� ���� ����(Stop Condition)�� ����
    I2C_sendStopCondition(base);

    // 4. I2C �������� ���� ����(Stop Condition)�� ���� �Ϸ� ���
    while(1)
    {
   		// 4.1 STOP ������ 0 �̸� ��� ���� 
		if(I2C_getStopConditionStatus(base) == 0)
		{
		   break;
		}
    }

    // 5. I2C ������ BUSY ������ ���� �ɶ����� ���
    while(1)
    {	
   		// 5.1 ���� ���°�  0 �̸� ��� ���� 
        if(I2C_isBusBusy(base) == 0)
        {
            break;
        }
    }
}


// ��� : I2C ������ ����� �����̺� ��ġ�� Ž���ϴ� ����� �����ϴ� �Լ� 
// �̷� :
//      2024.05.23 : ���漱 : �ʱ� �ۼ� 

static uint16_t I2CBusScan(uint32_t base, uint16_t *pAvailableI2C_slaves)
{
    uint16_t probeSlaveAddress, i;
    uint16_t status = 0U;

    //Disable interrupts on Stop condition, NACK and arbitration lost condition
    I2C_disableInterrupt(base, (I2C_INT_ADDR_SLAVE|I2C_INT_STOP_CONDITION | I2C_INT_ARB_LOST | I2C_INT_NO_ACK));

    i = 0;
    for(probeSlaveAddress=1;probeSlaveAddress<=MAX_10_BIT_ADDRESS;probeSlaveAddress++)
    {
        //Check I2C bus status
        status = checkBusStatus(base);
        if(status != 0U)
        {
           ESTOP0;
           return status;
        }

        I2C_setConfig(base, (I2C_MASTER_SEND_MODE | I2C_REPEAT_MODE));

        //Enable 10-bit addressing if probeSlaveAddress is greater than 127U
        if(probeSlaveAddress > MAX_7_BIT_ADDRESS)
        {
            //10-bit addressing
            I2C_setAddressMode(base, I2C_ADDR_MODE_10BITS);
        }

        // Setup slave address
        I2C_setSlaveAddress(base, probeSlaveAddress);


        I2C_sendStartCondition(base);

        //Wait for the slave address to be transmitted
        while(!(I2C_getStatus(base) & I2C_STS_REG_ACCESS_RDY))
        {
            ;
        }

        //Generate STOP condition
        I2C_sendStopCondition(base);

        //Wait for the I2CMDR.STP to be cleared
        while(I2C_getStopConditionStatus(base))
        {
            ;
        }

        //Wait for the Bus busy bit to be cleared
        while(I2C_isBusBusy(base))
        {
            ;
        }

        uint16_t I2CStatus = I2C_getStatus(base);

        //If Slave address is acknowledged, store slave address
        //in pAvailableI2C_slaves
        if(!(I2CStatus & I2C_STS_NO_ACK))
        {
            pAvailableI2C_slaves[i++] = probeSlaveAddress;
        }
        //Clear NACK bit in I2CSTR
        I2C_clearStatus(base,I2C_STS_NO_ACK|I2C_STS_ARB_LOST|I2C_STS_REG_ACCESS_RDY|I2C_STS_STOP_CONDITION);
    }

    I2C_setConfig(base, (I2C_MASTER_SEND_MODE));
    I2C_setAddressMode(base, I2C_ADDR_MODE_7BITS); //7-bit addressing
    I2C_enableInterrupt(base, (I2C_INT_ADDR_SLAVE|I2C_INT_STOP_CONDITION | I2C_INT_ARB_LOST | I2C_INT_NO_ACK));
    return SUCCESS;
}

#if 0
static uint16_t I2CBusScan(uint32_t base, uint16_t *pAvailableI2C_slaves)
{
    uint16_t probeSlaveAddress, i;
    uint16_t status = 0U;
    uint16_t busstatus = 0U;

    // 1. ������ ����� ��ġ ��ĵ�� ���ͷ�Ʈ�� ���Ͽ� ���ع��� �ʵ��� ���ͷ�Ʈ ��Ȱ��ȭ 
    //    I2C_INT_STOP_CONDITION, I2C_INT_ARB_LOST,I2C_INT_NO_ACK 
    I2C_disableInterrupt(base, (I2C_INT_ADDR_SLAVE|I2C_INT_STOP_CONDITION | I2C_INT_ARB_LOST | I2C_INT_NO_ACK));

    i = 0;
	// 2. ������ ��� �����̺� �ּҸ� Ž�� ����
    for(probeSlaveAddress=1;probeSlaveAddress<=MAX_10_BIT_ADDRESS;probeSlaveAddress++)
    {
        // 2.1. I2C ���� ���¸� Ȯ�� (checkBusStatus() ���), BUSY �Ǵ� NOT_READY �̸� ��� Ž���� �����Ѵ�.
        busstatus = checkBusStatus(base);
        if(busstatus != 0U)
        {
           status = busstatus;
           break;
        }
	    else 
	    {
	    	// 2.2 ������ BUSY �Ǵ� NOT_READY �� �ƴ� ���� ������ ����  2.2.1 ~ 2.2.6 �� �����Ѵ�.
			// 2.2.1 ������ �۽� ��� �� �ݺ� ��带 ����
	        I2C_setConfig(base, (I2C_MASTER_SEND_MODE | I2C_REPEAT_MODE));

	        // 2.2.2 ���κ� �ּҰ� 127(7��Ʈ �ּ��� �ִ밪)�� �ʰ��ϴ� ��� 10��Ʈ �ּ� ���� ����
	        if(probeSlaveAddress > MAX_7_BIT_ADDRESS)
	        {
	            //10-bit addressing
	            I2C_setAddressMode(base, I2C_ADDR_MODE_10BITS);
	        }

	        // 2.2.3 �����̺� �ּ� ���� 
	        I2C_setSlaveAddress(base, probeSlaveAddress);


			// 2.2.4 I2C ��� 
	        I2C_wait(base);

			// 2.2.5 �����̺� �ּҿ� ���� NACK ���¸� Ȯ��, ������ ������ pAvailableI2C_slaves�� �ּ� ���� 
	        uint16_t I2CStatus = I2C_getStatus(base);
	        if((I2CStatus & I2C_STS_NO_ACK) == 0U)
	        {
	            pAvailableI2C_slaves[i++] = probeSlaveAddress;
	        }
			
			// 2.2.6 �����̺� ���µ���  Ŭ����  : I2C_INT_STOP_CONDITION, I2C_INT_ARB_LOST,I2C_INT_NO_ACK , I2C_STS_REG_ACCESS_RDY
	        I2C_clearStatus(base,I2C_STS_NO_ACK|I2C_STS_ARB_LOST|I2C_STS_REG_ACCESS_RDY|I2C_STS_STOP_CONDITION);
	    }
    }

	// 4. �����̺� �ּ� Ž���� �Ϸ����� Ȯ��
    if(status == 0U)
    {
		// 4.1 ������ �۽� ���� ���� 
        I2C_setConfig(base, (I2C_MASTER_SEND_MODE));
		// 4.2 ���κ� �ּ� 7��Ʈ�� ����  
        I2C_setAddressMode(base, I2C_ADDR_MODE_7BITS); 
		// 4.3 ���ͷ�Ʈ Ȱ��ȭ : I2C_INT_ADDR_SLAVE, I2C_INT_STOP_CONDITION, I2C_INT_ARB_LOST,I2C_INT_NO_ACK 
		I2C_enableInterrupt(base, (I2C_INT_ADDR_SLAVE|I2C_INT_STOP_CONDITION | I2C_INT_ARB_LOST | I2C_INT_NO_ACK));

		// 4.4 ���¸� SUCCESS(0x0000)�� ���� 
        status = SUCCESS;
    }
    else
    {
    	// 4.5 ���� ���� ���¸� ���� 
        status = busstatus;
    }

	// 5. ���� ���¸� ����, SUCCESS (0x0000), 
    return status;
}
#endif

// ��� : I2C �����ͷμ� �����̺� �����͸� �����ϱ� ���� �ʱ� ���� �Լ� 
//      
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
static uint16_t I2C_TransmitSlaveAddress_ControlBytes(struct I2CHandle *I2C_Params)
{
    uint16_t status;

    uint32_t base = I2C_Params->base;

    status = checkBusStatus(base);
    if(status != 0U)
    {
        return status;
    }

    I2C_disableFIFO(base);

    I2C_setConfig(base, (I2C_MASTER_SEND_MODE));

    if((I2C_Params->SlaveAddr) > MAX_7_BIT_ADDRESS)
    {
        //10-bit addressing
        I2C_setAddressMode(base, I2C_ADDR_MODE_10BITS);
    }

    // Setup slave address
    I2C_setSlaveAddress(base, I2C_Params->SlaveAddr);

    I2C_setDataCount(base, (I2C_Params->NumOfAddrBytes));

    I2C_enableFIFO(base);

    uint32_t temp = *(I2C_Params->pControlAddr);

    temp = temp & 0x00FFFFFFU;

    temp |= (uint32_t)(I2C_Params->NumOfDataBytes)<<24U;

    int16_t i;
    i = I2C_Params->NumOfAddrBytes-1;

    for(i=I2C_Params->NumOfAddrBytes-1;i>=0;i--)
    {
       I2C_putData(base, (temp >> (i*8U)) & 0xFFU);
    }

    I2C_sendStartCondition(base);

    return SUCCESS;
}



#if 0
static uint16_t I2C_TransmitSlaveAddress_ControlBytes(struct I2CHandle const *I2C_Params)
{
    uint16_t status;
    uint16_t busstatus;


    uint32_t base = I2C_Params->base;
	// 1. I2C ���� ���¸� Ȯ��, 0�� �ƴϸ� status�� ���� ����  
    busstatus = checkBusStatus(base);
    if(busstatus != 0U)
    {
        status = busstatus;
    }
    else
    {
    	//1.1 I2C ���� ���°� 1�̸� 1.1.1 ~ 1.1.9�� �����Ѵ�.
    	//1.1.1 FIFO ��Ȱ��ȭ
        I2C_disableFIFO(base);

		// 1.1.2 I2C�� ������ �۽� ���� ����
        I2C_setConfig(base, (I2C_MASTER_SEND_MODE));
		// 1.1.3 �����̺� �ּҰ� 127(7��Ʈ �ּ��� �ִ밪)�� �ʰ��ϴ� ��� 10��Ʈ �ּ� ���� ����
        if((I2C_Params->SlaveAddr) > MAX_7_BIT_ADDRESS)
        {
            //10-bit addressing
            I2C_setAddressMode(base, I2C_ADDR_MODE_10BITS);
        }

        // 1.1.4 �����̺� �ּ� ���� 
        I2C_setSlaveAddress(base, I2C_Params->SlaveAddr);

		// 1.1.5 ���� �� ������ ����Ʈ �� ����
        I2C_setDataCount(base, (I2C_Params->NumOfAddrBytes));

		// 1.1.6 FIFO Ȱ��ȭ
        I2C_enableFIFO(base);

		// 1.1.7 ���� �ּҸ� �����ͼ� ���� 24��Ʈ�� ����� ���� 8��Ʈ�� ������ ������ ����Ʈ ���� ����
        uint32_t temp = *(I2C_Params->pControlAddr);
        temp = temp & 0x00FFFFFFU;
        temp |= (uint32_t)(I2C_Params->NumOfDataBytes)<<24U;

		// 1.1.8 ������ ���� �ּҿ� ������ ����Ʈ ���� ���� ����Ʈ���� ���� ����Ʈ ������ FIFO�� ����
		int16_t i;
		for(i=(int16_t)I2C_Params->NumOfAddrBytes-1;i>=0;i--)
        {
           I2C_putData(base, (uint16_t)(temp >> ((uint16_t)i*8U)) & 0x00FFU);
        }

		// 1.1.9 I2C �������� ���� ����(Start Condition)�� �����Ͽ� �����̺���� ���
        I2C_sendStartCondition(base);

        status = SUCCESS;
    }

	// 2. ���� ���¸� ����
    return status;
}
#endif

// ��� :  I2C ������ ��忡�� �����̺꿡 �����͸� �۽��ϴ� �Լ� 
//      
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 

uint16_t I2C_MasterTransmitter(struct I2CHandle *I2C_Params)
{
    uint16_t status;

    uint32_t base = I2C_Params->base;

    I2C_Params->numofSixteenByte  = (I2C_Params->NumOfDataBytes) / I2C_FIFO_LEVEL;
    I2C_Params->remainingBytes    = (I2C_Params->NumOfDataBytes) % I2C_FIFO_LEVEL;

    ASSERT(I2C_Params->NumOfDataBytes <= MAX_BUFFER_SIZE);

    I2C_enableFIFO(base);

    status = I2C_TransmitSlaveAddress_ControlBytes(I2C_Params);

    if(status != 0U)
    {
        return status;
    }

    I2C_setDataCount(base, (I2C_Params->NumOfAddrBytes + I2C_Params->NumOfDataBytes));

    I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, I2C_FIFO_RXFULL);

    I2C_enableInterrupt(base, (I2C_INT_STOP_CONDITION | I2C_INT_ARB_LOST | I2C_INT_NO_ACK));
    I2C_enableInterrupt(base, I2C_INT_TXFF);

    I2C_clearInterruptStatus(base, I2C_INT_TXFF);

    return SUCCESS;
}

#if 0
uint16_t I2C_MasterTransmitter(struct I2CHandle *I2C_Params)
{
    uint16_t status;
    uint16_t i2cstatus;

    uint32_t base = I2C_Params->base;

	// 1. �۽� ������ �غ� 
    I2C_Params->numofSixteenByte  = (I2C_Params->NumOfDataBytes) / I2C_FIFO_LEVEL;
    I2C_Params->remainingBytes    = (I2C_Params->NumOfDataBytes) % I2C_FIFO_LEVEL;

	// 2. �۽��� �������� ũ�Ⱑ �ִ� ���� ũ�� �������� Ȯ��
    if(I2C_Params->NumOfDataBytes <= MAX_BUFFER_SIZE)
    {
    	// 2.1 ������ ������ ���¸� SUCCESS ���� 
        status = SUCCESS;
    }
    else
    {
		// 2.2 �۽��� �������� ũ�Ⱑ �ִ� ���� ũ�⸦ �ʰ��ϴ� ��� FIFO�� Ȱ��ȭ
        I2C_enableFIFO(base);

		// 2.3 �����̺� �ּ� �� ���� ����Ʈ �۽�, ������ ��� 0 ����. 
        i2cstatus = I2C_TransmitSlaveAddress_ControlBytes(I2C_Params);

		// 2.4 �����̺� �ּҿ� ���� ����Ʈ �۽��� �������� Ȯ��
        if(i2cstatus == 0U)
        {
        	// 2.4.1  �۽��� �����Ϳ� �ּ��� �� ����Ʈ ���� ������
            I2C_setDataCount(base, (I2C_Params->NumOfAddrBytes + I2C_Params->NumOfDataBytes));

			// 2.4.2 FIFO ���ͷ�Ʈ ���� ����
            I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, I2C_FIFO_RXFULL);

			// 2.4.3 ���ͷ�Ʈ Ȱ��ȭ 
            I2C_enableInterrupt(base, (I2C_INT_STOP_CONDITION | I2C_INT_ARB_LOST | I2C_INT_NO_ACK));

			// 2.4.4 �۽� FIFO ���ͷ�Ʈ Ȱ��ȭ
            I2C_enableInterrupt(base, I2C_INT_TXFF);

			// 2.4.4 �۽� FIFO ���ͷ�Ʈ ���¸� �ʱ�ȭ			
            I2C_clearInterruptStatus(base, I2C_INT_TXFF);

			//2.4.5 ���¸� SUCCESS�� ����
            status = SUCCESS;
        }
        else
        {	
        	//2.4.5  �����̺� �ּҿ� ���� ����Ʈ �۽��� ���и� ���� ���¸� ���� 
            status = i2cstatus;
        }
    }

	//3. ���� ���� 
    return status;
}
#endif

// ��� : I2C ������ ��忡�� �����̺� ��ġ�κ��� �����͸� ����
//      
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
uint16_t I2C_MasterReceiver(struct I2CHandle *I2C_Params)
{
    uint16_t status;
    uint32_t base = I2C_Params->base;

	// 1. �۽� ������ �غ� 
    I2C_Params->numofSixteenByte  = (I2C_Params->NumOfDataBytes) / I2C_FIFO_LEVEL;
    I2C_Params->remainingBytes    = (I2C_Params->NumOfDataBytes) % I2C_FIFO_LEVEL;

	// 2. ���ͷ�Ʈ ��Ȱ��ȭ
    I2C_disableInterrupt(base, I2C_INT_TXFF|I2C_INT_RXFF);

	// 3. ���ͷ�Ʈ ���� �ʱ�ȭ
    I2C_clearInterruptStatus(base, (I2C_INT_REG_ACCESS_RDY|I2C_INT_TXFF|I2C_INT_RXFF));

	// 4. ���ͷ�Ʈ Ȱ��ȭ (�������� ���� �غ�) 
    I2C_enableInterrupt(base, I2C_INT_REG_ACCESS_RDY);

	// 5. �����̺� �ּ� �� ���� ����Ʈ �۽�
    status = I2C_TransmitSlaveAddress_ControlBytes(I2C_Params);

	// 6. �ð� ���� 
    SysCtl_delay(50); //Adding delay to correctly read I2C bus status

    if(status != 0U)
    {
        return status;
    }

    return SUCCESS;
}


// ��� : I2C ������ BUSY �� NOT_READY ���¸� Ȯ���ϴ� �Լ� 
//      
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
static uint16_t checkBusStatus(uint32_t base)
{

    uint16_t done = SUCCESS;

	// 1. ������ ��� ���� ���, status�� ERROR_BUS_BUSY�� ����
    if(I2C_isBusBusy(base))
    {
        done = ERROR_BUS_BUSY;
    }
	// 2. I2C �������� ���� Ȯ��,  ���� ������ �غ���� �ʾҴٸ�, status�� ERROR_STOP_NOT_READY ���� 
    else if(I2C_getStopConditionStatus(base))
    {
        done = ERROR_STOP_NOT_READY;
    }
    else
    {
        done = SUCCESS;
    }

    return done;
}


#if 0
// 
// ��� : I2C ��ſ��� NACK(No Acknowledge) ��ȣ�� ó���ϴ� �Լ� 
//      
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
uint16_t handleNACK(uint32_t base)
{
    uint16_t status;

	// 1. NACK ���� Ȯ��
    if((I2C_getStatus(base) & I2C_STS_NO_ACK) != 0U)
    {
    	//1.1 NACK ���� �ʱ�ȭ
        I2C_clearStatus(base, I2C_STS_NO_ACK);
		//1.2 FIFO ��Ȱ��ȭ
        I2C_disableFIFO(base);
		// 1.3 ���� ���� ����
		I2C_sendStopCondition(base);
		// 1.4 FIFO ��Ȱ��ȭ
        I2C_enableFIFO(base);

		// 1.5 NACK�̹Ƿ� status�� ERROR_NACK_RECEIVED�� ����
        status =  ERROR_NACK_RECEIVED;
    }
    else
    {
    	// 1.6 NACK �ƴϸ� status�� SUCCESS ����
        status = SUCCESS;
    }

	// 2. ���� ���� 
    return status;
}
#endif

// 
// ��� : ������ ��忡�� ���� ���� ��ȯ�ϱ� �Լ�  
//      
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
static void I2C_accessReady(uint32_t base)
{
	// 1. ���ͷ�Ʈ ��Ȱ��ȭ
    I2C_disableInterrupt(base, I2C_INT_REG_ACCESS_RDY);
    I2C_disableInterrupt(base, I2C_INT_TXFF);
	// 2. FIFO ��Ȱ��ȭ �� ��Ȱ��ȭ
    I2C_disableFIFO(base);
    I2C_enableFIFO(base);

	// 3. ���� ���� ����
    I2C_setConfig(base, (I2C_MASTER_RECEIVE_MODE));

	// 4. �۽� fifo ���ͷ�Ʈ ���� Ŭ����
    I2C_clearInterruptStatus(base, I2C_INT_TXFF);
}


// ��� : I2C ��ſ��� �����̺� ����� �۽� �Ǵ� ���� ���¸� �����ϴ� �Լ� 
//      
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
static void I2C_addressSave(uint32_t base)
{
	// 1. �����Ͱ� �����̺�κ��� �����͸� ��û�ϴ� �۽� ��� ���� Ȯ��, 0 �ƴϸ� �۽Ÿ�� 
    if(((I2C_getStatus(base) & I2C_STS_SLAVE_DIR)) != 0U)
    {
    	// 1.1 �����̺� ��带 �۽� ���� ����
        I2C_setConfig(base, I2C_SLAVE_SEND_MODE);
        // 1.2 TX FIFO ���ͷ�Ʈ�� Ȱ��ȭ�ϰ�, RX FIFO ���ͷ�Ʈ�� ��Ȱ��ȭ
        I2C_enableInterrupt(base, I2C_INT_TXFF);
        I2C_disableInterrupt(base, I2C_INT_RXFF);
		// 1.3 ���ͷ�Ʈ ���¸� Ŭ����
		I2C_clearInterruptStatus(base, (I2C_INT_TXFF|I2C_INT_RXFF));
    }
    else
    {
    	// 1.4 �����̺� ��带 ���� ���� ����
        I2C_setConfig(base, I2C_SLAVE_RECEIVE_MODE);

        // 1.5 TX FIFO ���ͷ�Ʈ�� Ȱ��ȭ�ϰ�, RX FIFO ���ͷ�Ʈ�� ��Ȱ��ȭ
        I2C_disableInterrupt(base, I2C_INT_TXFF);
        I2C_enableInterrupt(base, I2C_INT_RXFF);
		
		// 1.6 ���ͷ�Ʈ ���¸� Ŭ����
        I2C_clearInterruptStatus(base, (I2C_INT_TXFF|I2C_INT_RXFF));

    }
}

// ��� :  I2C ����� �پ��� ���� ��Ȳ�� ó���ϴ� �Լ� 
//			I2C_INTSRC_NONE,				//!< No interrupt pending
//			I2C_INTSRC_ARB_LOST,			//!< Arbitration-lost interrupt
//			I2C_INTSRC_NO_ACK,				//!< NACK interrupt
//			I2C_INTSRC_REG_ACCESS_RDY,		//!< Register-access-ready interrupt
//			I2C_INTSRC_RX_DATA_RDY, 		//!< Receive-data-ready interrupt
//			I2C_INTSRC_TX_DATA_RDY, 		//!< Transmit-data-ready interrupt
//			I2C_INTSRC_STOP_CONDITION,		//!< Stop condition detected
//			I2C_INTSRC_ADDR_TARGET			 //!< Addressed as target interrupt
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
static void handleI2C_ErrorCondition(struct I2CHandle *I2C_Params)
{

    uint32_t base = I2C_Params->base;
    uint16_t status;

	// 1. ���ͷ�Ʈ �ҽ� Ȯ��
    I2C_InterruptSource intSource = I2C_getInterruptSource(base);

    switch (intSource)
    {

    	// 1.1 Arbitration Lost ��Ȳ ó��
        case I2C_INTSRC_ARB_LOST:
            //Report Arbitration lost failure
            status = ERROR_ARBITRATION_LOST;
            break;

		// 1.2 NACK(ACK ���� ����) ó�� 
        case I2C_INTSRC_NO_ACK:
            //1.2.1 NACK(ACK ���� ����) �÷��׸� Ŭ�����ϰ�, STOP ������ ����
            I2C_clearStatus(base, I2C_STS_NO_ACK);
            I2C_sendStopCondition(base);
            status = ERROR_NACK_RECEIVED;
            break;

		// 1.3  �������� ���� �غ� ���ͷ�Ʈ 
        case I2C_INTSRC_REG_ACCESS_RDY:
            I2C_disableInterrupt(base, I2C_INT_REG_ACCESS_RDY);
            I2C_disableInterrupt(base, I2C_INT_TXFF);
            I2C_disableFIFO(base);
            I2C_enableFIFO(base);
            I2C_setConfig(base, (I2C_MASTER_RECEIVE_MODE));
            I2C_clearInterruptStatus(base, I2C_INT_TXFF);
            if(I2C_Params->numofSixteenByte != 0U)
            {
                I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, I2C_FIFO_RXFULL);
            }
            else
            {
                I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, (I2C_RxFIFOLevel)I2C_Params->remainingBytes);
            }

			// 1.3.3 ������ ���� ������ �� ���� �� ���� ������ ����
            I2C_setDataCount(base, I2C_Params->NumOfDataBytes);
            I2C_sendStartCondition(base);
            I2C_sendStopCondition(base);

            break;
			
		// 1.4 ���� ������ �غ� 
        case I2C_INTSRC_RX_DATA_RDY:
            break;
			
		// 1.5 �۽� ������ �غ� 
        case I2C_INTSRC_TX_DATA_RDY:
            break;

		// 1.6 STOP ���� �߻� ���ͷ�Ʈ  
        case I2C_INTSRC_STOP_CONDITION:
			// 1.6.1 �۽� �� ���� ���ͷ�Ʈ�� ��Ȱ��ȭ, �޽��� ���� �����͸� ����
            I2C_disableInterrupt(base, (I2C_INT_TXFF | I2C_INT_RXFF));
            I2C_Params->pTX_MsgBuffer   = TX_MsgBuffer;
            I2C_Params->pRX_MsgBuffer   = RX_MsgBuffer;
            break;

		// 1.7 �����̺� �ּ� ��û ���ͷ�Ʈ 
        case I2C_INTSRC_ADDR_SLAVE:
            // 1.7.1 �ۼ��� FIFO ���ͷ�Ʈ ������ �����ϰ�, �����̺� �ּҸ� ����
            I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, (I2C_RxFIFOLevel)(I2C_Params->NumOfAddrBytes));

            if((I2C_getStatus(base) & I2C_STS_SLAVE_DIR) != 0U)
            {
                //Slave Transmitter (SDIR = 1)
                I2C_setConfig(base, I2C_SLAVE_SEND_MODE);
                //Enable TX FIFO interrupt and disable RXFF interrupt
                I2C_enableInterrupt(base, I2C_INT_TXFF);
                I2C_disableInterrupt(base, I2C_INT_RXFF);
                I2C_clearInterruptStatus(base, (I2C_INT_TXFF|I2C_INT_RXFF));
            }
            else
            {
                //Slave Receiver (SDIR = 0)
                I2C_setConfig(base, I2C_SLAVE_RECEIVE_MODE);

                //Enable RX FIFO interrupt and disable TXFF interrupt
                I2C_disableInterrupt(base, I2C_INT_TXFF);
                I2C_enableInterrupt(base, I2C_INT_RXFF);
                I2C_clearInterruptStatus(base, (I2C_INT_TXFF|I2C_INT_RXFF));

            }
            break;

        default :
           break;

    }
}

// ��� : I2C ���ͷ�Ʈ ���¿� ���� ������ �۽� �Ǵ� ������ ó���ϴ� �Լ� 
//      
// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ� 
static void Write_Read_TX_RX_FIFO(struct I2CHandle *I2C_Params)
{
    int16_t i;
    uint32_t base = I2C_Params->base;
    uint16_t numofSixteenByte = I2C_Params->numofSixteenByte;
    uint16_t remainingBytes  = I2C_Params->remainingBytes;

	// 1. ���� I2C ���ͷ�Ʈ ���¿� TX FIFO ���ͷ�Ʈ Ȱ��ȭ ���¸� Ȯ��
    struct I2CHandle *currentPtr = I2C_Params->currentHandlePtr;
    uint32_t intSource = (uint32_t)I2C_getInterruptStatus(base);
    uint32_t txFIFOinterruptenabled = HWREGH(base + I2C_O_FFTX) & I2C_FFTX_TXFFIENA;


    // 2. ���� FIFO ���ͷ�Ʈ�̰�, ���� �ּ�(pControlAddr)�� 0 ���� Ȯ�� 
    if(((intSource & I2C_INT_RXFF) != 0U) && ((I2C_Params->pControlAddr) == NULL))
    {
        uint32_t Addr_Ctrl = 0U;
        int16_t NumRX_Bytes = I2C_getRxFIFOStatus(base);

		// 2.1. Addr_Ctrl�� ���� �����͸� �����Ͽ� pControlAddr �� ������ ����Ʈ ���� ����
        for(i=NumRX_Bytes-1;i>=0;i--)
        {
            Addr_Ctrl |= (uint32_t)(I2C_getData(base))<<(i*8U);
        }

		// 2.2 ������Ʈ�� ���� �ּҿ� ������ ����Ʈ ���� �����մϴ�.
        I2C_Params->pControlAddr       = (uint32_t *)Addr_Ctrl;
        I2C_Params->NumOfDataBytes     = Addr_Ctrl >> 24U;
        I2C_Params->numofSixteenByte   = I2C_Params->NumOfDataBytes / I2C_FIFO_LEVEL;
        I2C_Params->remainingBytes     = I2C_Params->NumOfDataBytes % I2C_FIFO_LEVEL;

        numofSixteenByte = I2C_Params->numofSixteenByte;
        remainingBytes   = I2C_Params->remainingBytes;
		
		// 2.3. �۽��� �������� ���� ���� FIFO ���ͷ�Ʈ ������ ����
        if(numofSixteenByte != 0U)
        {
            I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, I2C_FIFO_RXFULL);
        }
        else
        {
            I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, (I2C_RxFIFOLevel)remainingBytes);
        }
		// 2.4 ���� FIFO ���ͷ�Ʈ Ŭ���� 
        I2C_clearInterruptStatus(base,(I2C_INT_RXFF));
    }

    else
    {
    	// 2.5 �۽� FIFO(TXFF) �Ǵ� ���� FIFO(RXFF) ���ͷ�Ʈ�� Ȱ��ȭ�Ǿ� �ִ��� Ȯ��
        if(((intSource & I2C_INT_TXFF) != 0U) || ((intSource & I2C_INT_RXFF) != 0U))
        {
          //2.5.1 �ۼ��� �����Ͱ� ���� �ִ��� Ȯ��  
          if((remainingBytes != 0U) && (numofSixteenByte == 0U))
          {
          	// 2.5.1.1 �۽� ���ͷ�Ʈ�� Ȱ��ȭ�� ��� ���� �ִ� ����Ʈ ����ŭ �����͸� �۽�
          	// 		   ���� ���ͷ�Ʈ�� Ȱ��ȭ�� ��� ���ŵ� �����͸� ���ۿ� ����
            for(i=0;i<remainingBytes;i++)
            {
                if(((intSource & I2C_INT_TXFF) != 0U) && (txFIFOinterruptenabled != 0U))
                {
                    I2C_putData(base, *(currentPtr->pTX_MsgBuffer++));
                }
                if((intSource & I2C_INT_RXFF) != 0U)
                {
                    *(currentPtr->pRX_MsgBuffer++) = I2C_getData(base);
                }
            }
            remainingBytes = 0U;
          }

          //2.5.2 numofSixteenByte�� 0���� ũ�� 16����Ʈ ������ �ۼ����� ó��
          if(numofSixteenByte != 0U)
          {

			// 2.5.2.1 �۽� ���ͷ�Ʈ�� Ȱ��ȭ�� ��� ���� �ִ� ����Ʈ ����ŭ �����͸� �۽� �� numofSixteenByte ���� ��Ŵ 
            if(((intSource & I2C_INT_TXFF) !=0U) && (txFIFOinterruptenabled !=0U))
            {
                for(i=0;i<I2C_FIFO_TXFULL;i++)
                {
                   I2C_putData(base, *(currentPtr->pTX_MsgBuffer++));
                }
                numofSixteenByte--;
            }
			
			// 2.5.2.2 ���� ���ͷ�Ʈ�� Ȱ��ȭ�� ��� ���ŵ� �����͸� ���ۿ� ���� �� numofSixteenByte ���� ��Ŵ
            if((intSource & I2C_INT_RXFF) != 0U)
            {
                for(i=0;i<I2C_FIFO_RXFULL;i++)
                {
                    *(currentPtr->pRX_MsgBuffer++) = I2C_getData(base);
                }
                numofSixteenByte--;
            }
          }

          // 2.5.3 numofSixteenByte�� 0�̰�, ���� ����Ʈ�� �ִ��� Ȯ�� 
          if((numofSixteenByte == 0U) && (remainingBytes != 0U))
          {
          	// 2.5.3.1 RX FIFO ������ ���� ����Ʈ ���� ����
            I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, (I2C_RxFIFOLevel)remainingBytes);

          }

          // 2.5.4 ��� ����Ʈ�� ���� �Ǿ����� Ȯ�� 
          if((remainingBytes == 0U) && (numofSixteenByte == 0U))
          {
          	// 2.5.4.1 I2C ������ ����� ��� STOP ������ �����Ͽ� ����� ����
			if(HWREGH(I2C_Params->base + I2C_O_MDR) & I2C_MDR_MST)
			{
			  I2C_sendStopCondition(base);
			}
          }

		  // 2.5.5 ���ͷ�Ʈ ���� �÷��׸� �ʱ�ȭ
          I2C_clearInterruptStatus(base,(I2C_INT_TXFF | I2C_INT_RXFF));
        }

		// 2.5.6 numofSixteenByte �� remainingBytes ���� ������Ʈ�Ͽ� ���� �ۼ��� �� ���
        I2C_Params->numofSixteenByte = numofSixteenByte;
        I2C_Params->remainingBytes   = remainingBytes;
    }
}
#endif
