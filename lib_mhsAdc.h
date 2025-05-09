#ifndef LIB_MHSADC_H_
#define LIB_MHSADC_H_

#include "driverlib.h"
#include "device.h"
#include "board.h"
#include <math.h>
#include "init.h"

#define EX_ADC_RESOLUTION   16

#define VOLTAGE_28          (0.93f)   // 24.52V ¿œ∂ß.
#define VOLTAGE_5           (0.72f)
#define VOLTAGE_3P3         (0.92f)
#define VOLTAGE_1P2         (1.27f)

enum {
	ADC_CH_INDEX_FLUX_X = 0,
	ADC_CH_INDEX_FLUX_Y,
	ADC_CH_INDEX_FLUX_Z,
	ADC_CH_INDEX_ACCEL_X,
	ADC_CH_INDEX_ACCEL_Y,
	ADC_CH_INDEX_ACCEL_Z,
	ADC_CH_INDEX_VOLYAGE_28V,
	ADC_CH_INDEX_VOLYAGE_5V,
	ADC_CH_INDEX_VOLYAGE_3p3V,
	ADC_CH_INDEX_VOLYAGE_1p2V,
	ADC_CH_INDEX_VOLYAGE_TEMPERATUR,
	ADC_CH_INDEX_MAX
};

 __interrupt void adcA1ISR(void);
 void initADCSOC(void);
void configureADC(uint32_t adcBase, uint16_t bittype);
void setupPPBOffset(void);

extern uint16_t ema[ADC_CH_INDEX_MAX];

#if 0

extern int16_t ema_0;
extern int16_t ema_1;
extern int16_t ema_2;
extern int16_t ema_3;
extern int16_t ema_4;
extern int16_t ema_5;
extern int16_t ema_6;
extern int16_t ema_7;
extern int16_t ema_8;
extern int16_t ema_9;
extern int16_t ema_10;

extern uint16_t gadcvalue28V;
extern uint16_t gadcvalue5V;
extern uint16_t gadcvalue3V3;
extern uint16_t gadcvalue1V2;
#endif


#if 0
extern float64_t magResolution;
extern void Matrix_flux(float64_t a[3][3], const float64_t b[3]);
extern float64_t acceResolution;
extern int32_t average[6];
extern float64_t Result_calibration[3];
#endif

#endif /* LIB_MHSADC_H_ */
