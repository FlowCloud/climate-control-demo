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

#ifndef SENSOR_DHT_H
#define	SENSOR_DHT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define SENSOR_PIN_READ() PLIB_PORTS_PinGet(PORTS_ID_0, DHT_SENSOR_PORT, DHT_SENSOR_PIN)
#define SENSOR_PIN_WRITE(state) do {\
PLIB_PORTS_PinWrite(PORTS_ID_0, DHT_SENSOR_PORT, DHT_SENSOR_PIN, state);\
} while(0)
/**
 * \memberof Heater
 * \param void
 * \brief Initializes Sensor.
 *
 *
*/
void Sensor_DHT_Init(void);

/**
 * \memberof Heater
 * \param place holders for temperature
 * \brief Note: Temperature value set is the cached value from last humidity read(after Sensor_DHT_ReadHumidity)
*/
bool Sensor_DHT_ReadTemperature(float* const temperature);

/**
 * \memberof Heater
 * \param place holders for humidity
 * \brief Returns :- whether retrieving values from the sensor is successful or not.
*/
bool Sensor_DHT_ReadHumidity(float* const humidity);

/**
 * \memberof Heater
 * \param place holders for humidity and temperature
 * \brief Returns :- whether retrieving values from the sensor is successful or not.
*/
bool Sensor_DHT_ReadHumidityAndTemperature(float* const humidity, float* const temperature);

#ifdef	__cplusplus
}
#endif

#endif	/* SENSOR_DHT_H */

