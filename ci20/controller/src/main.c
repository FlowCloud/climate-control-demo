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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include "controller.h"
#include "flow_interface.h"
#include "controller_logging.h"
#include "console.h"

#define MINIMAL_STACK_SIZE (4096)
#define QUEUE_SIZE (20)
#define USER_TASK_PRIORITY (1)
#define DEBUG_LEVEL_STRING "DEBUG_LEVEL"

static Controller _Controller =
{
	.isUserUpdate = false,
	.config =
	{
		.heartBeat = DEFAULT_HEARTBEAT,
	},
	.sensorConfig =
	{
		.isAlive = false,
		.sensorId = NULL,
		.heartBeat = DEFAULT_SENSOR_HEARTBEAT,
	},
	.actuatorConfig =
	{
		.isAlive = false,
		.actuatorId = NULL,
		.heartBeat = DEFAULT_ACTUATOR_HEARTBEAT,
	},
	.sensors =
	{
		{
			.type = Sensor_Temperature,
			.threshold = DEFAULT_TEMP_THRESHOLD,
			.value = DEFAULT_SENSOR_VALUE,
			.orientation = Orientation_Below,
			.readInterval = DEFAULT_TEMP_READ_INTERVAL,
			.readDelta = DEFAULT_TEMP_READ_DELTA,
			.sensorTagString = TEMPERATURE_XML_TAG,
			.thresholdTagString = TEMP_THRESHOLD_XML_TAG,
			.orientationTagString = TEMP_ORIENTATION_XML_TAG,
			.readIntervalTagString = TEMP_READ_INTERVAL_XML_TAG,
			.readDeltaTagString = TEMP_READ_DELTA_XML_TAG,
		},
		{
			.type = Sensor_Humidity,
			.threshold = DEFAULT_HMDT_THRESHOLD,
			.value = DEFAULT_SENSOR_VALUE,
			.orientation = Orientation_Above,
			.readInterval = DEFAULT_HMDT_READ_INTERVAL,
			.readDelta = DEFAULT_HMDT_READ_DELTA,
			.sensorTagString = HUMIDITY_XML_TAG,
			.thresholdTagString = HMDT_THRESHOLD_XML_TAG,
			.orientationTagString = HMDT_ORIENTATION_XML_TAG,
			.readIntervalTagString = HMDT_READ_INTERVAL_XML_TAG,
			.readDeltaTagString = HMDT_READ_DELTA_XML_TAG,
		},
	},
	.relays =
	{
		{
			.type = Relay_Heater,
			.status = Relay_Off,
			.mode = Relay_Auto,
			.relayTagString = RELAY_1_XML_TAG,
			.relayName = RELAY_1_STR,
		},
		{
			.type = Relay_Fan,
			.status = Relay_Off,
			.mode = Relay_Auto,
			.relayTagString = RELAY_2_XML_TAG,
			.relayName = RELAY_2_STR,
		},
	},
};

static bool ControllerInit(Controller *me)
{
	ControllerLog(ControllerLogLevel_Info, INFO_PREFIX "---------Initialize Controller------" );

	me->controllerThread = FlowThread_New("ControllerTask", USER_TASK_PRIORITY, MINIMAL_STACK_SIZE, ControllerThread, me);

	if (me->controllerThread == NULL)
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Creation of controller thread failed");
		return false;
	}

	me->flowInterfaceThread = FlowThread_New("FlowInterfaceTask", USER_TASK_PRIORITY, MINIMAL_STACK_SIZE, FlowInterfaceThread, me);

	if (me->flowInterfaceThread == NULL)
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Creation of flow messaging thread failed");
		return false;
	}
	return true;
}

int main(int argc, char *argv[])
{
	int result = -1;
	Controller *me = &_Controller;
	ControllerLog_Type level = ControllerLogLevel_None;

	if (argc > 1)
	{
		if (strcmp(DEBUG_LEVEL_STRING, argv[1]) == 0)
		{
			level = atoi(argv[2]);
		}
	}

	me->sendMsgQueue = FlowQueue_NewBlocking(QUEUE_SIZE);
	me->receiveMsgQueue = FlowQueue_NewBlocking(QUEUE_SIZE);

	if (!ControllerLogSetLevel(level))
	{
		printf("DEBUG_LEVEL should be less than %d\n",ControllerLogLevel_Max);
	}

	if (InitializeFlowInterface(me))
	{
		if (ControllerInit(me));
		{
			StartConsole();
		}
	}

	FlowQueue_Free(&me->sendMsgQueue);
	FlowQueue_Free(&me->receiveMsgQueue);

	return result;
}

