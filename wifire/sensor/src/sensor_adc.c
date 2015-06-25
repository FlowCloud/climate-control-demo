/**************************************************************************************************
	Copyright (c) 2015, Imagination Technologies Limited
	All rights reserved.
	Redistribution and use of the Software in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:
	1. The Software (including after any modifications that you make to it) must support
	   the FlowCloud Web Service API provided by Licensor and accessible at http://ws-uat.flowworld.com
	   and/or some other location(s) that we specify.
	2. Redistributions of source code must retain the above copyright notice, this list of
	   conditions and the following disclaimer.
	3. Redistributions in binary form must reproduce the above copyright notice, this list
	   of conditions and the following disclaimer in the documentation and/or other materials
	   provided with the distribution.
	4. Neither the name of the copyright holder nor the names of its contributors may be used
	   to endorse or promote products derived from this Software without specific prior written permission.
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
	IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
	THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************************************/

#include "sensor_adc.h"
#include "adc_custom.h"

#define  WIFIRE_POTENTIOMETER_PIN (12)

void Sensor_ADC_Init(void)
{
	//ADC setup
	adc12_Init();
	adc12_ConfigPort(WIFIRE_POTENTIOMETER_PIN);
	adc12_Enable();
}


bool Sensor_ADC_ReadHumidityAndTemperature(float* const humidity, float* const temperature)
{
	return Sensor_ADC_ReadTemperature(temperature) && Sensor_ADC_ReadHumidity(humidity);
}

bool Sensor_ADC_ReadTemperature(float* const temperature)
{
	unsigned int adcValue =  adc12_Read(WIFIRE_POTENTIOMETER_PIN);
	*temperature = adcValue * 3.3 / 4095;
	//Linear approximation, value will be in the range of -25 to 24.5
	*temperature *= 15;
	*temperature -= 25;
	return true;
}

bool Sensor_ADC_ReadHumidity(float* const humidity)
{
	unsigned int adcValue =  adc12_Read(WIFIRE_POTENTIOMETER_PIN);
	*humidity = adcValue * 3.3 / 4095;
	//Linear approximation, value will be in the range of 0 to 99
	*humidity *= 30;
	return true;
}
