/*
 * csu_Cbit.c
 *
 * ��� : �ڱ��尪�� �����Ͽ� ���� ������ �˻��ϴ� ��� 
 * ������� : CBIT CSU (D-MHS-SFR-012)
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */

#include <stdbool.h> 
#include "mhs_project.h"
 
static uint16_t mag_acc_check(void);
static uint16_t temp_check(void);
static uint16_t volt_check(void);

#define VOLT_28_MORE        (32.0L)
#define VOLT_28_LESS        (12.0L)

#define VOLT_5_MORE         (5.0L*1.05L)
#define VOLT_5_LESS         (5.0L*0.95L)

#define VOLT_3V3_MORE       (3.3L*1.05L)
#define VOLT_3V3_LESS       (3.3L*0.95L)

#define VOLT_1V2_MORE       (1.2L*1.05L)
#define VOLT_1V2_LESS       (1.2L*0.95L)


// ��ɼ���
// CBIT ���� Ȯ���ϴ� �Լ� 
//
// ��º���
// return �� : CBIT ���� �߻� ��Ȳ (1: ���� �߻�, 0 : ���� �۵�)
uint16_t checkCbit(void)
{
    uint16_t error = 0U;

	// 1. �ڱ��� ���� �˻� �� ���ϰ��� Ȯ�� 
	// ���ϰ� : 1 (error), 0 (No error)
    if(mag_acc_check() != 0U)
    {
        error = 1U;
        arinc429_error.fluxError = 1U;
    }
    else
    {
        arinc429_error.fluxError = 0U;
    }

    if(temp_check() != 0U)
    {
        error = 1U;
        arinc429_error.tempError = 1U;
    }
    else
    {
        arinc429_error.tempError = 0U;
    }

    if(volt_check() != 0U)
    {
        error = 1U;
        arinc429_error.voltageError = 1U;
    }
    else
    {
        arinc429_error.voltageError = 0U;
    }

    return error;

}


// ��ɼ���
// �ڱ��� ���� �˻��ϴ� �Լ� �� x,y,z �ڱ��� ���� -7000 ~ +7000 gauss ������ ����� ���� �߻�
//  ���ϰ� : 1 : error,  0: No error
//
// ��º���
// return �� : CBIT ���� �߻� ��Ȳ (1: ���� �߻�, 0 : ���� �۵�)
static uint16_t mag_acc_check(void)
{
    uint16_t error = 0U;

	// 1. ���͵� �ڱ��� Box,Boy,Boz ���� ���� -70 uT ~ +70 uT ���� �ȿ� ������ 0, ����� 1�� error �� ���� �ϰ� error�� ��ȯ
    if((Box > 7000.0L) || (Box < -7000.0L))
    {
        error = 1U;
    }

    if((Boy > 7000.0L) || (Boy < -7000.0L))
    {
        error = 1U;
    }

    if((Boz > 7000.0L) || (Boz < -7000.0L))
    {
        error = 1U;
    }
	// 2. ���� ���� ��ȭ 
    return error;
}

// ��ɼ���
// �µ� ���� �˻��ϴ� �Լ� ���� -40 ~ +125 �� ������ ����� ���� �߻�
//  ���ϰ� : 1 : error,  0: No error
//
// ��º���
// return �� :  ���� �߻� ��Ȳ (1: ���� �߻�, 0 : ���� �۵�)
static uint16_t temp_check(void)
{
    uint16_t error = 0U;

    // 1. �µ����� -40 ~ +125  ���� �ȿ� ������ 0, ����� 1�� error �� ���� �ϰ� error�� ��ȯ
    if((mhsensor_sensor_Data.tempValue > 125) || (mhsensor_sensor_Data.tempValue < -40))
    {
        error = 1U;
    }

    // 2. ���� ���� ��ȭ
    return error;
}

// ��ɼ���
// ���� ���� �˻��ϴ� �Լ� �� ���� 1.2, 3.3, 5, 28V ���� -5% ~ +5% �̳� ������ ����� ���� �߻�
//  ���ϰ� : 1 : error,  0: No error
//
// ��º���
// return �� : ���� �߻� ��Ȳ (1: ���� �߻�, 0 : ���� �۵�)
static uint16_t volt_check(void)
{
    uint16_t error = 0U;

    // 1. ���͵� �ڱ��� Box,Boy,Boz ���� ���� -70 uT ~ +70 uT ���� �ȿ� ������ 0, ����� 1�� error �� ���� �ϰ� error�� ��ȯ
    if((mhsensor_sensor_Data.voltage28v > VOLT_28_MORE) || (mhsensor_sensor_Data.voltage28v < VOLT_28_LESS))
    {
        error = 1U;
    }

    if((mhsensor_sensor_Data.voltage5v > VOLT_5_MORE) || (mhsensor_sensor_Data.voltage5v < VOLT_5_LESS))
    {
        error = 1U;
    }

    if((mhsensor_sensor_Data.voltage3v3 > VOLT_3V3_MORE) || (mhsensor_sensor_Data.voltage3v3 < VOLT_3V3_LESS))
    {
        error = 1U;
    }

    if((mhsensor_sensor_Data.voltage1v2 > VOLT_1V2_MORE) || (mhsensor_sensor_Data.voltage1v2 < VOLT_1V2_LESS))
    {
        error = 1U;
    }

    // 2. ���� ���� ��ȭ
    return error;
}

