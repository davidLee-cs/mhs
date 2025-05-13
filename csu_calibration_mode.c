/*
 * calibration_mode.c
 *
 * ��� : �����ڼ� �ɼ� ���� �� EEProm�� �����ϴ� ���
 * ������� : Calibration mode CSU (D-MHS-SFR-009)
 * �̷� :
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */

#include "mhs_project.h"


#define X_AXIS  0
#define Y_AXIS  1
#define Z_AXIS  2


// ȸ�� ���� ����
static int16_t cw_rotations;     // CW ���� ȸ�� Ƚ��

static uint16_t emaCnt;          // ���� �ð��� �ֱ� ���� ī����
static uint16_t calRotationDone; // MHS ȸ�� �Ϸ� �÷���

static int16_t calibration_ema_filter(float64_t adc, float64_t alpha, int16_t previous_ema_filtered_value);
static void detect_rotation(float64_t sensor_value);


// ��� : discrete_1 ����ġ�� LOW �̸� cal�� CBIT �� �����ϰ�, MHS�� 2ȸ �̻� ȸ�� �Ŀ� eeprom ��忡 ������ �����ϰ� ssm ���¸� cal ��忡�� noraml ���� �����Ѵ�.
//       ��, CBIT ���� �� ������ �߻� ���Ŀ��� ssm ���¸�  STATUS_BCD_FW ���·� �����Ѵ�.
// ����� ��������
// float64_t Box;  //�ڱ��� x��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
// float64_t Boy;  //�ڱ��� y��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
// float64_t Boz;  //�ڱ��� z��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
// mhsensor_calibration_Data.calOffsetBx;    // �ڱ��� x�� calibration ��� �ɼ�
// mhsensor_calibration_Data.calOffsetBy;    // �ڱ��� y�� calibration ��� �ɼ�
// mhsensor_calibration_Data.calOffsetBz;    // �ڱ��� z�� calibration ��� �ɼ�

// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ�
void calibrationMode(void)
{

    uint16_t ssmData = 0U;

    static uint16_t errorCbit = 0U;
    static int16_t emaBx=0;
    static int16_t firstCalRun = 0;
    static int16_t calModeCnt= 0;
    static uint16_t HeandingCnt= 0U;
    static uint16_t HeandingAngle= 0U;
    static uint16_t calOffsetDone = 0U;

	static int16_t maxB[3] = {0,};	// calibration mode���� ������ �ڱ��� x�� �ִ밪
	static int16_t minB[3] = {0,};	// calibration mode���� ������ �ڱ��� x�� �ּҰ�
    int16_t getB[3] = {0,};

    static int16_t maxB_1[6] = {0,};   // calibration mode���� ������ �ڱ��� x�� �ִ밪
    static int16_t maxB_2[6] = {0,};   // calibration mode���� ������ �ڱ��� x�� �ִ밪
    float64_t error_B[6] = {0,};
    int16_t i;

    // 1. �ڱ��� ����, ����, �µ��� ����
    MeasureFlux();
    MeasureVoltage();
    MeasureBoardTemperature();

    // 2. �ڱ��� x���� ema ���͸� ����
    emaBx = calibration_ema_filter(Box, 0.90, emaBx);

    // 3. �� �ڱ��尪�� ���ۿ� ����
    getB[X_AXIS] = (int16_t)Box; // min/max �񱳸� ���Ͽ� float�� int �� ��ȯ
    getB[Y_AXIS] = (int16_t)Boy;
    getB[Z_AXIS] = (int16_t)Boz;

    // 4. ó�� ���� �� ���� min, max ���� ���� ������ ����.
    if(firstCalRun == 0)
    {
        firstCalRun = 1;

        maxB[X_AXIS] = getB[X_AXIS];
        minB[X_AXIS] = getB[X_AXIS];

        maxB[Y_AXIS] = getB[Y_AXIS];
        minB[Y_AXIS] = getB[Y_AXIS];

        maxB[Z_AXIS] = getB[Z_AXIS];
        minB[Z_AXIS] = getB[Z_AXIS];

    }


    // 5. �� �ະ ���簪�� max[] ���� ũ�� max[]�� ����, min[] ���� ������  min[]�� ����
    for(i=0; i<3; i++)
    {

        if(getB[i] >= maxB[i])
        {
           maxB[i] = getB[i];
        }

        if(minB[i] > getB[i])
        {
           minB[i] = getB[i];
        }
    }

    // 6. 1ȸ�� ���� ���̸� maxB_1[] ���ۿ� ����
    if(cw_rotations == 1)
    {
        maxB_1[0] = maxB[X_AXIS];
        maxB_1[1] = maxB[Y_AXIS];
        maxB_1[2] = maxB[Z_AXIS];
        maxB_1[3] = minB[X_AXIS];
        maxB_1[4] = minB[Y_AXIS];
        maxB_1[5] = minB[Z_AXIS];
    }

    // 7. 2ȸ�� �������̸� maxB_2[] ���ۿ� ����
    if(cw_rotations == 2)
    {
        maxB_2[0] = maxB[X_AXIS];
        maxB_2[1] = maxB[Y_AXIS];
        maxB_2[2] = maxB[Z_AXIS];
        maxB_2[3] = minB[X_AXIS];
        maxB_2[4] = minB[Y_AXIS];
        maxB_2[5] = minB[Z_AXIS];
    }

    // 8. 2.5�� ���� detect_rotation() �Լ� ȣ���Ͽ� ȸ�� ���� ����
    if(calModeCnt++ > 50) /// 2.5�� ���
    {
        calModeCnt = 0;
        // 8.1 calOffsetDone =0, ��, cal ����϶��� ����
        if(calOffsetDone == 0U) // cal ��� �϶��� ����,
        {
            detect_rotation((float64_t)emaBx);
        }
    }

    // 9. checkCbit()�� ȣ���Ͽ� CBIT ���� �� ���� Ȯ��
    if(checkCbit() == 1U)
    {
        errorCbit = 1U;
    }

    // 10. 2ȸ�� �̻� ȸ�� �� calOffsetDone  = 1 �� ���ϸ� 1ȸ�� �� �� �� �ִ�,�ּҿ� 2ȸ�� �� �� �� �ִ�, �ְ��� ���̰� ���� ������ +/- 20% �̻� �����
    // cbit error�� 1�� �����Ѵ�.
    if(calOffsetDone == 1U)
    {
        for(i=0; i<6; i++)
        {
            error_B[i] = (float64_t)maxB_1[i] / (float64_t)maxB_2[i];

            if((1.2L < error_B[i]) || (error_B[i] < 0.8L))
            {
                errorCbit = 1U;
                calOffsetDone = 0U;
            }
        }
    }


    // 11. errorCbit �ѹ� �̻� �߻��ϸ� cal ��忡���� ��� STATUS_BCD_FW ����
    if((errorCbit == 1U) || (bParameterError == 1U))
    {
        ssmData = STATUS_BCD_FW;
    }
    else
    {
        // 11.1 eeprom �����и� ���������� �о����� Ȯ�� �� ssm ���� ��Ʈ ����
        // mhsensor_calibration_Data.ssm = 1 �̸� STATUS_NO_COMPUT_DATA ����
        //  mhsensor_calibration_Data.ssm  = 0 �̸� STATUS_BCD_NORMAL ����
        if(mhsensor_calibration_Data.ssm == 1U)
        {
            ssmData = STATUS_NO_COMPUT_DATA;
        }
        else
        {
            ssmData = STATUS_BCD_NORMAL;
        }
    }

    // 12. 2ȸ�� �̻� ȸ�� �� calOffsetDone  = 1 �� ���ϸ� ���� ��带 NORMAL_MODE �� ���� ������ ����
    // ��, �װ������� �Ϸ�Ǿ� ���� ���� ���� ��.
    if(calOffsetDone == 1U)
    {
        mhs_status_trans(NORMAL_MODE, ssmData);
    }
    else
    {
        // 12.1 �װ� ���� ��忡���� 1�� �ֱ�� 105 �� �� ���� CAL_MODE�� ������ ����
        if(HeandingCnt++ >= 20U) // 1��
        {
            HeandingCnt = 0U;
            HeandingAngle += 1050U;
            if(HeandingAngle >= 3600U)
            {
                HeandingAngle = HeandingAngle - 3600U;
            }

            mhsensor_data.MagHeading = HeandingAngle;
        }
        // 1.1..4. �������� SFI�� ����
        mhs_status_trans(CAL_MODE, ssmData);
    }

    // 13. calRotationDone 1 �̸� cal offset x,y,z ���� eeprom �� ����
    if(calRotationDone == 1U)
    {

    	// 13.1  CAL �Ϸ�� EEPROM �����ϰ� eerorm ������ crc16 üũ �� calRotationDone = 0 ���� �����Ͽ� �� cal ��带 �Ϸ��ϰ� operation mode�� �����͸� �����Ѵ�.
        mhsensor_calibration_Data.calOffsetBx = (maxB[X_AXIS] + minB[X_AXIS]) / 2 ;
        mhsensor_calibration_Data.calOffsetBy = (maxB[Y_AXIS] + minB[Y_AXIS]) / 2 ;
        mhsensor_calibration_Data.calOffsetBz = (maxB[Z_AXIS] + minB[Z_AXIS]) / 2 ;

        (void)data_write_to_eeprom_calibraion(EEPROM_BX_CAL_OFFSET_ADDRESS, (uint16_t)mhsensor_calibration_Data.calOffsetBx);
        (void)data_write_to_eeprom_calibraion(EEPROM_BY_CAL_OFFSET_ADDRESS, (uint16_t)mhsensor_calibration_Data.calOffsetBy);
        (void)data_write_to_eeprom_calibraion(EEPROM_BZ_CAL_OFFSET_ADDRESS, (uint16_t)mhsensor_calibration_Data.calOffsetBz);

        writeEepromCRC16();

        calRotationDone = 0;
        cw_rotations = 0; // �ʱ�ȭ
        emaCnt = 0;
        firstCalRun = 0;
        calOffsetDone = 1U;  // cal �Ϸ�Ǹ� ���� ���� ������ cal ���� ����.
        mhsensor_calibration_Data.ssm = 0U;
    }


#if 0
        calsendUart(cw_rotations, emaBx);
#endif

    // 14. ���� ���¸� calibration mode �� ����
    Can_State_Ptr = &calibrationMode;//calibration mode
}

// ��� : calibration ��� ���� �� ȸ���� Ȯ���ϴ� �Լ�
// ����� ��������
// float64_t sensor_value;  //�ڱ��� x��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
// static uint16_t emaCnt;          // ���� �ð����� ���� �ð��� �ֱ� ���� ī����;
// static int16_t cw_rotations;     // CW ���� ȸ�� Ƚ��
// static uint16_t calRotationDone; // MHS ȸ�� �Ϸ� �÷���

// �̷� :
//      2024.05.23 : ����� : �ʱ� �ۼ�
static void detect_rotation(float64_t sensor_value) {

    static uint16_t is_increasing = 0U;  // ���� ���� ���� ������ ����
    static int16_t first_measurement = 1; // ù ���� ����
    static float64_t baseline = 0.0L;     // ù ��° ������ (���ذ�)

    // 1. ù ��° �������� baseline ���ذ����� ����
    if (first_measurement == 1)
    {

        if(emaCnt++ > 3U)
        {
            baseline = sensor_value;
            first_measurement = 0;
        }
    }
    else
    {
        // 1.1 ȸ�� �� ������ ���̰� 0.2uT �����̸� ���� ȸ�� ������ �Ǵ��ϰ� ȸ�� ���ø� ���� �ʰ� return ��Ŵ.
        if(fabsl(sensor_value - baseline) < 20.0L)
        {
            return;
        }
        else
        {

            // 1.1.1 ���� ���� ������ ��, ���Ⱑ ����̸� �� ���� ���� ��Ų �� is_increasing ���¸� 1�� �����Ͽ� ���� ȸ�� ���� ����
            if ((sensor_value > baseline) && (is_increasing == 0U)) {
                // ���� ���� ������ ��, ���ذ��� �ʰ��ϸ� �� ���� ����
                cw_rotations++;
                is_increasing = 1U; // ���� ���·� �ٲ�
            }

            // 1.1.2 ���� ���� ���� �� ���Ⱑ ���� ��.is_increasing ���¸� 0�� �����Ͽ� ���Ⱑ ����� ���� ���
            if ((sensor_value < baseline) && (is_increasing == 1U)) {
                // ���ذ� ���Ϸ� ��������, ȸ���� �������� �����ϰ� ���� ���¸� ����
                is_increasing = 0;
            }

            // 1.1.3 CW ���� 2ȸ��(720��) ���� �� calRotationDone = 1�� �����Ͽ� cal �Ϸ� ��.
            if (cw_rotations >= 3) {
                calRotationDone = 1U;
            }

        }
    }

}


/*
��ɼ���
 ���� �̵� ���(EMA, Exponential Moving Average) ���͸� �����Ͽ� ADC ���� ���͸� �ϴ� �Լ�

����� ����
float64_t adc : ���� ������ ADC ��
float64_t alpha : ���� ���� ���� ��
int16_t previous_ema_filtered_value : ���� ���͵� ��
*/
static int16_t calibration_ema_filter(float64_t adc, float64_t alpha, int16_t previous_ema_filtered_value)
{
    float64_t new_ema_filtered_value;

    // 1. �����̵����(EMA)
    // ���� : EMAnew=(1?��)��ADC+�᡿EMAprev
    new_ema_filtered_value  = ((1.0 - alpha) * adc) + (alpha * (float64_t)previous_ema_filtered_value);

    // 2. ���͵� �� ����
    return  (int16_t)new_ema_filtered_value;
}


