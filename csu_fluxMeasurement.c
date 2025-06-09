/*
 * csu_fluxMeasurement.c
 *
 * ���� : 1. �����ε� ADC �����Ϳ��� �ڱ��尪 ��ȯ 
 *       2. ��ȯ �� �ڱ��尪�� ARINC429 ������ �ڱ��� �������� ��ȯ
 *
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 *
 */


#include "mhs_project.h"


#define MAG_RESOLUTION      (32768.0L/200.0L)  // +/-range / 2^15
#define MAX_100UH           (10000)
#define MIN_100UH           (-10000)
#define MAX_120UH           (12000)
#define MIN_120UH           (-12000)


float64_t Box = 0.0L;  //�ڱ��� x��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000 
float64_t Boy = 0.0L;  //�ڱ��� y��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000 
float64_t Boz = 0.0L;  //�ڱ��� z��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000 

int16_t filterBx = 0;
int16_t filterBy = 0;
int16_t filterBz = 0;
int16_t filterAx = 0;
int16_t filterAy = 0;


struct _mhsensor_data mhsensor_data;

static void converterToArinc429(void);
static void libConverterToArinc429(int16_t *pmag, float64_t fluxOut);

static float64_t gConstant_B[3][3] =
        { {1.0L, 0.0L, 0.0L},       //x�� ��� 3x3
          {0.0L, 1.0L, 0.0L},       //y�� ��� 3x3
          {0.0L, 0.0L, 1.0L}        //z�� ��� 3x3
        };



// eeprom�� ����� �ڱ��� �ຸ�� ��� ���� �о� gConstant_B ����
// �Է� �������� :
//      gConstant_B[][] : �ڱ��� �ຸ���� ��� 3x3 ����
// �̷� :
//    2024.05.23 : ����� : �ʱ� �ۼ�

void flux_matrix_init(void)
{
    // 1. eeprom�� ���� �� x,y,z ���� �ຸ������ �� ��� gConstant 3x3 ������ �����Ѵ�.
    gConstant_B[0][0] = mhsensor_fluxrightAngle_Data.matrix_x00;
    gConstant_B[0][1] = mhsensor_fluxrightAngle_Data.matrix_x01;
    gConstant_B[0][2] = mhsensor_fluxrightAngle_Data.matrix_x02;

    gConstant_B[1][0] = mhsensor_fluxrightAngle_Data.matrix_y10;
    gConstant_B[1][1] = mhsensor_fluxrightAngle_Data.matrix_y11;
    gConstant_B[1][2] = mhsensor_fluxrightAngle_Data.matrix_y12;

    gConstant_B[2][0] = mhsensor_fluxrightAngle_Data.matrix_z20;
    gConstant_B[2][1] = mhsensor_fluxrightAngle_Data.matrix_z21;
    gConstant_B[2][2] = mhsensor_fluxrightAngle_Data.matrix_z22;
}


// adc���� ���͸� �����͸� �̿��Ͽ� �ڱ��� ������ ��ȯ
// �Է� �������� : ema[ADC_CH_INDEX_FLUX_X], ema[ADC_CH_INDEX_FLUX_Y], ema[ADC_CH_INDEX_FLUX_Z], 
//              Offset_Bx, Offset_By, Offset_Bz, 
//              Gain_Bx, Gain_By, Gain_Bz, 
//              calOffsetBx, calOffsetBy, calOffsetBz
// ��� : Box, Boy, Boz
void MeasureFlux(void)
{
    int16_t i;
    float64_t Result_calibration[3];
    float64_t gAverageADC_B[3];

    float64_t Constx;
    float64_t Consty;
    float64_t Constz;

    int16_t realBx;
    int16_t realBy;
    int16_t realBz;

    int16_t maxBvalue;
    int16_t minBvalue;

    // 1.x,y,z �ະ�� �ڱ��尪 ��� = (���͸� �� adc �� - �ɼ�) * ���� 
    int16_t fluxX0 = filterBx - mhsensor_calibration_Data.Offset_Bx;
    gAverageADC_B[0] = ((float64_t)(fluxX0)) * mhsensor_calibration_Data.Gain_Bx;
    int16_t fluxY0 = filterBy - mhsensor_calibration_Data.Offset_By;
    gAverageADC_B[1] = ((float64_t)(fluxY0)) * mhsensor_calibration_Data.Gain_By;
    int16_t fluxZ0 = filterBz - mhsensor_calibration_Data.Offset_Bz;
    gAverageADC_B[2] = ((float64_t)(fluxZ0)) * mhsensor_calibration_Data.Gain_Bz;


    // 2. x,y,z�� ������ ���� = (x,y,z �ະ�� �ڱ��尪) x (������ ������ ���)
    for(i=0;i<3;i++)
    {
        Constx = (float64_t)gConstant_B[i][0]*gAverageADC_B[0];
        Consty= (float64_t)gConstant_B[i][1]*gAverageADC_B[1];
        Constz =  (float64_t)gConstant_B[i][2]*gAverageADC_B[2];
        Result_calibration[i] = Constx + Consty + Constz;
    }

    // 3. cal ��� �϶��� cal �ɼ� �������� �����ϰ� ���� �ڱ��� ���� �����Ѵ�. �Ϲ� ��忡���� cal �ɼ��� ó���� �� �����Ѵ�.
    if(calibration_mode == 1U)
    {
        realBx = (int16_t)Result_calibration[0];
        realBy = (int16_t)Result_calibration[1];
        realBz = (int16_t)Result_calibration[2];
    }
    else
    {
        realBx = (int16_t)Result_calibration[0] - mhsensor_calibration_Data.calOffsetBx;
        realBy = (int16_t)Result_calibration[1] - mhsensor_calibration_Data.calOffsetBy;
        realBz = (int16_t)Result_calibration[2] - mhsensor_calibration_Data.calOffsetBz;
    }


    // 4. �� �ະ �ִ�, �ּҰ��� factory mode �϶��� +/-120 uT ����,
    // operation mode�� calibration mode�� +/-100uT �� ����
    if(factory_Fluxmode == 1U)
    {
        maxBvalue = MAX_120UH;
        minBvalue = MIN_120UH;
    }
    else
    {
        maxBvalue = MAX_100UH;
        minBvalue = MIN_100UH;
    }

    if(realBx > maxBvalue)
    {
        Box = (float64_t)maxBvalue;
    }
    else if(realBx < minBvalue)
    {
        Box = (float64_t)minBvalue;
    }
    else
    {
        Box = (float64_t)realBx;
    }

    if(realBy > maxBvalue)
    {
        Boy = (float64_t)maxBvalue;
    }
    else if(realBy < minBvalue)
    {
        Boy = (float64_t)minBvalue;
    }
    else
    {
        Boy = (float64_t)realBy;
    }

    if(realBz > maxBvalue)
    {
        Boz = (float64_t)maxBvalue;
    }
    else if(realBz < minBvalue)
    {
        Boz = (float64_t)minBvalue;
    }
    else
    {
        Boz = (float64_t)realBz;
    }

	// 5. ������ �ڱ����� ARINC429 �������� ��ȯ
	converterToArinc429();
}


// ���͵� �ڱ��� x,y,z ���� aring429 �����ϱ� ���� �����ͷ� ��ȯ 
// �Է� �������� : Box, Boy, Boz
// ��� �������� : mhsensor_data
static void converterToArinc429(void)
{
    libConverterToArinc429(&mhsensor_data.Mag_x, Box);
    libConverterToArinc429(&mhsensor_data.Mag_y, Boy);
    libConverterToArinc429(&mhsensor_data.Mag_z, Boz);
}


/*
 ��ɼ���
 ���͵� �ڱ��� ���� arinc429 �������ݷ� ��ȯ�ϴ� �Լ�

�Էº���
uint16_t *pmag : mhsensor_data ����ü�� �ڱ��� �� ���� ��ġ ������
uint16_t *psignmag : mhsensor_data ����ü�� �ڱ��� ��ȣ ��ġ ������
float64_t fluxOut : �ڱ��� ��
*/
static void libConverterToArinc429(int16_t *pmag, float64_t fluxOut)
{
    int16_t returntMagnectic;
    float64_t flux;
    float64_t flux_uT;
    // 1. ���͵� �ڱ��� ���� �����̸� psigmag ���� 1, ����̸� 0���� ����, �ڱ��� ���� �������ǰ����� ������ �����͸� ��ȯ�Ѵ�.
    // ��ȯ�� �ڱ��� ���� 14�̳��� ǥ���ؾ� �Ѵ�.

    flux_uT = fluxOut * 0.01L;

    flux = flux_uT * MAG_RESOLUTION;
    returntMagnectic = (int16_t)flux;

    *pmag  =  returntMagnectic;

}

