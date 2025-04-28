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


// ADC �ػ� �� �Է� ��ȣ ��� ���� �Լ� 
void
ADC_setMode(uint32_t base, ADC_Resolution resolution,
            ADC_SignalMode signalMode)
{
    // 1. base �ּҰ� ��ȿ�� ADC ����� �⺻ �ּ����� Ȯ��
    ASSERT(ADC_isBaseValid(base));

    //
    // Check for correct signal mode & resolution. In this device:
    // Single ended signal conversions are supported in 12-bit mode only
    // Differential signal conversions are supported in 16-bit mode only
    //

	// 2. ADC �Է� ��� �� ���ش� ������ �´��� Ȯ�� 
    if(signalMode == ADC_MODE_SINGLE_ENDED)
    {
        ASSERT(resolution == ADC_RESOLUTION_12BIT);
    }
    else
    {
        ASSERT(resolution == ADC_RESOLUTION_16BIT);
    }


    //
    // 3. base �ּҿ� ���ش� �� �Է� ��带 ����
    //
    EALLOW;
    HWREGH(base + ADC_O_CTL2) = (HWREGH(base + ADC_O_CTL2) &
                                 ~(ADC_CTL2_RESOLUTION | ADC_CTL2_SIGNALMODE)) |
                                ((uint16_t)resolution | (uint16_t)signalMode);
    EDIS;

    // 4. ADC�� ������ ����(INL)�� ������ Ʈ�� ���� 
    ADC_setINLTrim(base);
    ADC_setOffsetTrim(base);
}


// ADC ����� ������(INL, Integral Non-Linearity) Ʈ�� ������ �����ϴ� �Լ�
void
ADC_setINLTrim(uint32_t base)
{
    ADC_Resolution resolution;

    // 1. base �ּҰ� ��ȿ�� ADC ����� �⺻ �ּ����� Ȯ�� 
    ASSERT(ADC_isBaseValid(base));

	// 2. ADC�� ���ش� ���� 
	resolution = (ADC_Resolution)
                 (HWREGH(base + ADC_O_CTL2) & ADC_CTL2_RESOLUTION);


	// 3. �� ADC ���(ADCA, ADCB, ADCC, ADCD)���� OTP �޸𸮿� ����� INL Ʈ�� �Լ��� ȣ���Ͽ�, ������ ����
	EALLOW;
    switch(base)
    {
        case ADCA_BASE:
			// 3.1  ADCA OTP �޸� �ּҿ� ����� Ʈ�� �Լ� ������ Ȯ�� 
            if(HWREGH(ADC_calADCAINL) != 0xFFFFU)
            {
				//3.1.1 ADC_calADCAINL() �Լ��� ȣ���Ͽ� INL Ʈ��
                (*((void (*)(void))ADC_calADCAINL))();
            }
            else
            {
				//3.1.2 �̼��� 
            }
            break;
        case ADCB_BASE:
			
			// 3.2  ADCB OTP �޸� �ּҿ� ����� Ʈ�� �Լ� ������ Ȯ�� 
            if(HWREGH(ADC_calADCBINL) != 0xFFFFU)
            {
				//3.2.1 ADC_calADCBINL() �Լ��� ȣ���Ͽ� INL Ʈ��
                (*((void (*)(void))ADC_calADCBINL))();
            }
            else
            {
				//3.2.2 �̼��� 
            }
            break;
        case ADCC_BASE:
			// 3.3  ADCC OTP �޸� �ּҿ� ����� Ʈ�� �Լ� ������ Ȯ�� 
            if(HWREGH(ADC_calADCCINL) != 0xFFFFU)
            {
				//3.3.1 ADC_calADCCINL() �Լ��� ȣ���Ͽ� INL Ʈ��
                (*((void (*)(void))ADC_calADCCINL))();
            }
            else
            {
				//3.3.2 �̼��� 

            }
            break;
        case ADCD_BASE:
			// 3.4  ADCD OTP �޸� �ּҿ� ����� Ʈ�� �Լ� ������ Ȯ�� 
            if(HWREGH(ADC_calADCDINL) != 0xFFFFU)
            {
				//3.4.1 ADC_calADCDINL() �Լ��� ȣ���Ͽ� INL Ʈ��
                (*((void (*)(void))ADC_calADCDINL))();
            }
            else
            {
				//3.4.2 �̼��� 
            }
            break;
        default:
            break;
    }

    //
    // 4. 12-bit ADC ���شɿ� ���� Ʈ�� ���� 
    //
    if(resolution == ADC_RESOLUTION_12BIT)
    {
        // 4.1 12-bit ���� Ʈ�� ����
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
