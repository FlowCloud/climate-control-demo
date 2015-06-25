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

#ifndef SENSOR_H
#define	SENSOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

#ifdef USE_ADC_SENSOR
	#include "sensor_adc.h"

	#define Sensor_Init Sensor_ADC_Init
	#define Sensor_ReadTemperature Sensor_ADC_ReadTemperature
	#define Sensor_ReadHumidity Sensor_ADC_ReadHumidity
	#define Sensor_ReadHumidityAndTemperature Sensor_ADC_ReadHumidityAndTemperature

#endif	/* USE_ADC_SENSOR */

#ifdef USE_DHT_SENSOR
	#include "sensor_dht.h"

	#define Sensor_Init Sensor_DHT_Init
	#define Sensor_ReadTemperature Sensor_DHT_ReadTemperature
	#define Sensor_ReadHumidity Sensor_DHT_ReadHumidity
	#define Sensor_ReadHumidityAndTemperature Sensor_DHT_ReadHumidityAndTemperature
#endif	/* USE_DHT_SENSOR */

#ifdef USE_THERMISTOR_SENSOR
	#include "sensor_thermistor.h"

	#define Sensor_Init Sensor_Thermistor_Init
	#define Sensor_ReadTemperature Sensor_Thermistor_ReadTemperature
	static inline bool Sensor_ReadHumidity(float* const humidity)
	{
		*humidity = 0.0f;
		return true;
	}
	static inline bool Sensor_ReadHumidityAndTemperature(float* const humidity, float* const temperature)
	{
		return Sensor_Thermistor_ReadTemperature(temperature) && Sensor_ReadHumidity(humidity);
	}

#endif	/* USE_THERMISTOR_SENSOR */

#ifdef USE_THERMISTOR_AND_DHT_SENSOR
	#include "sensor_thermistor.h"
	#include "sensor_dht.h"
	#define Sensor_Init Sensor_Thermistor_Init
	#define Sensor_ReadTemperature Sensor_Thermistor_ReadTemperature
	#define Sensor_ReadHumidity Sensor_DHT_ReadHumidity

#endif	/* USE_THERMISTOR_SENSOR */
#ifdef	__cplusplus
}
#endif


#endif //SENSOR_H
