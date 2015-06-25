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

#include "climate_sensor.h"
#include "app.h"
#include "sensor.h"
#include "flow_interface.h"
#include "flow/flowcore.h"
#include "flow/flowmessaging.h"
#include "flow_task_priority.h"
#include <math.h>
#include <float.h>
#include <time.h>
#include "queue_wrapper.h"
#include "climate_control_logging.h"
#include "send_message.h"


struct tm *gmtime_r(const time_t *timep, struct tm *result);

/*============================================================================*/
/*                     FUNCTIONS (LOCAL)									  */
/*============================================================================*/

static void QueueMeasurementMsg(ClimateSensor* me);
static void ClimateSensorThread(FlowThread thread, void *taskParameters);
static int CompareMeasurements(float a, float b, float offset);
static char* CreateMessageXML(ClimateSensor* me, time_t time);
static void CmdTimerHandler(FlowTimer timer, void *context);
static void CleanUp(ClimateSensor* me);
static ControllerCmd* ParseMsgAndCreateControllerCmd(const char* msgString);
static ControllerCmd* CreateControllerCmd(char *cmd, const char *msgString);
static bool NodeValueToInt(TreeNode root, unsigned int* valueToSet, char* nodeName);
static bool NodeValueToFloat(TreeNode root, float* valueToSet, char* nodeName);
static void FreeControllerCmd(ControllerCmd *command);
static void StopAndFreeTimers(ClimateSensor *me);
static bool IsSensorValueChanged(ClimateSensor* me, Sensor_Type sensor);
static float GetCurrentSensorValue(ClimateSensor *me, Sensor_Type type);
static void ParseAndUpdateSettings(ClimateSensor* me, const char* details);
static void CommandsHandlerInternal(ClimateSensor* me, ControllerCmd* command);
static void StartTimers(ClimateSensor *me);

static int CompareMeasurements(float a, float b, float offset)
{
	return (fabs(a - b) > offset);
}

static char* CreateMessageXML(ClimateSensor* me, time_t time)
{
	char *tempString = NULL;
	struct tm timeNow;
	gmtime_r(&time, &timeNow);
	unsigned int i;
	char tagArray[TAG_ARRAY_SIZE] = {0};
	for (i = 0; i < NUM_SENSORS; ++i)
	{
		unsigned int len;
		char tag[TAG_SIZE] = {0};
		len = strlen(me->sensors[i].xmlTagString) + 1;
		snprintf(tag, len, me->sensors[i].xmlTagString, me->sensors[i].value);
		strncat(tagArray, tag, len);
	}
	const char *msgXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
						"<event>"
							"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
							"<type>Sensor</type>"
							"<info>"
								"%s"
							"</info>"
						"</event>";
	unsigned int stringSize = strlen(msgXML) + strlen(tagArray) + 1;
	tempString = Flow_MemAlloc(stringSize);
	if (tempString)
	{
		snprintf(tempString, stringSize, msgXML,
				timeNow.tm_year + 1900,
				timeNow.tm_mon + 1,
				timeNow.tm_mday,
				timeNow.tm_hour,
				timeNow.tm_min,
				timeNow.tm_sec,
				tagArray);
	}
	return tempString;
}

static float GetCurrentSensorValue(ClimateSensor *me, Sensor_Type type)
{
	unsigned  int i;
	for (i = 0; i < NUM_SENSORS; ++i)
	{
		if (type == me->sensors[i].type)
		{
			return me->sensors[i].value;
		}
	}
	return FLT_MAX;
}

/*
 * Read sensor value and return true if value has been changed more than read delta
 */
static bool IsSensorValueChanged(ClimateSensor* me, Sensor_Type sensor)
{
	float value = 0.0f;
	unsigned int index;
	for (index = 0; index < NUM_SENSORS; ++index)
	{
		if (sensor == me->sensors[index].type)
		{
			break;
		}
	}
	if (index < NUM_SENSORS)
	{
		bool success;
		success = me->sensors[index].readFunc(&value);
		if (success)
		{
			if (CompareMeasurements(me->sensors[index].value, value, me->sensors[index].readDelta))
			{
				me->sensors[index].value = value;
				return true;
			}
		}
	}
	return false;
}

static void QueueMeasurementMsg(ClimateSensor* me)
{
	time_t time;
	Flow_GetTime(&time);
	char* msgXML = CreateMessageXML(me, time);
	if (msgXML)
	{
		MeasurementMsg msg;
		msg.deviceId = FlowString_Duplicate(me->controllerId);
		msg.details = msgXML;

		if (false == QueueSend(me->sendMessageQueue, &msg))
		{
			ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Enqueuing sendMessageQueue failed");
			Flow_MemFree((void **)&(msg.deviceId));
			Flow_MemFree((void **)&msgXML);
		}
		else
		{
			struct tm date;
			gmtime_r(&time, &date);
			ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "Measurement %0.2f %0.2f %d:%d:%d",
												GetCurrentSensorValue(me, Sensor_Temperature),
												GetCurrentSensorValue(me, Sensor_Humidity),
												date.tm_hour,
												date.tm_min,
												date.tm_sec);
		}
	}
	else
	{
		ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Creating measurement msg failed");
	}
}

static void CmdTimerHandler(FlowTimer timer, void *context)
{
	if (context)
	{
		TimerContext* timerContext = context;
		QueueHandle* cmdQueue = timerContext->queue ;
		ClimateSensorCmd cmd;
		cmd.cmdType = timerContext->cmdType;
		cmd.details = NULL;
		if (false == QueueSend(cmdQueue, &cmd))
		{
			ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "ClimateSensor thread can't post in queue %d/%d",
								QueueNumOfItems(cmdQueue),
								CLIMATE_SENSOR_MSG_QUEUE_SIZE);
		}
	}
}

static void ParseAndUpdateSettings(ClimateSensor* me, const char* details)
{
	if (details)
	{
		TreeNode xmlTreeRoot = TreeNode_ParseXML((uint8_t*)details, strlen(details), true);
		if (xmlTreeRoot)
		{
			unsigned int i, value;
			if (NodeValueToInt(xmlTreeRoot, &value, HEART_BEAT_XML_TAG) &&
				(me->heartBeatTimer) &&
				(me->heartBeat != value))
			{
				ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "Changed heartbeat timer period from %u to %u ",
																me->heartBeat, value);
				me->heartBeat = value;
				FlowTimer_SetPeriod(me->heartBeatTimer, me->heartBeat);
			}

			for (i = 0 ; i < NUM_SENSORS; ++i)
			{
				unsigned int readInterval;
				float readDelta;
				if (NodeValueToInt(xmlTreeRoot, &readInterval, me->sensors[i].readIntervalString) &&
					(readInterval != me->sensors[i].readInterval) &&
					(readInterval > me->sensors[i].minimumReadInterval) &&
					(me->sensors[i].readSensorTimer))
				{
					ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "Changed read timer period from %u to %u",
										me->sensors[i].readInterval,
										readInterval);
					me->sensors[i].readInterval = readInterval;
					FlowTimer_SetPeriod(me->sensors[i].readSensorTimer, me->sensors[i].readInterval);
				}

				if (NodeValueToFloat(xmlTreeRoot, &readDelta, me->sensors[i].readDeltaString) &&
					(me->sensors[i].readDelta != readDelta))
				{
					ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "Changed read delta from %f to %f",
										me->sensors[i].readDelta,
										readDelta);
					me->sensors[i].readDelta = readDelta;
				}
			}
			Tree_Delete(xmlTreeRoot);
		}
	}
}

static void CommandsHandlerInternal(ClimateSensor* me, ControllerCmd* command)
{
	ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Sensor cmd %s", command->cmd);
	if (strcmp(command->cmd, UPDATE_SETTINGS_STR) == 0)
	{
		ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "%s", UPDATE_SETTINGS_STR);
		ParseAndUpdateSettings(me, command->payload);
	}
	else
	{
		ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Unknown command received:\t%s", command->cmd);
	}
}

static void StopAndFreeTimers(ClimateSensor *me)
{
	unsigned int i;
	for (i = 0; i < NUM_SENSORS; ++i)
	{
		if (me->sensors[i].readSensorTimer)
		{
			FlowTimer_Stop(me->sensors[i].readSensorTimer);
			FlowTimer_Free(me->sensors[i].readSensorTimer);
		}
	}
	if (me->heartBeatTimer)
	{
		FlowTimer_Stop(me->heartBeatTimer);
		FlowTimer_Free(me->heartBeatTimer);
	}
}

static void StartTimers(ClimateSensor *me)
{
	unsigned int i;
	for (i = 0; i < NUM_SENSORS; ++i)
	{
		FlowTimer_Start(me->sensors[i].readSensorTimer);
	}
	FlowTimer_Start(me->heartBeatTimer);
}

static void ClimateSensorThread(FlowThread thread, void *taskParameters)
{
	// 1. Reads the sensor periodically
	// 2. if change in temperature/humidity Queue measurement msg in SendMessage Queue
	// 3. if HEARTBEAT timeout  Queue heartbeat msg in SendMessage Queue.
	ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "climateSensorThread started");
	ClimateSensor* me = taskParameters;

	TimerContext humidityTimerContext = {me->cmdQueue, ClimateSensorCmd_ReadHumiditySensor};
	TimerContext temperatureTimerContext = {me->cmdQueue, ClimateSensorCmd_ReadTemperatureSensor};
	TimerContext heartBeatTimerContext = {me->cmdQueue, ClimateSensorCmd_SendHeartBeat};

	me->sensors[Sensor_Humidity].readSensorTimer = FlowTimer_New("ReadHumiditySensorTimer", MINIMUM_HUMIDITY_SENSOR_READ_PERIOD, true, CmdTimerHandler, (void *)&humidityTimerContext);
	me->sensors[Sensor_Temperature].readSensorTimer = FlowTimer_New("ReadTemperatureSensorTimer", MINIMUM_TEMPERATURE_SENSOR_READ_PERIOD, true, CmdTimerHandler, (void *)&temperatureTimerContext);
	me->heartBeatTimer = FlowTimer_New("HeartBeatCmdTimer", DEFAULT_HEART_BEAT_PERIOD, true, CmdTimerHandler, (void *)&heartBeatTimerContext);
	if (me->sensors[Sensor_Temperature].readSensorTimer == NULL ||
		me->heartBeatTimer == NULL ||
		me->sensors[Sensor_Humidity].readSensorTimer == NULL
		)
	{
		ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Timer creation failed");
		StopAndFreeTimers(me);
		// can't do much return
		return;
	}
	StartTimers(me);
	while (me)
	{
		//check for any commands
		ClimateSensorCmd cmd;
		if (QueueReceive(me->cmdQueue, &cmd, -1))
		{
			switch (cmd.cmdType)
			{
				case ClimateSensorCmd_ReadTemperatureSensor:
				{
					ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Received ClimateSensorCmd_ReadTemperatureSensor");
					if (IsSensorValueChanged(me, Sensor_Temperature))
					{
						QueueMeasurementMsg(me);
					}
					break;
				}
				case ClimateSensorCmd_ReadHumiditySensor:
				{
					ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Received ClimateSensorCmd_ReadHumiditySensor");
					if (IsSensorValueChanged(me, Sensor_Humidity))
					{
						QueueMeasurementMsg(me);
					}
					break;
				}
				case ClimateSensorCmd_SendHeartBeat:
				{
					ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Received ClimateSensorCmd_SendHeartBeat");
					QueueMeasurementMsg(me);
					break;
				}
				case ClimateSensorCmd_ControllerCmd:
				{
					ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Received ClimateSensorCmd_ControllerCmd");
					CommandsHandlerInternal(me, cmd.details);
					FreeControllerCmd(cmd.details);
					break;
				}
				default:
					ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Received Unknown Cmd");
			}
		}
	}
	StopAndFreeTimers(me);
}

static void CleanUp(ClimateSensor* me)
{
	if (me->sendMessageQueue)
		QueueDelete(me->sendMessageQueue);
	if(me->cmdQueue)
		QueueDelete(me->cmdQueue);
	if (me->climateSensorThread)
		FlowThread_Free(me->climateSensorThread);
}

static bool NodeValueToInt(TreeNode root, unsigned int* valueToSet, char* nodeName)
{
	TreeNode node = TreeNode_Navigate(root, nodeName);
	if (node)
	{
		char *buf = (char *)TreeNode_GetValue(node);
		if (buf && *buf)
		{
			*valueToSet = (unsigned int)strtoul (buf, NULL, 10);
			return true;
		}
	}
	return false;
}

static bool NodeValueToFloat(TreeNode root, float* valueToSet, char* nodeName)
{
	TreeNode node = TreeNode_Navigate(root, nodeName);
	if (node)
	{
		char *buf = (char *)TreeNode_GetValue(node);
		if (buf && *buf)
		{
			*valueToSet = atof(buf);
			return true;
		}
	}
	return false;
}

static void FreeControllerCmd(ControllerCmd *command)
{
	if (command)
	{
		if (command->cmd)
		{
			Flow_MemFree((void**)&(command->cmd));
		}
		if (command->payload)
		{
			Flow_MemFree((void**)&(command->payload));
		}
		Flow_MemFree((void**)&(command));
	}
}

static ControllerCmd* CreateControllerCmd(char *cmd, const char *msgString)
{
	ControllerCmd *controllerCmd  = (ControllerCmd *)Flow_MemAlloc(sizeof(ControllerCmd));
	bool success = false;
	if (controllerCmd)
	{
		controllerCmd->cmd = NULL;
		controllerCmd->payload = NULL;
		unsigned int len = strlen(cmd);
		controllerCmd->cmd = (char *)Flow_MemAlloc(len + 1);
		if (controllerCmd->cmd)
		{
			strncpy(controllerCmd->cmd, cmd, len + 1);
			if (strcmp(UPDATE_SETTINGS_STR, cmd) == 0)
			{
				unsigned int msgLen = strlen(msgString);
				controllerCmd->payload = Flow_MemAlloc(msgLen + 1);
				if (controllerCmd->payload)
				{
					strncpy(controllerCmd->payload, msgString, msgLen + 1);
					success = true;
				}
			}
		}
		if (!success)
		{
			FreeControllerCmd(controllerCmd);
		}
	}
	return controllerCmd;
}

static ControllerCmd* ParseMsgAndCreateControllerCmd(const char* msgString)
{
	ControllerCmd *controllerCmd  = NULL;
	if (msgString)
	{
		TreeNode xmlTreeRoot = TreeNode_ParseXML((uint8_t*)msgString, strlen(msgString), true);
		if (xmlTreeRoot)
		{
			// Sample XML format which we will receive from controller
			//<?xml version="1.0" encoding="UTF-8"?>
			//<command>
			//	<time type="datetime">2014-11-28T15:30:09Z</time>
			//	<info>UPDATE_SETTINGS</info>
			//	<settings>
			//		<HeartBeat>15000</HeartBeat>
			//		<TemperatureReadInterval>1000</TemperatureReadInterval>
			//		<HumidityReadInterval>2500</HumidityReadInterval>
			//		<TemperatureReadDelta>0.5</TemperatureReadDelta>
			//		<HumidityReadDelta>2</HumidityReadDelta>
			//	</settings>
			//</command>

			TreeNode eventDetails = TreeNode_Navigate(xmlTreeRoot, "command/info");
			if (eventDetails)
			{
				char *buf = (char *)TreeNode_GetValue(eventDetails);
				if (buf && *buf)
				{
					controllerCmd = CreateControllerCmd(buf, msgString);
				}
			}
			Tree_Delete(xmlTreeRoot);
		}
	}
	return controllerCmd;
}

/*============================================================================*/
/*                     FUNCTIONS (EXPORTED)									  */
/*============================================================================*/

void ClimateSensor_Initialize(ClimateSensor* me)
{
	ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "---------Initialize ClimateSensor------" );
	bool success = false;

	//Try infinitely to look fo controller, if there is no controller then there
	//is no point in carrying out the rest of the operations
	while (1)
	{
		me->controllerId = ClimateControl_FindDevice(CONTROLLER_DEVICE_TYPE);
		if (me->controllerId)
		{
			ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "found device of type %s", CONTROLLER_DEVICE_TYPE);
			ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "device id  = %s", me->controllerId);
			break;
		}
		ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "Couldn't find device of type %s", CONTROLLER_DEVICE_TYPE);
		FlowThread_Sleep(NULL, 5);
	}
	Sensor_Init();
	me->sendMessageQueue = QueueCreate(SEND_MESSAGE_QUEUE_SIZE, sizeof(MeasurementMsg));
	me->cmdQueue = QueueCreate(CLIMATE_SENSOR_MSG_QUEUE_SIZE, sizeof(ClimateSensorCmd));
	if (me->sendMessageQueue != 0 && me->cmdQueue != 0)
	{
		me->climateSensorThread = FlowThread_New("ClimateSensorThread", USER_TASK_PRIORITY,
									configMINIMAL_STACK_SIZE, ClimateSensorThread, me);
		if (me->climateSensorThread != NULL)
		{
			me->sendMessageThread = FlowThread_New("SendMessageThread", USER_TASK_PRIORITY,
										configMINIMAL_STACK_SIZE, SendMessageThread, me->sendMessageQueue);
			if (me->sendMessageThread != NULL)
			{
				success = true;
			}
			else
			{
				ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Creating send message thread failed");
			}
		}
		else
		{
			ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Creating climate sensor thread failed");
		}
	}
	else
	{
		ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Queue creation failed");
	}

	if (!success)
	{
		CleanUp(me);
		return;
	}
	RegisterCallback(MessageReceptionCallback);
	ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "---------Initialize ClimateSensor Done------" );
}

void ClimateSensor_CommandsHandler(ClimateSensor* me, const char* msgString)
{
	ControllerCmd *controllerCmd = ParseMsgAndCreateControllerCmd(msgString);
	if (NULL == controllerCmd)
	{
		ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Could not parse received message");
		if (msgString)
		{
			ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Unknown message received  =  %s", msgString);
		}
	}
	else
	{
		ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "cmd %s received from controller", controllerCmd->cmd);
		ClimateSensorCmd cmd;
		cmd.cmdType = ClimateSensorCmd_ControllerCmd;
		cmd.details = controllerCmd;
		//Queue command if it fails there is nothing we can do.
		if (false == QueueSend(me->cmdQueue, &cmd))
		{
			FreeControllerCmd(controllerCmd);
		}
	}
}