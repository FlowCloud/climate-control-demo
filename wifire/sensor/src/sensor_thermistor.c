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
	//Inspired by DHT library under MIT license by Seeed Studio

*******************************************************************************************************/

#include "sensor_thermistor.h"
#include "adc_custom.h"
#include <math.h>
#include "peripheral/ports/plib_ports.h"

/*
 * Driver for this NTC thermistor sensor connected to grove port A2 on wifire board
 * http://www.seeedstudio.com/wiki/Grove_-_Temperature_Sensor
 * http://www.seeedstudio.com/wiki/images/a/a1/NCP18WF104F03RC.pdf
 * 
 */

#define THERMISTOR_INPUT_ADC_NUM	(2)
#define B_CONSTANT					(4200)

void Sensor_Thermistor_Init(void)
{
	//ADC setup
	adc12_Init();
	adc12_ConfigPort(THERMISTOR_INPUT_ADC_NUM);
	adc12_Enable();

	//TBD: temporary fix for first reading being wrong
	adc12_Read(THERMISTOR_INPUT_ADC_NUM);
}

/*
 *  Formula derived from the datasheet
 *		R=R0 expB (1/T-1/T0)
 *		R: Resistance in ambient temperature T (K) (K: absolute temperature)
 *		R0: Resistance in ambient temperature T0 (K)
 *		B: B-Constant of Thermistor
 *
 *		B=ln (R/R0) / (1/T-1/T0)
 */

bool Sensor_Thermistor_ReadTemperature(float *const temperature)
{
	//TBD: temporary fix for first reading being wrong
	adc12_Read(THERMISTOR_INPUT_ADC_NUM);
	unsigned int adcValue =  adc12_Read(THERMISTOR_INPUT_ADC_NUM);
	float resistance=(float)((4096-adcValue)*100000)/(adcValue); //get the resistance of the sensor;
	*temperature=1/(log(resistance/100000)/B_CONSTANT+1/298.15)-273.15;//convert to temperature via datasheet ;
	return true;
}
