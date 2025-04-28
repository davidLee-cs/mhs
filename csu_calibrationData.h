
#ifndef CSU_CALIBRATIONDATA_H_
#define CSU_CALIBRATIONDATA_H_

#include "Typedef.h"


struct _mhsensor_calibration_Data
{
    float64_t Gain_Ax;    // ���ӵ� x�� �̵�
    float64_t Gain_Ay;    // ���ӵ� y�� �̵�
    float64_t Gain_Az;    // ���ӵ� z�� �̵�
    int16_t  Offset_Ax; // ���ӵ� x�� �ɼ�
    int16_t  Offset_Ay; // ���ӵ� y�� �ɼ�
    int16_t  Offset_Az; // ���ӵ� z�� �ɼ�
    float64_t Gain_Bx;   // �ڱ��� x�� �̵�
    float64_t Gain_By;   // �ڱ��� y�� �̵�
    float64_t Gain_Bz;   // �ڱ��� z�� �̵�
    int16_t  Offset_Bx; // �ڱ��� x�� �ɼ�
    int16_t  Offset_By; // �ڱ��� y�� �ɼ�
    int16_t  Offset_Bz; // �ڱ��� z�� �ɼ�
    int16_t calOffsetBx;    // �ڱ��� x�� calibration ��� �ɼ�
    int16_t calOffsetBy;    // �ڱ��� y�� calibration ��� �ɼ�
    int16_t calOffsetBz;    // �ڱ��� z�� calibration ��� �ɼ�

    uint16_t ssm;
};


struct _mhsensor_fluxrightAngle_Data
{
    float64_t matrix_x00;    // ������ x 00��
    float64_t matrix_x01;    // ������ x 01��
    float64_t matrix_x02;    // ������ x 02��
    float64_t matrix_y10;    // ������ y 10��
    float64_t matrix_y11;    // ������ y 11��
    float64_t matrix_y12;    // ������ y 12��
    float64_t matrix_z20;    // ������ z 20��
    float64_t matrix_z21;    // ������ z 21��
    float64_t matrix_z22;    // ������ z 22��

};

struct _mhsensor_accelrightAngle_Data
{
    float64_t matrix_x00;    // ������ x�� 00
    float64_t matrix_x01;    // ������ x�� 01
    float64_t matrix_y10;    // ������ y�� 10
    float64_t matrix_y11;    // ������ y�� 11

};

struct _mhsensor_sensor_Data
{
    int16_t tempValue;    //
    float64_t voltage1v2;    // ������ x�� 01
    float64_t voltage3v3;    // ������ y�� 10
    float64_t voltage5v;    // ������ y�� 11
    float64_t voltage28v;    // ������ y�� 11

};


extern struct _mhsensor_calibration_Data mhsensor_calibration_Data;
extern struct _mhsensor_fluxrightAngle_Data mhsensor_fluxrightAngle_Data;
extern struct _mhsensor_accelrightAngle_Data mhsensor_accelrightAngle_Data;
extern struct _mhsensor_sensor_Data mhsensor_sensor_Data;


extern uint16_t  bParameterError;
extern uint16_t gCheckSum_code;
extern uint16_t eepromcrc;
extern uint16_t gversion_code;

void read_parameter(void);
uint16_t data_write_to_eeprom(uint32_t address, uint16_t data);
uint16_t data_write_to_eeprom_calibraion(uint32_t address, uint16_t data);

#endif /* CSU_CALIBRATIONDATA_H_ */
