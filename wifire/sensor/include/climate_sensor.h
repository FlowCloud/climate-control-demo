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

#ifndef CLIMATE_SENSOR_H
#define	CLIMATE_SENSOR_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "flow/flowcore.h"
#include "flow/flowmessaging.h"
#include "queue_wrapper.h"

#define NUM_SENSORS (2)

#ifdef USE_DHT_SENSOR
	#define MINIMUM_TEMPERATURE_SENSOR_READ_PERIOD	(2500) //milliseconds
#else
	#define MINIMUM_TEMPERATURE_SENSOR_READ_PERIOD	(500) //milliseconds
#endif
#define	DEFAULT_HEART_BEAT_PERIOD					(15*1000)//milliseconds
#define MINIMUM_HUMIDITY_SENSOR_READ_PERIOD			(2500) //milliseconds
#define CLIMATE_SENSOR_MSG_QUEUE_SIZE				(15)
#define DEFAULT_TEMPERATURE_READING_DELTA			(0.5f)
#define DEFAULT_HUMIDITY_READING_DELTA				(2.0f)
#define TAG_ARRAY_SIZE								(300)
#define TAG_SIZE									(70)
#define CONTROLLER_DEVICE_TYPE						"ClimateControlDemoController"
#define UPDATE_SETTINGS_STR							"UPDATE_SETTINGS"
#define HEART_BEAT_XML_TAG							"command/settings/HeartBeat"
#define READ_TEMPERATURE_INTERVAL_XML_TAG			"command/settings/TemperatureReadInterval"
#define READ_TEMPERATURE_DELTA_XML_TAG				"command/settings/TemperatureReadDelta"
#define READ_HUMIDITY_INTERVAL_XML_TAG				"command/settings/HumidityReadInterval"
#define READ_HUMIDITY_DELTA_XML_TAG					"command/settings/HumidityReadDelta"
#define TEMPERATURE_XML_TAG							"<Temperature>%0.2f</Temperature>"
#define HUMIDITY_XML_TAG							"<Humidity>%0.2f</Humidity>"

typedef bool (*SensorReadFunc)(float*  const);

typedef enum
{
	ClimateSensorCmd_ReadTemperatureSensor,
	ClimateSensorCmd_ReadHumiditySensor,
	ClimateSensorCmd_SendHeartBeat,
	ClimateSensorCmd_ControllerCmd,
}ClimateSensorCmd_Type;

typedef enum
{
	Sensor_Temperature,
	Sensor_Humidity,
}Sensor_Type;


typedef struct
{
	char *cmd;
	void *payload;
}ControllerCmd;

typedef struct
{
	ClimateSensorCmd_Type cmdType;
	ControllerCmd *details;
}ClimateSensorCmd;

typedef struct
{
	Sensor_Type type;
	float value;
	unsigned int readInterval;
	unsigned int minimumReadInterval;
	float readDelta;
	char *readIntervalString;
	char *readDeltaString;
	char *xmlTagString;
	SensorReadFunc readFunc;
	FlowTimer readSensorTimer;
}Sensor;

typedef struct
{
	QueueHandle* queue;
	ClimateSensorCmd_Type cmdType;
}TimerContext;


// currently all ClimateSensor datastructure are used in ClimateSensorThread
// using it in other thread would require synchronization
typedef struct
{
	QueueHandle sendMessageQueue; // Queue between ClimateSensorThread -> SendMessageThread
	QueueHandle cmdQueue; //To send self commands climateSensorThread->climateSensorThread
	FlowThread climateSensorThread;
	FlowThread sendMessageThread;
	char *controllerId;
	unsigned int heartBeat;
	FlowTimer heartBeatTimer;
	Sensor sensors[NUM_SENSORS];
}ClimateSensor;

/**
 * \memberof ClimateSensor
 * \param
 * \brief Initialize the ClimateSensor
 *
*/
void ClimateSensor_Initialize(ClimateSensor* me);

/**
 * \memberof ClimateSensor
 * \param
 * \brief Handles Commands sent by controller
 *
*/
void ClimateSensor_CommandsHandler(ClimateSensor* me, const char* command);

/**
  * \param message from the device
 * \brief callback to receive asynchronous messages
 *
*/
void MessageReceptionCallback(FlowMessagingMessage message);

#ifdef	__cplusplus
}
#endif

#endif	/* CLIMATE_SENSOR_H */

