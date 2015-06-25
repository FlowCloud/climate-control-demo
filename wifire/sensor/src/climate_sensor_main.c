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

#include "app.h"
#include "climate_sensor.h"
#include "climate_sensor_version.h"
#include "sensor.h"
#include <float.h>

#define DEVICE_TYPE    "ClimateControlDemoSensor"
#define CLIMATE_CONTROL_SENSOR_NAME "Flow climate control sensor"

static ClimateSensor _sensor =
{
	.heartBeat = DEFAULT_HEART_BEAT_PERIOD,
	.sensors =
	{
		{
			.type = Sensor_Temperature,
			.value = FLT_MAX,
			.readInterval = MINIMUM_TEMPERATURE_SENSOR_READ_PERIOD,
			.minimumReadInterval = MINIMUM_TEMPERATURE_SENSOR_READ_PERIOD,
			.readDelta = DEFAULT_TEMPERATURE_READING_DELTA,
			.readIntervalString = READ_TEMPERATURE_INTERVAL_XML_TAG,
			.readDeltaString = READ_TEMPERATURE_DELTA_XML_TAG,
			.xmlTagString = TEMPERATURE_XML_TAG,
			.readFunc = Sensor_ReadTemperature,
			.readSensorTimer = NULL
		},
		{
			.type = Sensor_Humidity,
			.value = FLT_MAX,
			.readInterval = MINIMUM_HUMIDITY_SENSOR_READ_PERIOD,
			.minimumReadInterval = MINIMUM_HUMIDITY_SENSOR_READ_PERIOD,
			.readDelta = DEFAULT_HUMIDITY_READING_DELTA,
			.readIntervalString = READ_HUMIDITY_INTERVAL_XML_TAG,
			.readDeltaString = READ_HUMIDITY_DELTA_XML_TAG,
			.xmlTagString = HUMIDITY_XML_TAG,
			.readFunc = Sensor_ReadHumidity,
			.readSensorTimer = NULL
		}
	}
};
void MessageReceptionCallback(FlowMessagingMessage message)
{
	char *content =  FlowMessagingMessage_GetContent(message);
	ClimateSensor_CommandsHandler(&_sensor, content);;
}

const char *ClimateControl_GetDeviceType(void)
{
	return DEVICE_TYPE;
}

const char *ClimateControl_GetExternalSoftwareVersion(void)
{
	return CLIMATE_SENSOR_EXTERNAL_VERSION;
}

const char *ClimateControl_GetInternalSoftwareVersion(void)
{
	return CLIMATE_SENSOR_INTERNAL_VERSION;
}

const char *ClimateControl_GetAppName(void)
{
	return CLIMATE_CONTROL_SENSOR_NAME;
}

void UserSetup(void)
{
	ClimateSensor_Initialize(&_sensor);
}