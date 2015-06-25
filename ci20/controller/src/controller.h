/****************************************************************************
 Copyright (c) 2015, Imagination Technologies Limited
 All rights reserved.

 Redistribution and use of the Software in source and binary forms, with or
 without modification, are permitted provided that the following conditions are met:

     1. The Software (including after any modifications that you make to it) must
        support the FlowCloud Web Service API provided by Licensor and accessible
        at  http://ws-uat.flowworld.com and/or some other location(s) that we specify.

     2. Redistributions of source code must retain the above copyright notice, this
        list of conditions and the following disclaimer.

     3. Redistributions in binary form must reproduce the above copyright notice, this
        list of conditions and the following disclaimer in the documentation and/or
        other materials provided with the distribution.

     4. Neither the name of the copyright holder nor the names of its contributors may
        be used to endorse or promote products derived from this Software without
        specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef CONTROLLER_H
#define CONTROLLER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>

#include "flow/core/flow_threading.h"
#include "flow/core/flow_queue.h"
#include "flow/core/flow_timer.h"

#define MAX_SIZE (50)
#define QUEUE_WAITING_TIME (10000)	//milliseconds
#define NUM_SENSORS (2)
#define NUM_RELAYS (2)

//Controller configuration defaults
#define DEFAULT_TEMP_THRESHOLD (25.00)	//degree centigrade
#define DEFAULT_HMDT_THRESHOLD (30.00)	//percentage
#define DEFAULT_HEARTBEAT (15000)	//milliseconds
#define DEFAULT_SENSOR_VALUE (-1000.00)

//Sensor configuration defaults
#define DEFAULT_SENSOR_HEARTBEAT (15000)	//milliseconds
#define DEFAULT_TEMP_READ_INTERVAL (1000)	//milliseconds
#define DEFAULT_HMDT_READ_INTERVAL (2500)	//milliseconds
#define DEFAULT_TEMP_READ_DELTA	(0.5)	//degree centigrade
#define DEFAULT_HMDT_READ_DELTA	(2.0)	//percentage

//Actuator configuration defaults
#define DEFAULT_ACTUATOR_HEARTBEAT (15000)	//milliseconds

//XML tags for constructing messages
#define TEMPERATURE_XML_TAG "<Temperature>%.02f</Temperature>"
#define TEMP_THRESHOLD_XML_TAG "<TemperatureThreshold>%0.2f</TemperatureThreshold>"
#define TEMP_READ_INTERVAL_XML_TAG "<TemperatureReadInterval>%u</TemperatureReadInterval>"
#define TEMP_READ_DELTA_XML_TAG "<TemperatureReadDelta>%0.2f</TemperatureReadDelta>"
#define TEMP_ORIENTATION_XML_TAG "<TemperatureOrientation>%s</TemperatureOrientation>"
#define HUMIDITY_XML_TAG "<Humidity>%0.2f</Humidity>"
#define HMDT_THRESHOLD_XML_TAG "<HumidityThreshold>%0.2f</HumidityThreshold>"
#define HMDT_READ_INTERVAL_XML_TAG "<HumidityReadInterval>%u</HumidityReadInterval>"
#define HMDT_READ_DELTA_XML_TAG "<HumidityReadDelta>%0.2f</HumidityReadDelta>"
#define HMDT_ORIENTATION_XML_TAG "<HumidityOrientation>%s</HumidityOrientation>"
#define RELAY_1_XML_TAG "<Relay_1><mode>%s</mode><status>%s</status></Relay_1>"
#define RELAY_2_XML_TAG "<Relay_2><mode>%s</mode><status>%s</status></Relay_2>"
#define RELAY_1_STR "RELAY_1"
#define RELAY_2_STR "RELAY_2"

typedef enum
{
	Sensor_Temperature,
	Sensor_Humidity,
}Sensor_Type;

typedef enum
{
	Orientation_Above,
	Orientation_Below,
}Orientation_Type;

typedef enum
{
	Relay_On,
	Relay_Off,
}Relay_Status;

typedef enum
{
	Relay_Auto,
	Relay_Manual,
}Relay_Mode;

typedef enum
{
	Relay_Heater,
	Relay_Fan,
}Relay_Type;

typedef struct
{
	Relay_Type type;
	Relay_Status status;
	Relay_Mode mode;
	char *relayTagString;
	char *relayName;
}Relay;

typedef struct
{
	Sensor_Type type;
	Orientation_Type orientation;
	float threshold;
	float value;
	unsigned int readInterval;
	float readDelta;
	char *sensorTagString;
	char *thresholdTagString;
	char *orientationTagString;
	char *readIntervalTagString;
	char *readDeltaTagString;
}Sensor;

typedef struct
{
	bool isAlive;
	char *actuatorId;
	unsigned int heartBeat;
}ActuatorConfig;

typedef struct
{
	bool isAlive;
	char *sensorId;
	unsigned int heartBeat;
}SensorConfig;

typedef struct
{
	unsigned int heartBeat;
	FlowTimer heartBeatTimer;	//Timer for controller's heartbeat
}ControllerConfig;

typedef struct
{
	char userId[MAX_SIZE];
	bool isUserUpdate;

	ControllerConfig config;
	SensorConfig sensorConfig;
	ActuatorConfig actuatorConfig;

	Sensor sensors[NUM_SENSORS];
	Relay relays[NUM_RELAYS];

	FlowTimer sensorTimer;	//Timer to track sensor's DEAD/ALIVE status
	FlowTimer actuatorTimer;	//Timer to track actuator's DEAD/ALIVE status

	FlowThread controllerThread;
	FlowThread flowInterfaceThread;

	FlowQueue sendMsgQueue;	//Used by controller thread for posting message to flow thread
	FlowQueue receiveMsgQueue;	//Used by flow thread for posting message to controller thread
}Controller;

typedef enum
{
	ControllerEvent_HeartBeat,	//Controller's heartbeat event to self
	ControllerEvent_SettingSuccess,	//Event for successfully reading settings
	ControllerEvent_SettingFailure,	//Event for failure in reading settings
	ControllerEvent_ReceivedMessage, //Event for message receiving
}ControllerEvent_Type;

typedef struct
{
	ControllerEvent_Type evtType;
	void *details;
}ControllerEvent;

void ControllerThread(FlowThread thread, void *taskParameters);

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROLLER_H */
