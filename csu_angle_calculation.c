/*
 * csu._angle_calculation.c
 *
 * ��� : ������ �ڱ���, ���ӵ� �����͸� �̿��Ͽ� �������� ����ϴ� ���
 * ������� : ������ ��� CSU (D-MHS-SFR-013)
 * �̷� :
 *    2024.05.23 : ����� : �ʱ� �ۼ� -
 */

#include <math.h>
#include "mhs_project.h"

static float64_t tiltAngle(float64_t gx, float64_t gy, float64_t bx, float64_t by, float64_t bz);

// ��� : ���� ���� ������ ���
// �Է�
//      accelx,  accely : ���ӵ� x��, y�� , ���� g, ���е�  , ����    0 ~  +/- 1.6g
// 		bx, by, bz      :�ڱ��� x,y,z ��  ���� gauss, ���е� 100nT , ����    0 ~ 10000,
// ��� : ���ϱ��� ������ ���� : ���� degress , ���е� 0.1 degrees , ���� 0 ~ 359.9
// tiltAngle() �Լ��� �̿��Ͽ� ������ ����ϰ� �������� infinite �ΰ��
uint16_t angle_calculation(float64_t accelx,float64_t accely, float64_t bx, float64_t by, float64_t bz)
{
    uint16_t angle =0;
    float64_t ayaw_radian = 0.0L;

	// 1. tiltAngle �� �̿��Ͽ� ����� ������ ���
	//    tiltAngle()�� ��� ���� ������ positive   ����.
    ayaw_radian = tiltAngle(accelx,accely,bx,by,bz);
	
	// 2. ���ŵ� ���̾� �������� ���� ������ ��� degrees�� ��ȯ �� 10�� ���Ͽ� �Ҽ� ù°�ڸ����� ������ ǥ���ϵ��� ��ȯ, 
	//    �� ������ �������� �������� ��� �������� 0���� �Ѵ�.
    if(isfinite(ayaw_radian))
    {
        float64_t fa = ayaw_radian*(180.0L/PI) * 10.0L;
        angle = (uint16_t)fa;  // fa �� �����
    }

    return angle;
}


// ��� : ���� ������ ��� �Լ�
// �Է�
//      gx,  gy : ���ӵ� x��, y�� , ���� g, ���е�  , ����    0 ~  +/- 1.6g
//      bx, by, bz      :�ڱ��� x,y,z ��  ���� gauss, ���е� 100nT , ����    0 ~ 10000,
// ��� : ���� ���� : ���� ���� , ���� 0 ~ 2 pi
static float64_t tiltAngle(float64_t gx, float64_t gy, float64_t bx, float64_t by, float64_t bz)
{
    float64_t Ay = (0.0L);
    static float64_t lastAy = 0.0L;
    float64_t Brx = 0.0L;
    float64_t Bry = 0.0L;
    float32_t atanB = 0.0;
    float64_t math_r = 0.0L;
    float64_t Arm = 0.0L;


    // 1. x�� ���ӵ��� ���� ���� ���
    float64_t Apm = (float64_t)(asin((float32_t)gx));
    if(Apm == 0.0L)
    {
        Arm = 0.0L;
    }
    else
    {
        math_r = gy/(float64_t)cos((float32_t)Apm);
        Arm = (float64_t)(-asin((float32_t)math_r));
    }

    // 2.  Apm ������ ����� z�� ����
    float64_t testAp = (float64_t)(sqrt(1.0-pow((float32_t)gx, 2.0)));
    float64_t cosAp = gy / testAp;

    // 3. ���� ������ �ڱ��� ���� Brx, Bry ���
    Brx = (float64_t)(sqrt(1.0-pow((float32_t)gx, 2.0))*bx) - (float64_t)(gx*cosAp*by) + (float64_t)(gx*sqrt(1.0-pow((float32_t)cosAp, 2.0))*bz);
    Bry = (float64_t)(sqrt(1.0-pow((float32_t)cosAp, 2.0))*by) + (cosAp*bz);

    if((Brx == 0.0L ) || (Bry == 0.0L))
    {
        atanB = 0.0;
    }
    else
    {
       atanB = (float32_t)(Bry / Brx);
    }

    // 4. atan �Լ��� ���ǹ��� ���� ���� Ay�� ���
    //    Bnx > 0.0: Bnx�� ����� ���� atan(Bny / Bnx)�� �״�� ���
    //    Bnx < 0.0�� Bny >= 0.0: Bnx�� �����̸鼭 Bny�� ����� ���� 180��(��)�� ����.
    //    Bnx < 0.0�� Bny <= 0.0: Bnx�� Bny ��� ������ ���� 180��(��)�� ���� ������ ����
    if(Brx > 0.0L)
    {
        Ay = (float64_t)atan((atanB));
    }
    else if((Brx < 0.0L) && (Bry >= 0.0L))
    {
        Ay = (float64_t)atan((atanB)) + 3.14159265359L;
    }
    else if((Brx < 0.0L) && (Bry <= 0.0L))
    {
        Ay = 3.14159265359L + (float64_t)atan((atanB));
    }
    else
    {
        Ay = lastAy;
    }

    if(Ay < 0.0L)
    {
      Ay = (2.0L*3.14159265359L) + Ay;
    }

    lastAy = Ay;

    // 5. ���� ���� : ���� (0 ~ 2��)
    return Ay;
}

