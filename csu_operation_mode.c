/*
 * operation_mode.c
 *
 * ��� : �ֱ���(50ms)���� �ڱ���, ���ӵ�, ����, �µ��� �����ϰ�, �ڱ���� ���ӵ��� �̿��Ͽ� ������ ��� �� SFI �� �����͸� �����Ѵ�.
 * ������� : Operation mode (D- )
 * �̷� : 
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 */

#include <stdbool.h>
#include "mhs_project.h"


/*
��ɼ���
�ֱ���(50ms)���� �ڱ���, ���ӵ�, ����, �µ��� �����ϰ�, �ڱ���� ���ӵ��� �̿��Ͽ� ������ ��� �� SFI �� �����͸� �����Ѵ�.

����� ��������
float64_t g0x;   ���ӵ� x��, ���� g, ���е�  , ����    0 ~  +/- 1.6g
float64_t g0y;   ���ӵ� y��, ���� g, ���е�  , ����    0 ~  +/- 1.6g
float64_t g0z;   ���ӵ� z��, ���� g, ���е�  , ����    0 ~  +/- 1.6g
float64_t Box;   �ڱ��� x��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
float64_t Boy;   �ڱ��� y��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
float64_t Boz;   �ڱ��� z��, ���� gauss, ���е� 100nT , ����    -10000 ~ +10000
uint16_t StatusMatrix;  C_bit ���� �� ���� Ȥ�� Parameter �������� ���� �߻� ����. -
(1: Error State, 0: No Error )
*/
void operation_mode(void)
{

	uint16_t AngleAy;

	uint16_t ssmData = 1U;


	// 1. interrupt ���� ������ filtered  �� ADC���� �ڱ���, ���ӵ�, ����, �µ��� ��ȯ
    MeasureFlux();
    MeasureAccelation();
    MeasureVoltage();
    MeasureBoardTemperature();

	// 2. ������ ���
    if (( isfinite( g0x ) !=0 ) && ( isfinite( g0y ) !=0 ) && ( isfinite( Box ) !=0 ) && ( isfinite( Boy ) !=0 ) && ( isfinite( Boz ) !=0 ))
    {
        AngleAy = angle_calculation(g0x,g0y,Box,Boy,Boz);
        mhsensor_data.MagHeading = AngleAy;
    }

	// 3. CBIT ����
    uint16_t errorCbit =  checkCbit();
    if((errorCbit == 1U) || (bParameterError == 1U))
    {
        ssmData = STATUS_BCD_FW;
    }
    else
    {
        ssmData = STATUS_BCD_NORMAL;
    }

    if(mhsensor_calibration_Data.ssm == 1U)
    {
        ssmData = STATUS_NO_COMPUT_DATA;
    }

	// 4. ���� ���� SFI�� ����
    mhs_status_trans(NORMAL_MODE, ssmData);

#if 1
    sendUart(Box, Boy, Boz, g0x, g0y, gFluxrx, gFluxry, AngleAy);
#endif

}

