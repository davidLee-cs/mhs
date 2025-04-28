/*
 * HI_3587_arinc.h
 *
 *  Created on: 2024. 1. 11.
 *      Author: USER
 */

#ifndef HI_3587_ARINC_H_
#define HI_3587_ARINC_H_


/* include peripheral declarations */
/* define value for LED's when on and off */

#include "Typedef.h"
//#include "F2837xD_Cla_typedefs.h"


#define ON 0
#define OFF 1
/* define value for switches when up (not pressed) and down (pressed) */
#define UP 1
#define DOWN 0
/* define LED's */

/* define ARINC device status flags */


// label address
#define MH_LABEL_ADDR           (0x0C)  // Octal :  00 001 100
#define MAG_X_LABEL_ADDR        (0xC0)  // Octal :  11 000 000
#define MAG_Y_LABEL_ADDR        (0xC1)  // Octal :  11 000 001
#define MAG_Z_LABEL_ADDR        (0xC2)  // Octal :  11 000 010
#define ACCEL_X_LABEL_ADDR      (0xD9)  // Octal :  11 011 001
#define ACCEL_Y_LABEL_ADDR      (0xDA)  // Octal :  11 011 010
#define CHK_EER_LABEL_ADDR      (0xD9)  // Octal :  11 011 001
#define VERSION_LABEL_ADDR      (0xDA)  // Octal :  11 011 010


// padding mode
#define NORMAL_MODE             (0x00)
#define CAL_MODE                (0x01)

//Status Matrix  BCD
#define STATUS_BCD_NORMAL       (0x0000)
#define STATUS_NO_COMPUT_DATA   (0x0001)
#define STATUS_BCD_FW           (0x0003)

//Status Matrix  BNR
#define STATUS_BNR_NORMAL       (0x0003)
#define STATUS_BNR_FW           (0x0000)


#define PARITY_ODD              (0x0000)


// angle shift
#define PARITY                  (31)
#define SSM                     (29)
#define DIG4                    (26)
#define DIG3                    (22)
#define DIG2                    (18)
#define DIG1                    (14)
#define PAD                     (10)
#define SDI                     (10) //(8)
#define LABEL                   (0)

#define SIGN                    (28)


// flux, accel
#define PARITY                  (31)
#define SSM                     (29)
#define DATA                    (13)
#define PAD                     (10)
#define SDI                     (10) //(8)
#define LABEL                   (0)


// check sum , error bit
#define PARITY                  (31)
#define SSM                     (29)
#define DATA                    (13)
#define EEP_ERROR               (12)
#define VOLT_ERROR              (11)
#define TEMP_ERROR              (10)
#define PAD                     (10)//(8)
#define LABEL                   (0)

// version value
#define PARITY                  (31)
#define SSM                     (29)
#define VER                     (14-1)
//#define VER_3                   (26-1)
//#define VER_2                   (22-1)
//#define VER_1                   (18-1)
//#define VER_0                   (14-1)
#define PAD                     (10)//(8)
#define LABEL                   (0)



//Additional Information
//Information on the Freescale demonstration board and
//microcontroller can be found by searching the device type
//number at www.Freescale.com.
//More information on programming with CodeWarrior can be
//found at www.Freescale.com/CodeWarrior.

/* define Status Register bits */
#define RxFIFO_Empty 0x01
#define RxFIFO_HFull 0x02 // Half-Full, 16 (or more) words
#define RxFIFO_Full 0x04
#define TxFIFO_Empty 0x08
#define TxFIFO_HFull 0x10 // Half-Full, 16 (or more) words
#define TxFIFO_Full 0x20
/* define Control Register bits */
// ACTION IF BIT IS SET // ACTION IF BIT IS *NEGATED*
//------------------------ // --------------------------------
#define DIVIDE_ACLK 0x0002 // ARINC TX/RX uses undivided ACLK
#define RXSPEED_LOW 0x0001 // ARINC receive bus speed = high
#define TXSPEED_LOW 0x0400 // ARINC transmit bus speed = high
#define LABELS_ON 0x0004 // ARINC word label decoding is off
#define LBL_NOREV 0x0800 // Tx/Rx label bit order is reversed
#define RXPARITY_ON 0x0010 // receive parity off (all 32 bits = data)
#define TXPARITY_ON 0x0008 // transmit parity off (all 32 bits = data)
#define TXPAR_EVEN 0x0200 // transmit parity = odd
#define LOOPBAK_OFF 0x0020 // Tx-to-Rx loop-back is enabled
#define BUSDRV_OFF 0x1000 // ARINC bus transmit is enabled
#define TXAUTOSTART 0x2000 // Tx starts with op code 0x12
#define TXFULL_FLAG 0x4000 // TFLAG output = Tx FIFO Empty
#define RXFULL_FLAG 0x8000 // RFLAG output = Rx FIFO Empty
#define RXDECODE_ON 0x0040 // decoding of Rx ARINC bits 10:9 is off
// next 2 apply only
#define RXBIT10_HI 0x0080 // Rx ARINC bit 10 must be low
#define RXBIT9_HI 0x0100 // Rx ARINC bit 9 must be low
/* define Control Register bits */
// ACTION IF BIT IS SET // ACTION IF BIT IS *NEGATED*
//------------------------ // --------------------------------
#define DIVIDE_ACLK 0x0002 // ARINC TX/RX uses undivided ACLK
#define RXSPEED_LOW 0x0001 // ARINC receive bus speed = high
#define TXSPEED_LOW 0x0400 // ARINC transmit bus speed = high
#define LABELS_ON 0x0004 // ARINC word label decoding is off
#define LBL_NOREV 0x0800 // Tx/Rx label bit order is reversed
#define RXPARITY_ON 0x0010 // receive parity off (all 32 bits = data)
#define TXPARITY_ON 0x0008 // transmit parity off (all 32 bits = data)
#define TXPAR_EVEN 0x0200 // transmit parity = odd
#define LOOPBAK_OFF 0x0020 // Tx-to-Rx loop-back is enabled
#define BUSDRV_OFF 0x1000 // ARINC bus transmit is enabled
#define TXAUTOSTART 0x2000// Tx starts with op code 0x12
#define TXFULL_FLAG 0x4000 // TFLAG output = Tx FIFO Empty
#define RXFULL_FLAG 0x8000 // RFLAG output = Rx FIFO Empty
#define RXDECODE_ON 0x0040 // decoding of Rx ARINC bits 10:9 is off
// next 2 apply only
#define RXBIT10_HI 0x0080 // Rx ARINC bit 10 must be low
#define RXBIT9_HI 0x0100 // Rx ARINC bit 9 must be low



void txOpCode (uint16_t txbyte);
void writeControlReg (void);
uint16_t readControlReg (void) ;
uint16_t readStatusReg (void);
void writeTxFIFO (uint16_t words_to_send);

#if 0
void resetAllLabels (void );
void setAllLabels (void );
void copyAllLabels (void );
#endif


#endif /* HI_3587_ARINC_H_ */
