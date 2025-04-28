//###########################################################################
//
// FILE:   adc.c
//
// TITLE:  C28x ADC driver.
//
//###########################################################################
// $Copyright:
// Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//###########################################################################

#include "adc.h"

//*****************************************************************************
//
// Defines for locations of ADC calibration functions in OTP for use in
// ADC_setMode() ONLY. Not intended for use by application code.
//
//*****************************************************************************
//
// The following functions calibrate the ADC linearity.  Use them in the
// ADC_setMode() function only.
//
#define ADC_calADCAINL          0x0703B4U
#define ADC_calADCBINL          0x0703B2U
#define ADC_calADCCINL          0x0703B0U
#define ADC_calADCDINL          0x0703AEU

//
// This function looks up the ADC offset trim for a given condition. Use this
// in the ADC_setMode() function only.
//
#define ADC_getOffsetTrim       0x0703ACU


// ADC 해상도 및 입력 신호 모드 설정 함수 
void
ADC_setMode(uint32_t base, ADC_Resolution resolution,
            ADC_SignalMode signalMode)
{
    // 1. base 주소가 유효한 ADC 모듈의 기본 주소인지 확인
    ASSERT(ADC_isBaseValid(base));

    //
    // Check for correct signal mode & resolution. In this device:
    // Single ended signal conversions are supported in 12-bit mode only
    // Differential signal conversions are supported in 16-bit mode only
    //

	// 2. ADC 입력 모드 와 분해능 설정이 맞는지 확인 
    if(signalMode == ADC_MODE_SINGLE_ENDED)
    {
        ASSERT(resolution == ADC_RESOLUTION_12BIT);
    }
    else
    {
        ASSERT(resolution == ADC_RESOLUTION_16BIT);
    }


    //
    // 3. base 주소에 분해능 및 입력 모드를 설정
    //
    EALLOW;
    HWREGH(base + ADC_O_CTL2) = (HWREGH(base + ADC_O_CTL2) &
                                 ~(ADC_CTL2_RESOLUTION | ADC_CTL2_SIGNALMODE)) |
                                ((uint16_t)resolution | (uint16_t)signalMode);
    EDIS;

    // 4. ADC의 비선형성 보정(INL)과 오프셋 트림 보정 
    ADC_setINLTrim(base);
    ADC_setOffsetTrim(base);
}


// ADC 모듈의 비선형성(INL, Integral Non-Linearity) 트림 보정을 수행하는 함수
void
ADC_setINLTrim(uint32_t base)
{
    ADC_Resolution resolution;

    // 1. base 주소가 유효한 ADC 모듈의 기본 주소인지 확인 
    ASSERT(ADC_isBaseValid(base));

	// 2. ADC의 분해능 설정 
	resolution = (ADC_Resolution)
                 (HWREGH(base + ADC_O_CTL2) & ADC_CTL2_RESOLUTION);


	// 3. 각 ADC 모듈(ADCA, ADCB, ADCC, ADCD)별로 OTP 메모리에 저장된 INL 트림 함수를 호출하여, 비선형성 보정
	EALLOW;
    switch(base)
    {
        case ADCA_BASE:
			// 3.1  ADCA OTP 메모리 주소에 저장된 트림 함수 포인터 확인 
            if(HWREGH(ADC_calADCAINL) != 0xFFFFU)
            {
				//3.1.1 ADC_calADCAINL() 함수를 호출하여 INL 트림
                (*((void (*)(void))ADC_calADCAINL))();
            }
            else
            {
				//3.1.2 미수행 
            }
            break;
        case ADCB_BASE:
			
			// 3.2  ADCB OTP 메모리 주소에 저장된 트림 함수 포인터 확인 
            if(HWREGH(ADC_calADCBINL) != 0xFFFFU)
            {
				//3.2.1 ADC_calADCBINL() 함수를 호출하여 INL 트림
                (*((void (*)(void))ADC_calADCBINL))();
            }
            else
            {
				//3.2.2 미수행 
            }
            break;
        case ADCC_BASE:
			// 3.3  ADCC OTP 메모리 주소에 저장된 트림 함수 포인터 확인 
            if(HWREGH(ADC_calADCCINL) != 0xFFFFU)
            {
				//3.3.1 ADC_calADCCINL() 함수를 호출하여 INL 트림
                (*((void (*)(void))ADC_calADCCINL))();
            }
            else
            {
				//3.3.2 미수행 

            }
            break;
        case ADCD_BASE:
			// 3.4  ADCD OTP 메모리 주소에 저장된 트림 함수 포인터 확인 
            if(HWREGH(ADC_calADCDINL) != 0xFFFFU)
            {
				//3.4.1 ADC_calADCDINL() 함수를 호출하여 INL 트림
                (*((void (*)(void))ADC_calADCDINL))();
            }
            else
            {
				//3.4.2 미수행 
            }
            break;
        default:
            break;
    }

    //
    // 4. 12-bit ADC 분해능에 대한 트림 보정 
    //
    if(resolution == ADC_RESOLUTION_12BIT)
    {
        // 4.1 12-bit 선형 트림 설정
        HWREG(base + ADC_O_INLTRIM1) &= 0xFFFF0000U;
        HWREG(base + ADC_O_INLTRIM2) &= 0xFFFF0000U;
        HWREG(base + ADC_O_INLTRIM4) &= 0xFFFF0000U;
        HWREG(base + ADC_O_INLTRIM5) &= 0xFFFF0000U;
    }
    EDIS;
}

//*****************************************************************************
//
// ADC_setOffsetTrim
//
//*****************************************************************************
void
ADC_setOffsetTrim(uint32_t base)
{
    uint16_t offsetIndex = 0U;
    uint16_t offsetTrim  = 0U;
    ADC_Resolution resolution;
    ADC_SignalMode signalMode;

    //
    // Check the arguments.
    //
    ASSERT(ADC_isBaseValid(base));

    resolution = (ADC_Resolution)
                 (HWREGH(base + ADC_O_CTL2) & ADC_CTL2_RESOLUTION);
    signalMode = (ADC_SignalMode)
                 (HWREGH(base + ADC_O_CTL2) & ADC_CTL2_SIGNALMODE);

    switch(base)
    {
        case ADCA_BASE:
            offsetIndex = (uint16_t)(0U * 4U);
            break;
        case ADCB_BASE:
            offsetIndex = (uint16_t)(1U * 4U);
            break;
        case ADCC_BASE:
            offsetIndex = (uint16_t)(2U * 4U);
            break;
        case ADCD_BASE:
            offsetIndex = (uint16_t)(3U * 4U);
            break;
        default:
            //
            // Invalid base address!
            //
            offsetIndex = 0U;
            break;
    }

    //
    // Offset trim function is programmed into OTP, so call it
    //
    if(HWREGH(ADC_getOffsetTrim) != 0xFFFFU)
    {
        //
        // Calculate the index into OTP table of offset trims and call
        // function to return the correct offset trim
        //
        offsetIndex += ((signalMode == ADC_MODE_DIFFERENTIAL) ? 1U : 0U) +
                       (2U * ((resolution == ADC_RESOLUTION_16BIT) ? 1U : 0U));

        offsetTrim =
            (*((uint16_t (*)(uint16_t index))ADC_getOffsetTrim))(offsetIndex);
    }
    else
    {
        //
        // Offset trim function is not populated, so set offset trim to 0
        //
        offsetTrim = 0U;
    }

    //
    // Apply the offset trim. Offset Trim is not updated here in case of TMX or
    // untrimmed devices. The default trims for TMX devices should be handled in
    // Device_init(). Refer to Device_init() and Device_configureTMXAnalogTrim()
    // APIs for more details.
    //
    if(offsetTrim > 0x0U)
    {
        EALLOW;
        HWREGH(base + ADC_O_OFFTRIM) = offsetTrim;
        EDIS;
    }
}


//*****************************************************************************
//
// ADC_setPPBTripLimits
//
//*****************************************************************************
void
ADC_setPPBTripLimits(uint32_t base, ADC_PPBNumber ppbNumber,
                     int32_t tripHiLimit, int32_t tripLoLimit)
{
    uint32_t ppbHiOffset;
    uint32_t ppbLoOffset;

    //
    // Check the arguments.
    //
    ASSERT(ADC_isBaseValid(base));
    ASSERT((tripHiLimit <= 65535) && (tripHiLimit >= -65536));
    ASSERT((tripLoLimit <= 65535) && (tripLoLimit >= -65536));

    //
    // Get the offset to the appropriate trip limit registers.
    //
    ppbHiOffset = (ADC_PPBxTRIPHI_STEP * (uint32_t)ppbNumber) +
                  ADC_O_PPB1TRIPHI;
    ppbLoOffset = (ADC_PPBxTRIPLO_STEP * (uint32_t)ppbNumber) +
                  ADC_O_PPB1TRIPLO;

    EALLOW;

    //
    // Set the trip high limit.
    //
    HWREG(base + ppbHiOffset) =
        (HWREG(base + ppbHiOffset) & ~ADC_PPBTRIP_MASK) |
        ((uint32_t)tripHiLimit & ADC_PPBTRIP_MASK);

    //
    // Set the trip low limit.
    //
    HWREG(base + ppbLoOffset) =
        (HWREG(base + ppbLoOffset) & ~ADC_PPBTRIP_MASK) |
        ((uint32_t)tripLoLimit & ADC_PPBTRIP_MASK);

    EDIS;
}
