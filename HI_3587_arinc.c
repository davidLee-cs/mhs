/*
 * HI_3587.c
 *
 * ���� : HI-3587 ������ �����, ���� ���
 * ������� :
 * �̷� :
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 *
 */

#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
#include "HI_3587_arinc.h"  // Include the demo board declarations
#include "mhs_project.h"
#include "board.h"

static uint16_t txrx8bits (uint16_t txbyte);

#define SLOW


#if 0
static uint16_t LabelArray[32] = {
    // ---------------------------------------------------------------
//     [0]       [1]     [2]     [3]      [4]    [5]     [6]     [7]
    // 000-007 008-015 016-023 024-031 032-039 040-047 048-055 056-063
    0x0B,       0x0C,   0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
    // ---------------------------------------------------------------
    // [8]       [9]     [10]    [11]    [12]   [13]    [14]     [15]
    // 064-071 072-079 080-087 088-095 096-103 104-111 112-119 120-127
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ---------------------------------------------------------------
    // [16]      [17]    [18]    [19]    [20]     [21]   [22]    [23]
    // 128-135 136-143 144-151 152-159 160-167 168-175 176-183 184-191
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ---------------------------------------------------------------
    // [24]      [25]   [26]     [27]    [28]    [29]    [30]    [31]
    // 192-119 200-207 208-215 216-223 224-231 232-239 240-247 248-255
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ---------------------------------------------------------------
    // label addresses 0,1,3,8,16,32,64 and 128 are enabled
};

// HI3587 ������ Tx���� �ʱ�ȭ
void InitTxArray (void) {
    int16_t i;
    uint32_t j = 0;
    ControlReg = TXAUTOSTART;
	
    for (i=0; i<32; i++) {
        j++;
        if((j&0xFU) == 0xAU)
        {
            j = j+6U;
        }

        TxBusWord[i] = (j<<24)+(j<<16)+(j<<8)+j;
    }
} /* InitTxArray() */

void InitTxArrayRolling(uint16_t x) {
    uint16_t i;
    uint32_t j = 0x80000000U;
    for (i=0U; i<32U; i++) {
        TxBusWord[i] = j >> i;
    }

    if (x == 0x0U){
        for (i=0U; i<32U; i++){
        TxBusWord[i] = ~TxBusWord[i];
        }
    }
} /* InitTxArrayRolling() */


/* ------------------------------------------------------------------
/ Clear Processor TxArray Function
/ ------------------------------------------------------------------
*/
void ClearTxArray (void) {
    uint16_t i;
    for (i=0U; i<32U; i++) {
        TxBusWord[i] = 0U;
    }
}

#endif

/*
��ɼ���
 ������ ���� 8bit OP code �� �����ϴ� �Լ�

�Էº���
uint16_t txbyte : OP �ڵ� �Է�,
*/
void txOpCode (uint16_t txbyte) {
    uint16_t wrBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	//1. SPI enable ��Ŵ. 
    GPIO_writePin(SPI_CS, 0);

	//2. ������ OP Code���� wrBuf���ۿ� ���� �� spi ������ ���� 
    wrBuf[0] = txbyte;
    SPI_writeDataNonBlocking(mySPI0_BASE, wrBuf[0]);
	
    //3. ������ ���� ��  ����  ������ �ö� ���� ��� 
    (void)SPI_readDataBlockingNonFIFO(mySPI0_BASE);

	// 4. SPI disable ��Ŵ.
    GPIO_writePin(SPI_CS, 1);

} /* txOpCode */


/*
��ɼ���
 SPI 8bit ������ ���� �� ���� �Լ�

����� ����
uint16_t txbyte : 8bit ������ ������ �Է�
return : ���� ������ ����
*/
static uint16_t txrx8bits (uint16_t txbyte) {
    uint16_t wrBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t wrxBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	// 1. ������ ������ ���߿� ���� �� spi ������ ����  
    wrBuf[0] = txbyte << 8;
    SPI_writeDataNonBlocking(mySPI0_BASE, wrBuf[0]);

	// 2. ���� ������ �ޱ� ������ ��� 
	wrxBuf[0] = SPI_readDataBlockingNonFIFO(mySPI0_BASE);

	// 3. ���� ������ ���� 
    return wrxBuf[0];
} /* txrx8bits */

/*
��ɼ���
 HI-3587 ��Ʈ�� �������� �����ϴ�  �Լ�
�Է� ��������
uint16_t ControlReg = TXAUTOSTART;  HI3587 �������� �ʱⰪ ���� 0x2000
*/
void writeControlReg (void) {

    uint16_t ControlReg = TXAUTOSTART;   // HI3587 �������� �ʱⰪ ����

	// 1. spi enable ��Ŵ. 
    GPIO_writePin(SPI_CS, 0);

#ifdef SLOW
    //2. HI 3587 ��Ʈ�� �������Ϳ� ���⸦ ���� OP code 0x10 ���� 
    (void)txrx8bits(0x10); 
    //3. ���� ���� ��ȯ �Ҽ� �ֵ��� ��Ʈ�� �������� ������ 16bit �� upper 8bit ����  
    uint16_t upCr = ControlReg >> 8;
    (void)txrx8bits(upCr); // upper CR
    //4. ���� ���� ��ȯ �Ҽ� �ֵ��� ��Ʈ�� �������� ������ 16bit �� lower 8bit ����  
    uint16_t lowerCr = ControlReg & 0xFFU;
    (void)txrx8bits(lowerCr); // lower CR
    
#else
    // fastr option (ends at "====" line)
//    dummy = SPI0SR; // clear SPI status register
    wrBuf[0] = 0x10; // writing opcode to Data Reg starts SPI xfer
    SPI_writeDataNonBlocking(mySPI0_BASE, wrBuf[0]);
    // Block until data is received and then return it
    wrxBuf[0] = SPI_readDataBlockingNonFIFO(mySPI0_BASE);

    //--------------------------------------------
//    wrBuf[0] = 0x10; // writing opcode to Data Reg starts SPI xfer
    SPI_writeDataNonBlocking(mySPI0_BASE, (int16_t)(wrControlReg>>8));
    // Block until data is received and then return it
    wrxBuf[1] = SPI_readDataBlockingNonFIFO(mySPI0_BASE);

    //--------------------------------------------
    SPI_writeDataNonBlocking(mySPI0_BASE, (int16_t)(wrControlReg & 0xFF));
    // Block until data is received and then return it
    wrxBuf[2] = SPI_readDataBlockingNonFIFO(mySPI0_BASE);
#endif
    //============================================

	//5. spi disable ��Ŵ.
    GPIO_writePin(SPI_CS, 1);

} /* writeControlReg() */


/*
��ɼ���
 HI-3587 ��Ʈ�� �������� �����͸� �д� �Լ�

��� ����
return : ��Ʈ�� �������Ϳ��� ���� ������ ����
*/
uint16_t readControlReg (void) {
    uint16_t rxword;
    uint16_t wrBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t wrxBuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    uint16_t txbyte = 0x0B;
	
	// 1. spi enable ��Ŵ. 
    GPIO_writePin(SPI_CS, 0);

    //2. HI 3587 ��Ʈ�� �������Ϳ� �б⸦ ���� OP code 0x0B ���� 
    wrBuf[0] = txbyte << 8;; // writing opcode to Data Reg starts SPI xfer
    SPI_writeDataNonBlocking(mySPI0_BASE, wrBuf[0]);
    wrxBuf[0] = SPI_readDataBlockingNonFIFO(mySPI0_BASE);

    // 3. ��Ʈ�� �������� ������ �б⸦ ���� ���� 8bit ������ ���� 
    wrBuf[0] = 0; // writing opcode to Data Reg starts SPI xfer
    SPI_writeDataNonBlocking(mySPI0_BASE, wrBuf[0]);
    // Block until data is received and then return it
    wrxBuf[1] = SPI_readDataBlockingNonFIFO(mySPI0_BASE);

    // 4. ��Ʈ�� �������� ������ �б⸦ ���� ���� 8bit ������ ���� 
    wrBuf[0] = 0; // writing opcode to Data Reg starts SPI xfer
    SPI_writeDataNonBlocking(mySPI0_BASE, wrBuf[0]);
    // Block until data is received and then return it
    wrxBuf[2] = SPI_readDataBlockingNonFIFO(mySPI0_BASE);

	//5. spi disable ��Ŵ.
    GPIO_writePin(SPI_CS, 1);

    // 6. ���� �����͸� 16bit�� ���� ��Ŵ.
    rxword = (wrxBuf[1]<<8) | wrxBuf[2] ;
    return rxword ;
} /* readControlReg() */


/*
��ɼ���
 HI-3587 �� ���� �������� �д� �Լ�

��� ����
return : ���� �������Ϳ��� ���� ������ ����
*/
uint16_t readStatusReg (void) {
    uint16_t rxdata;

	// 1. spi enable ��Ŵ. 
    GPIO_writePin(SPI_CS, 0);
    // send op code (ignore returned data byte)
	
    //2. HI 3587 ���� �������� �б⸦ ���� OP code 0x0A ���� 
    rxdata = txrx8bits(0x0A);
    // send dummy data / receive Status Reg byte
    rxdata = txrx8bits(0x00);

	//3. spi disable ��Ŵ.
    GPIO_writePin(SPI_CS, 1);

	//4. ���� �������� �� ���� 
    return rxdata;
} /* readStatusReg() */



/*
��ɼ���
  arinc429 ��� �������ݷ� SFI�� �����͸� �����ϱ� ���� HI-3587 TxFIFO ���� �Լ�

�Է� ����
uint16_t words_to_send : SFI �� ������ ������(32bit) ������ Ƚ��

�Է� ��������
uint32_t TxBusWord[32];     HI-3587�� ������(32bit) ������ ���� spi TX ����
*/
void writeTxFIFO (uint16_t words_to_send) {
    uint16_t i;
    uint16_t txbyte;
    uint16_t txbypt1;

	// 1. ���� �� ������ words_to_send ����ŭ ���� 
    for (i=0; i<words_to_send; i++) {

		// 1.1 spi enable ��Ŵ. 
        GPIO_writePin(SPI_CS, 0);

        // 1.2 op code 0x0E (Transmitter FIFO ���) ����
        (void)txrx8bits(0x0E);
	
		// 1.3 32bit ������ ������ �� ���� 8bit ������ ����  
        txbypt1 = (uint16_t)(TxBusWord[i]>>24);
        (void)txrx8bits(txbypt1);
		// 1.4 32bit ������ ������ �� ���� 8bit ������ ����  
        txbyte = (uint16_t)(TxBusWord[i]>>16);
        txbypt1  = txbyte & 0xFFU;
        (void)txrx8bits(txbypt1);
		// 1.5 32bit ������ ������ �� ���� 8bit ������ ����  
        txbyte = (uint16_t)(TxBusWord[i]>>8);
        txbypt1  = txbyte & 0xFFU;
        (void)txrx8bits(txbypt1);
		// 1.6 32bit ������ ������ �� ���� 8bit ������ ����  
        txbypt1 = (uint16_t)(TxBusWord[i] & 0xFFU);
        (void)txrx8bits(txbypt1);
		// 1.7 spi disable ��Ŵ.
        GPIO_writePin(SPI_CS, 1);

    }
} /* writeTxFIFO() */



#if 0
// HI-358x ACLK ���� �����ϴ� �Լ� 
void writeACLKdiv (uint16_t divisor) {
	// 1. spi enable ��Ŵ. 
    GPIO_writePin(SPI_CS, 0);

	// 2. ACLK ���ָ� �ϱ� ���� OP code 0x07 ����  
    (void)txrx8bits(0x07);
	
    // 3. ���� ������ ���� 
    (void)txrx8bits(divisor);

	//4. spi disable ��Ŵ.
    GPIO_writePin(SPI_CS, 1);

}


// HI-358x ACLK ���� �������� �д� �Լ� 
uint16_t readACLKdiv (void) {
    uint16_t rxdata;
	
	// 1. spi enable ��Ŵ. 
    GPIO_writePin(SPI_CS, 0);

	// 2. ACLK ���� �������� �б� ���� OP code 0x0C ����  
    rxdata = txrx8bits(0x0C);

	// 3. ���� ������ �б� ���� ���� ������  ���� 
    rxdata = txrx8bits(0x00);
	
	//4. spi disable ��Ŵ.
    GPIO_writePin(SPI_CS, 1);

	//5. ACLK ���� ������ ���� 
    return rxdata;
} /* readACLKdiv() */

/* ------------------------------------------------------------------
/ Read HI-358x next RxFIFO Word Function
/ ------------------------------------------------------------------
*/

uint32_t read1RxFIFO (void)
{
    uint32_t j, rxdata; // long = 32 bits

    GPIO_writePin(SPI_CS, 0);

    // send op code (ignore returned data byte)
    rxdata = txrx8bits(0x08);
    // send dummy data / receive and left-justify most signif. byte
    j = txrx8bits(0x00);
    rxdata = ( j << 24);
    // send dummy data / receive, left-shift and OR next byte
    j = txrx8bits(0x00);
    rxdata = rxdata | (j << 16);
    // send dummy data / receive, left-shift and OR next byte
    j = txrx8bits(0x00);
    rxdata = rxdata | (j << 8);
    // send dummy data / receive and OR the least signif. byte
    j = txrx8bits(0x00);
    rxdata = rxdata | j;

    GPIO_writePin(SPI_CS, 1);

    return rxdata;
} /* read1RxFIFO() */


/* ------------------------------------------------------------------
/ Reset all HI-358x ARINC label selections
/ ------------------------------------------------------------------
 */
void resetAllLabels (void ) {
    uint16_t i;
    // reset all label bits in HI-358x device
    txOpCode(2);
    // reset all bits all 32-bytes of global LabelArray[]
    for (i=0U; i<32U; i++) {
        LabelArray[i] = 0U;
    }
} /* resetAllLabels */

/* ------------------------------------------------------------------
/ Set all HI-358x ARINC label selections
/ ------------------------------------------------------------------
*/
void setAllLabels (void ) {
    uint16_t i;
    // set all label bits in HI-358x device
    txOpCode(3);
    // set all bits all 32-bytes of global LabelArray[]
    for (i=0U; i<32U; i++) {
        LabelArray[i] = 0xFFU;
    }
} /* setAllLabels */
/* ------------------------------------------------------------------
/ Copy HI-358x ARINC label selections from LabelArray[] to HI-358x
/ ------------------------------------------------------------------
*/
void copyAllLabels (void ) {
    int16_t i;

    GPIO_writePin(SPI_CS, 0);

    // send op code (ignore returned data byte)
    (void)txrx8bits(0x06);
    // send 32 bytes of ARINC label data
    for (i=31; i>=0; i--) {
        // send 1 byte of label data, ignore returned data byte
        (void)txrx8bits(LabelArray[i]);
    }

    GPIO_writePin(SPI_CS, 1);

} /* copyAllLabels */
/* ------------------------------------------------------------------
/ Verify match: HI-358x ARINC label selections to LabelArray[]
/ ------------------------------------------------------------------
*/

unsigned short checkAllLabels (void) {
    unsigned char rxbyte;
    int16_t i;
    uint16_t j;

    GPIO_writePin(SPI_CS, 0);

    // send op code (ignore returned data byte)
    rxbyte = txrx8bits(0x0D,1);
    j = 0xffff;
    // starting at high end, read 8-bit increments of ARINC label data
    for (i=31; i>=0; i--) {
        // send dummy data, read 1 byte of label data
        rxbyte = txrx8bits(0,1);
        // check for mismatch
        if (rxbyte != LabelArray[i]) {

            GPIO_writePin(SPI_CS, 1);
            // return the failing array pointer value
            return (i);
        }
    }

    GPIO_writePin(SPI_CS, 1);

    // return the "no fail" value
    return j;
} /* checkAllLabels */
#endif
