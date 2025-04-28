/*
 * csu_mhs_Status_trans.h
 *
 *  Created on: 2024. 10. 30.
 *      Author: Derac SON
 */

#ifndef CSU_MHS_STATUS_TRANS_H_
#define CSU_MHS_STATUS_TRANS_H_


#include "Typedef.h"

struct _arinc429_word
{
    uint16_t parity;     // odd : 0

    uint16_t ssm;        // b00 normal,           b01 no computed data
                        // b10 functional test,  b11 not used

    uint16_t dig_4;      // 100's degrees
    uint16_t dig_3;      // 10's degress
    uint16_t dig_2;      // 1's degress
    uint16_t dig_1;      // 0.1's degress

    uint16_t pad;        // b0000 normal mode
                        // b0001
    uint16_t label;      // octal 014 : 00 001 100

};


struct _arinc429_error
{
    uint16_t parity;     // odd : 0

    uint16_t ssm;        // b00 normal,           b01 no computed data
                        // b10 functional test,  b11 not used

    uint16_t checksum;

    uint16_t fluxError;
    uint16_t tempError;      //
    uint16_t voltageError;   //
    uint16_t eepromError;    //

    uint16_t pad;        // b000 normal mode

    uint16_t label;      // octal 331 : 00 001 100

};


void mhs_status_trans(uint16_t paddingbit, uint16_t StatusMatrix);

extern struct _arinc429_error arinc429_error;
extern uint32_t TxBusWord[32];


#endif /* CSU_MHS_STATUS_TRANS_H_ */
