/*
 * lib.mhsAdc.c
 *
 * ��� :ADC �ʱ⼳��,  �ڱ���, ���ӵ�, ����, �µ� ���� �� ���� ����
 * ������� : ��÷
 * �̷� :
 *    2024.05.23 : ����� : �ʱ� �ۼ�
 *
 *
 * */
 
#include "mhs_project.h"

static int16_t ema_filter(float64_t adc, float64_t alpha, int16_t previous_ema_filtered_value);
static void initValue(void);


int16_t ema[ADC_CH_INDEX_MAX] = {0};	// �����̵����(EMA)���͵� ���� �����ϴ� ���� 


#define EMA_FILTER_ALPHA        (0.7L)
#define EMA_FILTER_ALPHA_VOLT   (0.9L)

/*
��ɼ���
�ڱ���, ���ӵ� ���� ������ �ʱ�ȭ �Լ�

����� ��������
float64_t mhsensor_calibration_Data.Gain_Ax;   ���ӵ� x�� �̵�
float64_t mhsensor_calibration_Data.Gain_Ay;   ���ӵ� y�� �̵�
float64_t mhsensor_calibration_Data.Gain_Az;   ���ӵ� z�� �̵�
int16_t  mhsensor_calibration_Data.Offset_Ax;  ���ӵ� x�� �ɼ�
int16_t  mhsensor_calibration_Data.Offset_Ay;  ���ӵ� y�� �ɼ�
int16_t  mhsensor_calibration_Data.Offset_Az;  ���ӵ� z�� �ɼ�
float64_t mhsensor_calibration_Data.Gain_Bx;   �ڱ��� x�� �̵�
float64_t mhsensor_calibration_Data.Gain_By;   �ڱ��� y�� �̵�
float64_t mhsensor_calibration_Data.Gain_Bz;   �ڱ��� z�� �̵�
int16_t  mhsensor_calibration_Data.Offset_Bx;  �ڱ��� x�� �ɼ�
int16_t  mhsensor_calibration_Data.Offset_By;  �ڱ��� y�� �ɼ�
int16_t  mhsensor_calibration_Data.Offset_Bz;  �ڱ��� z�� �ɼ�
*/
static void initValue(void)
{
    // ��õ����
    mhsensor_calibration_Data.Gain_Bx = -0.965251;
    mhsensor_calibration_Data.Gain_By = -0.949307;
    mhsensor_calibration_Data.Gain_Bz = -0.954563;
    mhsensor_calibration_Data.Offset_Bx   = -120;
    mhsensor_calibration_Data.Offset_By   = 80;
    mhsensor_calibration_Data.Offset_Bz   = -147;

    mhsensor_calibration_Data.Gain_Ax = 0.000169;
    mhsensor_calibration_Data.Gain_Ay = -0.00019;
    mhsensor_calibration_Data.Gain_Az = 0.0001;
    mhsensor_calibration_Data.Offset_Ax   = 160;
    mhsensor_calibration_Data.Offset_Ay   = -700;
    mhsensor_calibration_Data.Offset_Az   = 0;

}

/*
��ɼ���
���ش�(bittype)�� ���� ADC�� �۵� ���� Ŭ�� ����, ���ͷ�Ʈ ���, ��ȯ�� Ȱ��ȭ ���� �����ϴ� �Լ�

�Է� ����
uint32_t adcBase,: ADC ��� �ּ�
uint16_t bittype : ADC ���ش� ������
*/

void configureADC(uint32_t adcBase, uint16_t bittype)
{
    uint16_t resoulution = bittype;

	// 1. ADC Ŭ���� �ý��� Ŭ���� 4���� 1�� ����
    ADC_setPrescaler(adcBase, ADC_CLK_DIV_4_0);

	// 2. ���ش��� ADC ��� ����
    if(resoulution == 12U)
    {
    	//2.1 12bit ���ش�, �̱� �ص� ���� ���� 
		ADC_setMode(adcBase, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    }
    else
    {
    	//2.2 16bit ���ش�, ���� ���� ���� 
        ADC_setMode(adcBase, ADC_RESOLUTION_16BIT, ADC_MODE_DIFFERENTIAL);
    }

	// 3. ADC ��ȯ ���� ������ ���ͷ�Ʈ �߻����� ���� 
    ADC_setInterruptPulseMode(adcBase, ADC_PULSE_END_OF_CONV);

	//4. ADC ����� Ȱ��ȭ�Ͽ� ��ȯ�� ���۵� ��
    ADC_enableConverter(adcBase);

	// ADC ����ȭ�� ���� ���� �ð� 
    delay_uS(1000.0L);
}


/*
��ɼ���
 ADC ����� SOC ����, ���ͷ�Ʈ ó���� �����ϴ� �ʱ�ȭ �Լ�
*/
void initADCSOC(void)
{
    // 1. initValue()�� �̿��Ͽ� �ڱ���, ���ӵ� ���� ���� �ʱ�ȭ
    initValue();

#ifdef NEW_BOARD
    // 2. 16bit ADCA ����� SOC(Sample and Hold Circuit) ä�κ�  ����
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0_ADCIN1, 63);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2_ADCIN3, 63);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN14_ADCIN15, 63);

    // 3. 16bit ADCB ����� SOC(Sample and Hold Circuit) ä�κ�  ����
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0_ADCIN1, 63);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2_ADCIN3, 63);

    // 4. 12bit ADCD ����� SOC(Sample and Hold Circuit) ä�κ�  ����
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0, 200);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN1, 200);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2, 200);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN3, 200);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER4, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN13, 200);

#else
    // 2. 16bit ADCA ����� SOC(Sample and Hold Circuit) ä�κ�  ����
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0_ADCIN1, 63);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2_ADCIN3, 63);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN4_ADCIN5, 63);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN12_ADCIN13, 63);

    // 3. 16bit ADCB ����� SOC(Sample and Hold Circuit) ä�κ�  ����
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0_ADCIN1, 63);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2_ADCIN3, 63);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN4_ADCIN5, 63);

    // 4. 12bit ADCD ����� SOC(Sample and Hold Circuit) ä�κ�  ����
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN0, 14);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN1, 14);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2, 14);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN3, 14);
#endif

    // 5. ADC_INT_NUMBER1 ���ͷ�Ʈ�� ADC_SOC_NUMBER3 ��ȯ �Ϸ� �� �߻�
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER3);
	
	// 6. ADC_INT_NUMBER1 ���ͷ�Ʈ�� Ȱ��ȭ�Ͽ� ��ȯ�� �Ϸ�� �� ���ͷ�Ʈ�� �߻�
	ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);

	// 7. ���� �ʱ� ���¿��� ���ͷ�Ʈ Ŭ���� ��Ŵ.
	ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);


}

/*
��ɼ���
ADCA ��⿡�� ADC ��ȯ �Ϸ� �� �߻��ϴ� ���ͷ�Ʈ ���� ��ƾ �Լ�

����� ��������

int16_t ema[12];   �����̵����(EMA)���͵� ���� �����ϴ� ����
*/
__interrupt void adcA1ISR(void)
{

    uint16_t read0,read1,read2;

    int16_t fluxread[3];
    uint16_t adcax0, adcax1;
    int16_t accread[3];

    uint16_t gadcvalue6=0;
    uint16_t gadcvalue7=0;
    uint16_t gadcvalue8=0;
    uint16_t gadcvalue9=0;
    uint16_t gadcvalue10=0;

    int16_t gBxADC;
    int16_t gByADC;
    int16_t gBzADC;
    int16_t gAxADC;
    int16_t gAyADC;
    int16_t gAzADC;

    uint16_t result_0, result_1, result_2;
    uint16_t result_a0, result_a1;

	// 1. x,y,z�� �ڱ��� ���� ������ �б� �� ��ȯ
#ifdef NEW_BOARD
    read0 =  ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER0);  // x
    read1 =  ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER1);  // y
    read2 =  ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER2);  // z
#else
    read0 =  ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);  // x
    read1 =  ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);  // y
    read2 =  ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);  // z

#endif

    // 2. ADCA ����� SOC ��ȣ 0�� ����� �о� �ڱ��� x �� ������(fluxread)�� ��ȯ
    //    ��, �о�� ���� 0x7FFFU���� ũ�� �״�� ����ϰ�, �۴ٸ� �����Ͽ� ��ȣȭ
    if(read0 > 0x7FFFU)
    {
        result_0 = read0 - 0x7FFFU;
        fluxread[0] = (int16_t)result_0;
    }
    else
    {
        result_0 = 0x7FFFU - read0;
        fluxread[0] = (int16_t)result_0 * -1;
    }

    // 3. ADCA ����� SOC ��ȣ 1�� ����� �о� �ڱ��� y �� ������(fluxread)�� ��ȯ
    //    ��, �о�� ���� 0x7FFFU���� ũ�� �״�� ����ϰ�, �۴ٸ� �����Ͽ� ��ȣȭ
    if(read1 > 0x7FFFU)
    {
        result_1 = read1 - 0x7FFFU;
        fluxread[1] = (int16_t)result_1;
    }
    else
    {
        result_1 = 0x7FFFU - read1;
        fluxread[1] = (int16_t)result_1 * -1;
    }

    // 4. ADCA ����� SOC ��ȣ 2�� ����� �о� �ڱ��� z �� ������(fluxread)�� ��ȯ
    //    ��, �о�� ���� 0x7FFFU���� ũ�� �״�� ����ϰ�, �۴ٸ� �����Ͽ� ��ȣȭ
    if(read2 > 0x7FFFU)
    {
        result_2 = read2 - 0x7FFFU;
        fluxread[2] = (int16_t)result_2;
    }
    else
    {
        result_2 = 0x7FFFU - read2;
        fluxread[2] = (int16_t)result_2 * -1;
    }


	// 5. x,y,z�� ���ӵ� ���� ������ �б� �� ��ȯ
    adcax0 = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0);
    adcax1 = ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER1);

    // 6. ADCB����� SOC ��ȣ 0�� ����� �о� ���ӵ� x �� ������(accread)�� ��ȯ
    //    ��, �о�� ���� 0x7FFFU���� ũ�� �״�� ����ϰ�, �۴ٸ� �����Ͽ� ��ȣȭ
    if(adcax0 > 0x7FFFU)
    {
        result_a0 = adcax0 - 0x7FFFU;
        accread[0] = (int16_t)result_a0;
    }
    else
    {
        result_a0 = 0x7FFFU - adcax0;
        accread[0] = (int16_t)result_a0 * -1;
    }

    // 7. ADCB����� SOC ��ȣ 1�� ����� �о� ���ӵ� y �� ������(accread)�� ��ȯ
    //    ��, �о�� ���� 0x7FFFU���� ũ�� �״�� ����ϰ�, �۴ٸ� �����Ͽ� ��ȣȭ
    if(adcax1 > 0x7FFFU)
    {
        result_a1 = adcax1 - 0x7FFFU;
        accread[1] = (int16_t)result_a1;
    }
    else
    {
        result_a1 = 0x7FFFU - adcax1;
        accread[1] = (int16_t)result_a1 * -1;
    }

    // 8. ADCB����� SOC ��ȣ 2�� ����� �о� ���ӵ� z �� ������(accread)�� ��ȯ
    //    ��, �о�� ���� 0x7FFFU���� ũ�� �״�� ����ϰ�, �۴ٸ� �����Ͽ� ��ȣȭ

    accread[2] = 0;


	// 9. ADCD�� ADCA ����� ��� ���� �о�, ����, �µ� ��� ���� �߰����� ������ gadcvalue6���� gadcvalue10�� ����
#ifdef NEW_BOARD
    gadcvalue6 = (uint16_t)ADC_readPPBResult(ADCARESULT_BASE, ADC_PPB_NUMBER1);
    gadcvalue7 = (uint16_t)ADC_readPPBResult(ADCARESULT_BASE, ADC_PPB_NUMBER2);
    gadcvalue8 = (uint16_t)ADC_readPPBResult(ADCARESULT_BASE, ADC_PPB_NUMBER3);
    gadcvalue9 = (uint16_t)ADC_readPPBResult(ADCARESULT_BASE, ADC_PPB_NUMBER4);
    gadcvalue10 = (uint16_t)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4);


#else

    gadcvalue6 = (uint32_t)ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER0);
    gadcvalue7 = (uint32_t)ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER1);
    gadcvalue8 = (uint32_t)ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER2);
    gadcvalue9 = (uint32_t)ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER3);

#endif

	// 10. ema ���͸� ���� �ڱ��� ������ �� ������ ���� 
#ifdef NEW_BOARD
    gBxADC = (int16_t)fluxread[0];  // x
    gByADC = (int16_t)fluxread[1];  // y
    gBzADC = (int16_t)fluxread[2];  // z
#else
    gBxADC = (int16_t)fluxread[2];  // x
    gByADC = (int16_t)fluxread[1];  // y
    gBzADC = (int16_t)fluxread[0];
#endif

    gAxADC = (int16_t)accread[0];
    gAyADC = (int16_t)accread[1];
    gAzADC = (int16_t)accread[2];

	// 11. �� ���� ������ �� ���� ���� ema_filter �Լ��� �̿��� ���� �̵� ��� ���͸� �����Ͽ� ema �迭�� ����.
    ema[ADC_CH_INDEX_FLUX_X] = ema_filter((float64_t)gBxADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_FLUX_X]);
    ema[ADC_CH_INDEX_FLUX_Y] = ema_filter((float64_t)gByADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_FLUX_Y]);
    ema[ADC_CH_INDEX_FLUX_Z] = ema_filter((float64_t)gBzADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_FLUX_Z]);
    ema[ADC_CH_INDEX_ACCEL_X] = ema_filter((float64_t)gAxADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_ACCEL_X]);
    ema[ADC_CH_INDEX_ACCEL_Y] = ema_filter((float64_t)gAyADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_ACCEL_Y]);
    ema[ADC_CH_INDEX_ACCEL_Z] = ema_filter((float64_t)gAzADC, EMA_FILTER_ALPHA, ema[ADC_CH_INDEX_ACCEL_Z]);

    ema[ADC_CH_INDEX_VOLYAGE_28V] = ema_filter((float64_t)gadcvalue6, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_28V]);
    ema[ADC_CH_INDEX_VOLYAGE_5V] = ema_filter((float64_t)gadcvalue7, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_5V]);
    ema[ADC_CH_INDEX_VOLYAGE_3p3V] = ema_filter((float64_t)gadcvalue8, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_3p3V]);
    ema[ADC_CH_INDEX_VOLYAGE_1p2V] = ema_filter((float64_t)gadcvalue9, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_1p2V]);
    ema[ADC_CH_INDEX_VOLYAGE_TEMPERATUR] = ema_filter((float64_t)gadcvalue10, EMA_FILTER_ALPHA_VOLT, ema[ADC_CH_INDEX_VOLYAGE_TEMPERATUR]);

	// 12. ���ͷ�Ʈ �÷��׸� Ŭ�����ϰ�, �����÷ο� �߻� ���θ� Ȯ��
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    if(TRUE == ADC_getInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1))
    {
		//12.1  �����÷ο찡 �߻����� ��� ���ͷ�Ʈ �����÷ο� �÷��׿� ���¸� �ʱ�ȭ
        ADC_clearInterruptOverflowStatus(ADCA_BASE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    }

	// 13. �׷� 1 ACK ��ȣ�� Ŭ�����Ͽ� ���� ���ͷ�Ʈ�� ������ �� �ֵ��� �غ�
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


/*
��ɼ���
 ���� �̵� ���(EMA, Exponential Moving Average) ���͸� �����Ͽ� ADC ���� ���͸� �ϴ� �Լ�

����� ����
float64_t adc : ���� ������ ADC ��
float64_t alpha : ���� ���� ���� ��
int16_t previous_ema_filtered_value : ���� ���͵� ��
*/
static int16_t ema_filter(float64_t adc, float64_t alpha, int16_t previous_ema_filtered_value)
{
	float64_t new_ema_filtered_value;

	// 1. �����̵����(EMA) 
	// ���� : EMAnew=(1?��)��ADC+�᡿EMAprev
	new_ema_filtered_value  = ((1.0 - alpha) * adc) + (alpha * (float64_t)previous_ema_filtered_value);

	// 2. ���͵� �� ����
	return 	(int16_t)new_ema_filtered_value;
}

/*
 ��ɼ���
 ADC ä�� ��ó�� �ɼ� ������ ����

    ���        28        5     3.3             1.2
    ������(ADC)  1640      0     1470            1600
               0.4004    0     0.35889         0.3906

    �̷а�       1.3308    0     1.0985          1.2
               0.4436          0.366166667     0.4
    �̷�adc     1816.9856 0     1499.818667     1638.4
    �̷а��������� 176.9856  0     29.81866667     38.4

    ���� ���

    1. �켱 ���� ���� �̷а��� ���к� ����Ѵ�.
    2. ���� �� ���к� ADC���� �д´�.
    3. �̷а��� �������� ADC ���̰��� ����Ѵ�.
    4. ���̰��� ADC_PPB �������� �Է��Ѵ�.
    5. ���� ���̰��� ����̸� �������� ������ �Է��Ѵ�.
    ������ �ݴ�� ����� �Է��Ѵ�.
*/

void setupPPBOffset(void)
{
    // 1. ADCA CH0�� ��ó�� ���(PPB)�� ����
    ADC_setupPPB(ADCA_BASE, ADC_PPB_NUMBER1, ADC_SOC_NUMBER0);
    // 2. ADCA CH0 ��ó�� �� �ɼ� ������ ����
    ADC_setPPBCalibrationOffset(ADCA_BASE, ADC_PPB_NUMBER1, -177);

    // 1. ADCA CH1 ��ó�� ���(PPB)�� ����
    ADC_setupPPB(ADCA_BASE, ADC_PPB_NUMBER2, ADC_SOC_NUMBER1);
    // 2. ADCA CH1 ��ó�� �� �ɼ� ������ ����
    ADC_setPPBCalibrationOffset(ADCA_BASE, ADC_PPB_NUMBER2, -90);

    // 1. ADCA CH2�� ��ó�� ���(PPB)�� ����
    ADC_setupPPB(ADCA_BASE, ADC_PPB_NUMBER3, ADC_SOC_NUMBER2);
    // 2. ADCA CH2 ��ó�� �� �ɼ� ������ ����
    ADC_setPPBCalibrationOffset(ADCA_BASE, ADC_PPB_NUMBER3, -30);

    // 1. ADCA CH3�� ��ó�� ���(PPB)�� ����
    ADC_setupPPB(ADCA_BASE, ADC_PPB_NUMBER4, ADC_SOC_NUMBER3);
    // 2. ADCA CH3 ��ó�� �� �ɼ� ������ ����
    ADC_setPPBCalibrationOffset(ADCA_BASE, ADC_PPB_NUMBER4, -38);

}

