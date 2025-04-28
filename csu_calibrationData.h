
#ifndef CSU_CALIBRATIONDATA_H_
#define CSU_CALIBRATIONDATA_H_

#include "Typedef.h"


struct _mhsensor_calibration_Data
{
    float64_t Gain_Ax;    // 가속도 x축 이득
    float64_t Gain_Ay;    // 가속도 y축 이득
    float64_t Gain_Az;    // 가속도 z축 이득
    int16_t  Offset_Ax; // 가속도 x축 옵셋
    int16_t  Offset_Ay; // 가속도 y축 옵셋
    int16_t  Offset_Az; // 가속도 z축 옵셋
    float64_t Gain_Bx;   // 자기장 x축 이득
    float64_t Gain_By;   // 자기장 y축 이득
    float64_t Gain_Bz;   // 자기장 z축 이득
    int16_t  Offset_Bx; // 자기장 x축 옵셋
    int16_t  Offset_By; // 자기장 y축 옵셋
    int16_t  Offset_Bz; // 자기장 z축 옵셋
    int16_t calOffsetBx;    // 자기장 x축 calibration 모드 옵셋
    int16_t calOffsetBy;    // 자기장 y축 calibration 모드 옵셋
    int16_t calOffsetBz;    // 자기장 z축 calibration 모드 옵셋

    uint16_t ssm;
};


struct _mhsensor_fluxrightAngle_Data
{
    float64_t matrix_x00;    // 직각도 x 00축
    float64_t matrix_x01;    // 직각도 x 01축
    float64_t matrix_x02;    // 직각도 x 02축
    float64_t matrix_y10;    // 직각도 y 10축
    float64_t matrix_y11;    // 직각도 y 11축
    float64_t matrix_y12;    // 직각도 y 12축
    float64_t matrix_z20;    // 직각도 z 20축
    float64_t matrix_z21;    // 직각도 z 21축
    float64_t matrix_z22;    // 직각도 z 22축

};

struct _mhsensor_accelrightAngle_Data
{
    float64_t matrix_x00;    // 직각도 x축 00
    float64_t matrix_x01;    // 직각도 x축 01
    float64_t matrix_y10;    // 직각도 y축 10
    float64_t matrix_y11;    // 직각도 y축 11

};

struct _mhsensor_sensor_Data
{
    int16_t tempValue;    //
    float64_t voltage1v2;    // 직각도 x축 01
    float64_t voltage3v3;    // 직각도 y축 10
    float64_t voltage5v;    // 직각도 y축 11
    float64_t voltage28v;    // 직각도 y축 11

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
