/*
 * csu._angle_calculation.c
 *
 * 기능 : 측정된 자기장, 가속도 데이터를 이용하여 방위각을 계산하는 기능
 * 구성요소 : 방위각 계산 CSU (D-MHS-SFR-013)
 * 이력 :
 *    2024.05.23 : 이충순 : 초기 작성 -
 */

#include <math.h>
#include "mhs_project.h"

static float64_t tiltAngle(float64_t gx, float64_t gy, float64_t bx, float64_t by, float64_t bz);

// 기능 : 정북 기준 방위각 계산
// 입력
//      accelx,  accely : 가속도 x축, y축 , 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
// 		bx, by, bz      :자기장 x,y,z 축  단위 gauss, 정밀도 100nT , 범위    0 ~ 10000,
// 출력 : 정북기준 방위각 리턴 : 단위 degress , 정밀도 0.1 degrees , 범위 0 ~ 359.9
// tiltAngle() 함수를 이용하여 방위각 계산하고 방위각이 infinite 인경우
uint16_t angle_calculation(float64_t accelx,float64_t accely, float64_t bx, float64_t by, float64_t bz)
{
    uint16_t angle =0;
    float64_t ayaw_radian = 0.0L;

	// 1. tiltAngle 을 이용하여 레디안 방위각 계산
	//    tiltAngle()의 결과 값의 범위는 positive   값임.
    ayaw_radian = tiltAngle(accelx,accely,bx,by,bz);
	
	// 2. 수신된 레이안 방위각이 정상 범위일 경우 degrees로 변환 후 10을 곱하여 소수 첫째자리까지 정수로 표현하도록 변환, 
	//    단 수신한 방위각이 비정상일 경우 방위각은 0으로 한다.
    if(isfinite(ayaw_radian))
    {
        float64_t fa = ayaw_radian*(180.0L/PI) * 10.0L;
        angle = (uint16_t)fa;  // fa 는 양수임
    }

    return angle;
}


// 기능 : 라디안 방위각 계산 함수
// 입력
//      gx,  gy : 가속도 x축, y축 , 단위 g, 정밀도  , 범위    0 ~  +/- 1.6g
//      bx, by, bz      :자기장 x,y,z 축  단위 gauss, 정밀도 100nT , 범위    0 ~ 10000,
// 출력 : 라디안 리턴 : 단위 라디안 , 범위 0 ~ 2 pi
static float64_t tiltAngle(float64_t gx, float64_t gy, float64_t bx, float64_t by, float64_t bz)
{
    float64_t Ay = (0.0L);
    static float64_t lastAy = 0.0L;
    float64_t Brx = 0.0L;
    float64_t Bry = 0.0L;
    float32_t atanB = 0.0;
    float64_t math_r = 0.0L;
    float64_t Arm = 0.0L;


    // 1. x축 가속도에 따른 기울기 계산
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

    // 2.  Apm 각도에 기반한 z축 보정
    float64_t testAp = (float64_t)(sqrt(1.0-pow((float32_t)gx, 2.0)));
    float64_t cosAp = gy / testAp;

    // 3. 기울기 보정된 자기장 벡터 Brx, Bry 계산
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

    // 4. atan 함수와 조건문을 통해 기울기 Ay를 계산
    //    Bnx > 0.0: Bnx가 양수일 때는 atan(Bny / Bnx)를 그대로 사용
    //    Bnx < 0.0와 Bny >= 0.0: Bnx가 음수이면서 Bny가 양수일 때는 180도(π)를 더함.
    //    Bnx < 0.0와 Bny <= 0.0: Bnx와 Bny 모두 음수일 때는 180도(π)를 더해 각도를 조정
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

    // 5. 기울기 리런 : 범위 (0 ~ 2π)
    return Ay;
}

