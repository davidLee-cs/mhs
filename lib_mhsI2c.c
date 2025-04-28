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


#if 0
// 기능 :  I2C 인터럽트 서비스 루틴 함수
//       발생된 I2C_INT_RXFF 인터럽트가  발생할 수 있도록  발생한 INTERRUPT 를 CLEAR 한다.
// 전력 입출력 변수 : 
//			 ControlAddr : eeprom에서 읽을 주소 (ISR과 연동) 
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
__interrupt void i2c_isr(void)
{
	// 1. 현재 장치가 마스터인지 슬레이브인지 확인 
    uint16_t MasterSlave = HWREGH(currentResponderPtr->base + I2C_O_MDR);

	// 2. currentResponderPtr이 가리키는 장치의 오류 조건을 처리
    handleI2C_ErrorCondition(currentResponderPtr);

	// 3.  I2C 모드가 마스터모드 인지 학인하여 마스터 모드가 아닐시 3.1~ 3.2 수행
    if((MasterSlave & I2C_MDR_MST) != 0U)
    {
    	// 3.1수신 FIFO 버퍼의 인터럽트(I2C_INT_RXFF)를 활성화하여 수신 버퍼에 의한 인터럽트 발생 활성화 
        I2C_enableInterrupt(currentResponderPtr->base, I2C_INT_RXFF);

		// 3.2 I2C_INT_RXFF 인터럽트 플래그를 초기화
        I2C_clearInterruptStatus(currentResponderPtr->base,(I2C_INT_RXFF));
    }

	// 4. 인터럽트 그룹 8 ACK 플래그를 clear 하여 GROUP8 INTERRUPT 가 계속 발생할 수 있도록 함.
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP8);
}


// 기능 : I2C FIFO 인터럽트가 발생했을 때 실행되는 인터럽트 서비스 루틴 함수
//       호출시 인터럽트 그룹 8 ACK 플래그를 clear 하여 GROUP8 INTERRUPT 가 계속 발생할 수 있도록 함.
// 입출력  전역 변수
//      currentResponderPtr : 현재 사용하는 I2C 인터페이스 정보
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
__interrupt void i2cFIFO_isr(void)
{
	// 1. currentResponderPtr 포인터가 가리키는 장치의 송신(TX) 및 수신(RX) FIFO를 처리
    Write_Read_TX_RX_FIFO(currentResponderPtr);

	// 2. 인터럽트 그룹 8 ACK 플래그를 초기화
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP8);
}

// 기능 : MCU eeprom 인터페이스를 사용하기 위한 핀 설정 
//      SDA(104), SCL(105) 에 대한 GPIO 설정
// 이력 :
//      2024.05.23 : 이충선 : 초기 작성 
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

// 기능 : MCU의 I2C 인터페이스 모드 설정
// 이력 :
//      2024.05.23 : 이충선 : 초기 작성 
static void I2CsubInit(void)
{
	// I2C 통신 설정
	// BitCount : 8비트 모드, 
	// DataCount : 2 bytes
	// AddressMode : 주소모드 (7 bits)
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

// 기능 : I2C 인터페이스의 초기 설정 
//      MCU 와 EEPROM 의 I2C 주소, 통신 속도 등 설정
// 이력 :
//      2024.05.23 : 이충선 : 초기 작성 
void I2Cinit(void)
{
    //1. I2C 모듈을 비활성화
    I2C_disableModule(I2CA_BASE);
	// 2. I2C 마스터 모드를 초기화, I2C 통신의 클록 주파수를 설정, 통신속도 100kHz로 설정, 클럭 듀티 50% 설정 
    I2C_initMaster(I2CA_BASE, DEVICE_SYSCLK_FREQ, 100000U, I2C_DUTYCYCLE_50);
	// 3. I2C 마스터 송신 모드로 설정
    I2C_setConfig(I2CA_BASE, I2C_MASTER_SEND_MODE);
	// 4. EEPROM 의 I2C 슬레이브의 주소를 설정(80)
    I2C_setSlaveAddress(I2CA_BASE, 80); 
	// 5. MCU 의 I2C 모듈의 자체 슬레이브 주소를 설정(96)
    I2C_setOwnSlaveAddress(I2CA_BASE, 96);
	// 6. 루프백 모드를 비활성화
    I2C_disableLoopback(I2CA_BASE);
	// 7. I2C 인터페이스의 초기 설정 함수 호출  
    I2CsubInit();
	// 8. 모든 설정이 완료된 후, I2C 모듈을 활성화
    I2C_enableModule(I2CA_BASE);
}

// 기능:  I2C bus가 busy 상태가 아닐때 까지 대기 하는 함수
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
static void I2C_wait(uint32_t base)
{
	// 1. I2C 버스에서 시작 조건(Start Condition)을 전송
    I2C_sendStartCondition(base);

    // 2. I2C 버스에서 시작 조건(Start Condition)이 전송 완료 대기
    while((I2C_getStatus(base) & I2C_STS_REG_ACCESS_RDY) == 0U)
    {
        ;
    }

    // 3. I2C 버스에서 정지 조건(Stop Condition)을 전송
    I2C_sendStopCondition(base);

    // 4. I2C 버스에서 정지 조건(Stop Condition)이 전송 완료 대기
    while(1)
    {
   		// 4.1 STOP 조건이 0 이면 대기 종료 
		if(I2C_getStopConditionStatus(base) == 0)
		{
		   break;
		}
    }

    // 5. I2C 버스가 BUSY 조건이 해제 될때까지 대기
    while(1)
    {	
   		// 5.1 버스 상태가  0 이면 대기 종료 
        if(I2C_isBusBusy(base) == 0)
        {
            break;
        }
    }
}


// 기능 : I2C 버스에 연결된 슬레이브 장치를 탐지하는 기능을 수행하는 함수 
// 이력 :
//      2024.05.23 : 이충선 : 초기 작성 

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

    // 1. 버스에 연결된 장치 스캔중 인터럽트에 의하여 방해받지 않도록 인터럽트 비활성화 
    //    I2C_INT_STOP_CONDITION, I2C_INT_ARB_LOST,I2C_INT_NO_ACK 
    I2C_disableInterrupt(base, (I2C_INT_ADDR_SLAVE|I2C_INT_STOP_CONDITION | I2C_INT_ARB_LOST | I2C_INT_NO_ACK));

    i = 0;
	// 2. 가능한 모든 슬레이브 주소를 탐지 시작
    for(probeSlaveAddress=1;probeSlaveAddress<=MAX_10_BIT_ADDRESS;probeSlaveAddress++)
    {
        // 2.1. I2C 버스 상태를 확인 (checkBusStatus() 사용), BUSY 또는 NOT_READY 이면 모든 탐지를 정지한다.
        busstatus = checkBusStatus(base);
        if(busstatus != 0U)
        {
           status = busstatus;
           break;
        }
	    else 
	    {
	    	// 2.2 버스가 BUSY 또는 NOT_READY 가 아닌 정상 상태일 때는  2.2.1 ~ 2.2.6 을 수행한다.
			// 2.2.1 마스터 송신 모드 및 반복 모드를 설정
	        I2C_setConfig(base, (I2C_MASTER_SEND_MODE | I2C_REPEAT_MODE));

	        // 2.2.2 프로브 주소가 127(7비트 주소의 최대값)을 초과하는 경우 10비트 주소 모드로 설정
	        if(probeSlaveAddress > MAX_7_BIT_ADDRESS)
	        {
	            //10-bit addressing
	            I2C_setAddressMode(base, I2C_ADDR_MODE_10BITS);
	        }

	        // 2.2.3 슬레이브 주소 설정 
	        I2C_setSlaveAddress(base, probeSlaveAddress);


			// 2.2.4 I2C 대기 
	        I2C_wait(base);

			// 2.2.5 슬레이브 주소에 대해 NACK 상태를 확인, 응답이 있으면 pAvailableI2C_slaves에 주소 저장 
	        uint16_t I2CStatus = I2C_getStatus(base);
	        if((I2CStatus & I2C_STS_NO_ACK) == 0U)
	        {
	            pAvailableI2C_slaves[i++] = probeSlaveAddress;
	        }
			
			// 2.2.6 슬레이브 상태들을  클리어  : I2C_INT_STOP_CONDITION, I2C_INT_ARB_LOST,I2C_INT_NO_ACK , I2C_STS_REG_ACCESS_RDY
	        I2C_clearStatus(base,I2C_STS_NO_ACK|I2C_STS_ARB_LOST|I2C_STS_REG_ACCESS_RDY|I2C_STS_STOP_CONDITION);
	    }
    }

	// 4. 슬레이브 주소 탐지가 완료인지 확인
    if(status == 0U)
    {
		// 4.1 마스터 송신 모드로 설정 
        I2C_setConfig(base, (I2C_MASTER_SEND_MODE));
		// 4.2 프로브 주소 7비트로 설정  
        I2C_setAddressMode(base, I2C_ADDR_MODE_7BITS); 
		// 4.3 인터럽트 활성화 : I2C_INT_ADDR_SLAVE, I2C_INT_STOP_CONDITION, I2C_INT_ARB_LOST,I2C_INT_NO_ACK 
		I2C_enableInterrupt(base, (I2C_INT_ADDR_SLAVE|I2C_INT_STOP_CONDITION | I2C_INT_ARB_LOST | I2C_INT_NO_ACK));

		// 4.4 상태를 SUCCESS(0x0000)로 설정 
        status = SUCCESS;
    }
    else
    {
    	// 4.5 현재 버스 상태를 설정 
        status = busstatus;
    }

	// 5. 현재 상태를 전송, SUCCESS (0x0000), 
    return status;
}
#endif

// 기능 : I2C 마스터로서 슬레이브 데이터를 전송하기 위한 초기 설정 함수 
//      
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
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
	// 1. I2C 버스 상태를 확인, 0이 아니면 status에 오류 저장  
    busstatus = checkBusStatus(base);
    if(busstatus != 0U)
    {
        status = busstatus;
    }
    else
    {
    	//1.1 I2C 버스 상태가 1이면 1.1.1 ~ 1.1.9를 수행한다.
    	//1.1.1 FIFO 비활성화
        I2C_disableFIFO(base);

		// 1.1.2 I2C를 마스터 송신 모드로 설정
        I2C_setConfig(base, (I2C_MASTER_SEND_MODE));
		// 1.1.3 슬레이브 주소가 127(7비트 주소의 최대값)을 초과하는 경우 10비트 주소 모드로 설정
        if((I2C_Params->SlaveAddr) > MAX_7_BIT_ADDRESS)
        {
            //10-bit addressing
            I2C_setAddressMode(base, I2C_ADDR_MODE_10BITS);
        }

        // 1.1.4 슬레이브 주소 설정 
        I2C_setSlaveAddress(base, I2C_Params->SlaveAddr);

		// 1.1.5 전송 할 데이터 바이트 수 설정
        I2C_setDataCount(base, (I2C_Params->NumOfAddrBytes));

		// 1.1.6 FIFO 활성화
        I2C_enableFIFO(base);

		// 1.1.7 제어 주소를 가져와서 하위 24비트만 남기고 상위 8비트에 전송할 데이터 바이트 수를 저장
        uint32_t temp = *(I2C_Params->pControlAddr);
        temp = temp & 0x00FFFFFFU;
        temp |= (uint32_t)(I2C_Params->NumOfDataBytes)<<24U;

		// 1.1.8 설정한 제어 주소와 데이터 바이트 수를 하위 바이트부터 상위 바이트 순으로 FIFO에 전송
		int16_t i;
		for(i=(int16_t)I2C_Params->NumOfAddrBytes-1;i>=0;i--)
        {
           I2C_putData(base, (uint16_t)(temp >> ((uint16_t)i*8U)) & 0x00FFU);
        }

		// 1.1.9 I2C 버스에서 시작 조건(Start Condition)을 전송하여 슬레이브와의 통신
        I2C_sendStartCondition(base);

        status = SUCCESS;
    }

	// 2. 전송 상태를 리턴
    return status;
}
#endif

// 기능 :  I2C 마스터 모드에서 슬레이브에 데이터를 송신하는 함수 
//      
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 

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

	// 1. 송신 데이터 준비 
    I2C_Params->numofSixteenByte  = (I2C_Params->NumOfDataBytes) / I2C_FIFO_LEVEL;
    I2C_Params->remainingBytes    = (I2C_Params->NumOfDataBytes) % I2C_FIFO_LEVEL;

	// 2. 송신할 데이터의 크기가 최대 버퍼 크기 이하인지 확인
    if(I2C_Params->NumOfDataBytes <= MAX_BUFFER_SIZE)
    {
    	// 2.1 조건이 맞으면 상태를 SUCCESS 설정 
        status = SUCCESS;
    }
    else
    {
		// 2.2 송신할 데이터의 크기가 최대 버퍼 크기를 초과하는 경우 FIFO를 활성화
        I2C_enableFIFO(base);

		// 2.3 슬레이브 주소 및 제어 바이트 송신, 정상인 경우 0 응답. 
        i2cstatus = I2C_TransmitSlaveAddress_ControlBytes(I2C_Params);

		// 2.4 슬레이브 주소와 제어 바이트 송신이 성공인진 확인
        if(i2cstatus == 0U)
        {
        	// 2.4.1  송신할 데이터와 주소의 총 바이트 수를 설정합
            I2C_setDataCount(base, (I2C_Params->NumOfAddrBytes + I2C_Params->NumOfDataBytes));

			// 2.4.2 FIFO 인터럽트 레벨 설정
            I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, I2C_FIFO_RXFULL);

			// 2.4.3 인터럽트 활성화 
            I2C_enableInterrupt(base, (I2C_INT_STOP_CONDITION | I2C_INT_ARB_LOST | I2C_INT_NO_ACK));

			// 2.4.4 송신 FIFO 인터럽트 활성화
            I2C_enableInterrupt(base, I2C_INT_TXFF);

			// 2.4.4 송신 FIFO 인터럽트 상태를 초기화			
            I2C_clearInterruptStatus(base, I2C_INT_TXFF);

			//2.4.5 상태를 SUCCESS로 저장
            status = SUCCESS;
        }
        else
        {	
        	//2.4.5  슬레이브 주소와 제어 바이트 송신이 실패면 에러 상태를 저장 
            status = i2cstatus;
        }
    }

	//3. 상태 리턴 
    return status;
}
#endif

// 기능 : I2C 마스터 모드에서 슬레이브 장치로부터 데이터를 수신
//      
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
uint16_t I2C_MasterReceiver(struct I2CHandle *I2C_Params)
{
    uint16_t status;
    uint32_t base = I2C_Params->base;

	// 1. 송신 데이터 준비 
    I2C_Params->numofSixteenByte  = (I2C_Params->NumOfDataBytes) / I2C_FIFO_LEVEL;
    I2C_Params->remainingBytes    = (I2C_Params->NumOfDataBytes) % I2C_FIFO_LEVEL;

	// 2. 인터럽트 비활성화
    I2C_disableInterrupt(base, I2C_INT_TXFF|I2C_INT_RXFF);

	// 3. 인터럽트 상태 초기화
    I2C_clearInterruptStatus(base, (I2C_INT_REG_ACCESS_RDY|I2C_INT_TXFF|I2C_INT_RXFF));

	// 4. 인터럽트 활성화 (레지스터 접근 준비) 
    I2C_enableInterrupt(base, I2C_INT_REG_ACCESS_RDY);

	// 5. 슬레이브 주소 및 제어 바이트 송신
    status = I2C_TransmitSlaveAddress_ControlBytes(I2C_Params);

	// 6. 시간 지연 
    SysCtl_delay(50); //Adding delay to correctly read I2C bus status

    if(status != 0U)
    {
        return status;
    }

    return SUCCESS;
}


// 기능 : I2C 버스의 BUSY 및 NOT_READY 상태를 확인하는 함수 
//      
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
static uint16_t checkBusStatus(uint32_t base)
{

    uint16_t done = SUCCESS;

	// 1. 버스가 사용 중인 경우, status를 ERROR_BUS_BUSY로 설정
    if(I2C_isBusBusy(base))
    {
        done = ERROR_BUS_BUSY;
    }
	// 2. I2C 정지조건 상태 확인,  정지 조건이 준비되지 않았다면, status를 ERROR_STOP_NOT_READY 설정 
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
// 기능 : I2C 통신에서 NACK(No Acknowledge) 신호를 처리하는 함수 
//      
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
uint16_t handleNACK(uint32_t base)
{
    uint16_t status;

	// 1. NACK 상태 확인
    if((I2C_getStatus(base) & I2C_STS_NO_ACK) != 0U)
    {
    	//1.1 NACK 상태 초기화
        I2C_clearStatus(base, I2C_STS_NO_ACK);
		//1.2 FIFO 비활성화
        I2C_disableFIFO(base);
		// 1.3 정지 조건 전송
		I2C_sendStopCondition(base);
		// 1.4 FIFO 재활성화
        I2C_enableFIFO(base);

		// 1.5 NACK이므로 status를 ERROR_NACK_RECEIVED로 설정
        status =  ERROR_NACK_RECEIVED;
    }
    else
    {
    	// 1.6 NACK 아니면 status를 SUCCESS 설정
        status = SUCCESS;
    }

	// 2. 상태 리턴 
    return status;
}
#endif

// 
// 기능 : 마스터 모드에서 수신 모드로 전환하기 함수  
//      
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
static void I2C_accessReady(uint32_t base)
{
	// 1. 인터럽트 비활성화
    I2C_disableInterrupt(base, I2C_INT_REG_ACCESS_RDY);
    I2C_disableInterrupt(base, I2C_INT_TXFF);
	// 2. FIFO 비활성화 및 재활성화
    I2C_disableFIFO(base);
    I2C_enableFIFO(base);

	// 3. 수신 모드로 설정
    I2C_setConfig(base, (I2C_MASTER_RECEIVE_MODE));

	// 4. 송신 fifo 인터럽트 상태 클리어
    I2C_clearInterruptStatus(base, I2C_INT_TXFF);
}


// 기능 : I2C 통신에서 슬레이브 모드의 송신 또는 수신 상태를 설정하는 함수 
//      
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
static void I2C_addressSave(uint32_t base)
{
	// 1. 마스터가 슬레이브로부터 데이터를 요청하는 송신 모드 인지 확인, 0 아니면 송신모드 
    if(((I2C_getStatus(base) & I2C_STS_SLAVE_DIR)) != 0U)
    {
    	// 1.1 슬레이브 모드를 송신 모드로 설정
        I2C_setConfig(base, I2C_SLAVE_SEND_MODE);
        // 1.2 TX FIFO 인터럽트를 활성화하고, RX FIFO 인터럽트를 비활성화
        I2C_enableInterrupt(base, I2C_INT_TXFF);
        I2C_disableInterrupt(base, I2C_INT_RXFF);
		// 1.3 인터럽트 상태를 클리어
		I2C_clearInterruptStatus(base, (I2C_INT_TXFF|I2C_INT_RXFF));
    }
    else
    {
    	// 1.4 슬레이브 모드를 수신 모드로 설정
        I2C_setConfig(base, I2C_SLAVE_RECEIVE_MODE);

        // 1.5 TX FIFO 인터럽트를 활성화하고, RX FIFO 인터럽트를 비활성화
        I2C_disableInterrupt(base, I2C_INT_TXFF);
        I2C_enableInterrupt(base, I2C_INT_RXFF);
		
		// 1.6 인터럽트 상태를 클리어
        I2C_clearInterruptStatus(base, (I2C_INT_TXFF|I2C_INT_RXFF));

    }
}

// 기능 :  I2C 통신의 다양한 오류 상황을 처리하는 함수 
//			I2C_INTSRC_NONE,				//!< No interrupt pending
//			I2C_INTSRC_ARB_LOST,			//!< Arbitration-lost interrupt
//			I2C_INTSRC_NO_ACK,				//!< NACK interrupt
//			I2C_INTSRC_REG_ACCESS_RDY,		//!< Register-access-ready interrupt
//			I2C_INTSRC_RX_DATA_RDY, 		//!< Receive-data-ready interrupt
//			I2C_INTSRC_TX_DATA_RDY, 		//!< Transmit-data-ready interrupt
//			I2C_INTSRC_STOP_CONDITION,		//!< Stop condition detected
//			I2C_INTSRC_ADDR_TARGET			 //!< Addressed as target interrupt
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
static void handleI2C_ErrorCondition(struct I2CHandle *I2C_Params)
{

    uint32_t base = I2C_Params->base;
    uint16_t status;

	// 1. 인터럽트 소스 확인
    I2C_InterruptSource intSource = I2C_getInterruptSource(base);

    switch (intSource)
    {

    	// 1.1 Arbitration Lost 상황 처리
        case I2C_INTSRC_ARB_LOST:
            //Report Arbitration lost failure
            status = ERROR_ARBITRATION_LOST;
            break;

		// 1.2 NACK(ACK 수신 실패) 처리 
        case I2C_INTSRC_NO_ACK:
            //1.2.1 NACK(ACK 수신 실패) 플래그를 클리어하고, STOP 조건을 생성
            I2C_clearStatus(base, I2C_STS_NO_ACK);
            I2C_sendStopCondition(base);
            status = ERROR_NACK_RECEIVED;
            break;

		// 1.3  레지스터 접근 준비 인터럽트 
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

			// 1.3.3 데이터 수를 설정한 후 시작 및 정지 조건을 생성
            I2C_setDataCount(base, I2C_Params->NumOfDataBytes);
            I2C_sendStartCondition(base);
            I2C_sendStopCondition(base);

            break;
			
		// 1.4 수신 데이터 준비 
        case I2C_INTSRC_RX_DATA_RDY:
            break;
			
		// 1.5 송신 데이터 준비 
        case I2C_INTSRC_TX_DATA_RDY:
            break;

		// 1.6 STOP 조건 발생 인터럽트  
        case I2C_INTSRC_STOP_CONDITION:
			// 1.6.1 송신 및 수신 인터럽트를 비활성화, 메시지 버퍼 포인터를 설정
            I2C_disableInterrupt(base, (I2C_INT_TXFF | I2C_INT_RXFF));
            I2C_Params->pTX_MsgBuffer   = TX_MsgBuffer;
            I2C_Params->pRX_MsgBuffer   = RX_MsgBuffer;
            break;

		// 1.7 슬레이브 주소 요청 인터럽트 
        case I2C_INTSRC_ADDR_SLAVE:
            // 1.7.1 송수신 FIFO 인터럽트 레벨을 설정하고, 슬레이브 주소를 저장
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

// 기능 : I2C 인터럽트 상태에 따라 데이터 송신 또는 수신을 처리하는 함수 
//      
// 이력 :
//      2024.05.23 : 이충순 : 초기 작성 
static void Write_Read_TX_RX_FIFO(struct I2CHandle *I2C_Params)
{
    int16_t i;
    uint32_t base = I2C_Params->base;
    uint16_t numofSixteenByte = I2C_Params->numofSixteenByte;
    uint16_t remainingBytes  = I2C_Params->remainingBytes;

	// 1. 현재 I2C 인터럽트 상태와 TX FIFO 인터럽트 활성화 상태를 확인
    struct I2CHandle *currentPtr = I2C_Params->currentHandlePtr;
    uint32_t intSource = (uint32_t)I2C_getInterruptStatus(base);
    uint32_t txFIFOinterruptenabled = HWREGH(base + I2C_O_FFTX) & I2C_FFTX_TXFFIENA;


    // 2. 수신 FIFO 인터럽트이고, 제어 주소(pControlAddr)가 0 인지 확인 
    if(((intSource & I2C_INT_RXFF) != 0U) && ((I2C_Params->pControlAddr) == NULL))
    {
        uint32_t Addr_Ctrl = 0U;
        int16_t NumRX_Bytes = I2C_getRxFIFOStatus(base);

		// 2.1. Addr_Ctrl에 읽은 데이터를 저장하여 pControlAddr 및 데이터 바이트 수를 설정
        for(i=NumRX_Bytes-1;i>=0;i--)
        {
            Addr_Ctrl |= (uint32_t)(I2C_getData(base))<<(i*8U);
        }

		// 2.2 업데이트된 제어 주소와 데이터 바이트 수를 설정합니다.
        I2C_Params->pControlAddr       = (uint32_t *)Addr_Ctrl;
        I2C_Params->NumOfDataBytes     = Addr_Ctrl >> 24U;
        I2C_Params->numofSixteenByte   = I2C_Params->NumOfDataBytes / I2C_FIFO_LEVEL;
        I2C_Params->remainingBytes     = I2C_Params->NumOfDataBytes % I2C_FIFO_LEVEL;

        numofSixteenByte = I2C_Params->numofSixteenByte;
        remainingBytes   = I2C_Params->remainingBytes;
		
		// 2.3. 송신할 데이터의 수에 따라 FIFO 인터럽트 레벨을 설정
        if(numofSixteenByte != 0U)
        {
            I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, I2C_FIFO_RXFULL);
        }
        else
        {
            I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, (I2C_RxFIFOLevel)remainingBytes);
        }
		// 2.4 수신 FIFO 인터럽트 클리어 
        I2C_clearInterruptStatus(base,(I2C_INT_RXFF));
    }

    else
    {
    	// 2.5 송신 FIFO(TXFF) 또는 수신 FIFO(RXFF) 인터럽트가 활성화되어 있는지 확인
        if(((intSource & I2C_INT_TXFF) != 0U) || ((intSource & I2C_INT_RXFF) != 0U))
        {
          //2.5.1 송수신 데이터가 남은 있는지 확인  
          if((remainingBytes != 0U) && (numofSixteenByte == 0U))
          {
          	// 2.5.1.1 송신 인터럽트가 활성화된 경우 남아 있는 바이트 수만큼 데이터를 송신
          	// 		   수신 인터럽트가 활성화된 경우 수신된 데이터를 버퍼에 저장
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

          //2.5.2 numofSixteenByte가 0보다 크면 16바이트 단위로 송수신을 처리
          if(numofSixteenByte != 0U)
          {

			// 2.5.2.1 송신 인터럽트가 활성화된 경우 남아 있는 바이트 수만큼 데이터를 송신 후 numofSixteenByte 감소 시킴 
            if(((intSource & I2C_INT_TXFF) !=0U) && (txFIFOinterruptenabled !=0U))
            {
                for(i=0;i<I2C_FIFO_TXFULL;i++)
                {
                   I2C_putData(base, *(currentPtr->pTX_MsgBuffer++));
                }
                numofSixteenByte--;
            }
			
			// 2.5.2.2 수신 인터럽트가 활성화된 경우 수신된 데이터를 버퍼에 저장 후 numofSixteenByte 감소 시킴
            if((intSource & I2C_INT_RXFF) != 0U)
            {
                for(i=0;i<I2C_FIFO_RXFULL;i++)
                {
                    *(currentPtr->pRX_MsgBuffer++) = I2C_getData(base);
                }
                numofSixteenByte--;
            }
          }

          // 2.5.3 numofSixteenByte가 0이고, 남은 바이트가 있는지 확인 
          if((numofSixteenByte == 0U) && (remainingBytes != 0U))
          {
          	// 2.5.3.1 RX FIFO 레벨을 남은 바이트 수로 설정
            I2C_setFIFOInterruptLevel(base, I2C_FIFO_TXEMPTY, (I2C_RxFIFOLevel)remainingBytes);

          }

          // 2.5.4 모든 바이트가 전송 되었는지 확인 
          if((remainingBytes == 0U) && (numofSixteenByte == 0U))
          {
          	// 2.5.4.1 I2C 마스터 모드일 경우 STOP 조건을 전송하여 통신을 종료
			if(HWREGH(I2C_Params->base + I2C_O_MDR) & I2C_MDR_MST)
			{
			  I2C_sendStopCondition(base);
			}
          }

		  // 2.5.5 인터럽트 상태 플래그를 초기화
          I2C_clearInterruptStatus(base,(I2C_INT_TXFF | I2C_INT_RXFF));
        }

		// 2.5.6 numofSixteenByte 및 remainingBytes 값을 업데이트하여 다음 송수신 시 사용
        I2C_Params->numofSixteenByte = numofSixteenByte;
        I2C_Params->remainingBytes   = remainingBytes;
    }
}
#endif
