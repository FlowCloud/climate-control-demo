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

#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#include "controller.h"
#include "controller_logging.h"
#include "flow_interface.h"
#include "construct_message.h"

#define SENSOR_HEARTBEAT_EXPIRY (DEFAULT_SENSOR_HEARTBEAT * 2)	//milliseconds
#define ACTUATOR_HEARTBEAT_EXPIRY (DEFAULT_ACTUATOR_HEARTBEAT * 2)	//milliseconds
#define FLOAT_COMPARE_PRECISION (100)

#define EVENT_STR "event"
#define COMMAND_STR "command"
#define SENSOR_STR "Sensor"
#define ACTUATOR_STR "Actuator"
#define ON_STR "ON"
#define OFF_STR "OFF"
#define RELAY_1_ON_STR "RELAY_1_ON"
#define RELAY_1_OFF_STR "RELAY_1_OFF"
#define RELAY_2_ON_STR "RELAY_2_ON"
#define RELAY_2_OFF_STR "RELAY_2_OFF"
#define RELAY_1_AUTO_STR "RELAY_1_AUTO"
#define RELAY_2_AUTO_STR "RELAY_2_AUTO"
#define PING_STR "PING"
#define UNKNOWN_COMMAND_STR "UNKNOWN_COMMAND"
#define RETRIEVE_SETTINGS_STR "RETRIEVE_SETTINGS"
#define ABOVE_STR "ABOVE"
#define BELOW_STR "BELOW"

//XML strings for parsing received messages
#define TEMPERATURE_THRESHOLD_XML_STR "ControllerConfig/TemperatureThreshold"
#define HUMIDITY_THRESHOLD_XML_STR "ControllerConfig/HumidityThreshold"
#define TEMPERATURE_ORIENTATION_XML_STR "ControllerConfig/TemperatureOrientation"
#define HUMIDITY_ORIENTATION_XML_STR "ControllerConfig/HumidityOrientation"
#define CONTROLLER_HEARTBEAT_XML_STR "ControllerConfig/HeartBeat"
#define SENSOR_HEARTBEAT_XML_STR "ControllerConfig/SensorConfig/HeartBeat"
#define TEMP_READ_INTERVAL_XML_STR "ControllerConfig/SensorConfig/TemperatureReadInterval"
#define HMDT_READ_INTERVAL_XML_STR "ControllerConfig/SensorConfig/HumidityReadInterval"
#define TEMP_READ_DELTA_XML_STR "ControllerConfig/SensorConfig/TemperatureReadDelta"
#define HMDT_READ_DELTA_XML_STR "ControllerConfig/SensorConfig/HumidityReadDelta"
#define ACTUATOR_HEARTBEAT_XML_STR "ControllerConfig/ActuatorConfig/HeartBeat"
#define EVENT_TYPE_XML_STR "event/type"
#define EVENT_TEMPERATURE_XML_STR "event/info/Temperature"
#define EVENT_HUMIDITY_XML_STR "event/info/Humidity"
#define EVENT_RELAY_1_XML_STR "event/info/Relay_1"
#define EVENT_RELAY_2_XML_STR "event/info/Relay_2"
#define COMMAND_INFO_XML_STR "command/info"
#define COMMAND_APP_TIME_XML_STR "command/app_time"

struct tm *gmtime_r(const time_t *timep, struct tm *result);

typedef enum
{
	Message_RelayCommandToActuator,
	Message_ResponseToUser,
	Message_PingResponseToUser,
	Message_HeartBeatToUser,
	Message_DeviceStatusToUser,
	Message_SensorStatusToUser,
	Message_ActuatorStatusToUser,
	Message_UpdateSettingsToSensor,
	Message_UpdateSettingsToActuator,
}Message_Type;

typedef enum
{
	Device_Sensor,
	Device_Actuator,
}Device_Type;

static void FreeEvent(ControllerEvent *event)
{
	if (event->details)
	{
		Flow_MemFree((void **)&event->details);
	}
	Flow_MemFree((void **)&event);
}

/**
 * Send a message to self for sending a heartBeat command
 */
static bool PostControllerEventHeartBeat(const FlowQueue* receiveMsgQueue)
{
	bool success = false;
	ControllerEvent *event = NULL;

	//Allocated memory should be freed by the receiver
	event = (ControllerEvent *)Flow_MemAlloc(sizeof(ControllerEvent));
	if (event)
	{
		event->evtType = ControllerEvent_HeartBeat;
		event->details = NULL;

		success = FlowQueue_Enqueue(*receiveMsgQueue, event);

		if (!success)
		{
			Flow_MemFree((void **)&event);
		}
	}

	return success;
}

/**
 * Add an instant message to user, in flow thread's queue.
 * data - Command message for user.
 *        In case of any failure, user should free any memory allocated to it.
 *        Otherwise, would be freed by receiver if successfully added to queue.
 */
static bool PostFlowInterfaceCmdSendMsgToUser(const FlowQueue* sendMsgQueue, char *data)
{
	bool success = false;
	FlowInterfaceCmd *cmd = NULL;

	//Allocated memory should be freed by the receiver
	cmd = (FlowInterfaceCmd *)Flow_MemAlloc(sizeof(FlowInterfaceCmd));

	if (cmd)
	{
		cmd->cmdType = FlowInterfaceCmd_SendMessageToUser;
		if (data)
		{
			cmd->details = data;
			success = FlowQueue_Enqueue(*sendMsgQueue, cmd);
		}

		if (!success)
		{
			Flow_MemFree((void **)&cmd);
		}
	}

	return success;
}

/**
 * Add an instant message to device, in flow thread's queue.
 * data - Command message for device.
 *        In case of any failure, user should free any memory allocated to it.
 *        Otherwise, would be freed by receiver if successfully added to queue.
 */
static bool PostFlowInterfaceCmdSendMsgToDevice(const FlowQueue* sendMsgQueue, char *data, Device_Type deviceType)
{
	bool success = false;
	FlowInterfaceCmd *cmd = NULL;

	//Allocated memory should be freed by the receiver
	cmd = (FlowInterfaceCmd *)Flow_MemAlloc(sizeof(FlowInterfaceCmd));

	if (cmd)
	{
		if (deviceType == Device_Actuator)
		{
			cmd->cmdType = FlowInterfaceCmd_SendMessageToActuator;
		}
		else
		{
			cmd->cmdType = FlowInterfaceCmd_SendMessageToSensor;
		}

		if (data)
		{
			cmd->details = data;
			success = FlowQueue_Enqueue(*sendMsgQueue, cmd);
		}

		if (!success)
		{
			Flow_MemFree((void **)&cmd);
		}
	}

	return success;
}

/**
 * Send a command to flow thread for getting KVS config
 */
static bool PostFlowInterfaceCmdGetSetting(const FlowQueue* sendMsgQueue)
{
	bool success = false;
	FlowInterfaceCmd *cmd = NULL;

	//Allocated memory should be freed by the receiver
	cmd = (FlowInterfaceCmd *)Flow_MemAlloc(sizeof(FlowInterfaceCmd));

	if (cmd)
	{
		cmd->cmdType = FlowInterfaceCmd_GetSetting;
		cmd->details = NULL;
		success = FlowQueue_Enqueue(*sendMsgQueue, cmd);

		if (!success)
		{
			Flow_MemFree((void **)&cmd);
		}
	}

	return success;
}

/**
 * Send a command to flow thread for setting KVS config
 * data - Device settings message.
 *        In case of any failure, user should free any memory allocated to it.
 *        Otherwise, would be freed by receiver if successfully added to queue.
 */
static bool PostFlowInterfaceCmdSetSetting(const FlowQueue* sendMsgQueue, char *data)
{
	bool success = false;
	FlowInterfaceCmd *cmd = NULL;

	//Allocated memory should be freed by the receiver
	cmd = (FlowInterfaceCmd *)Flow_MemAlloc(sizeof(FlowInterfaceCmd));

	if (cmd)
	{
		cmd->cmdType = FlowInterfaceCmd_SetSetting;
		if (data)
		{
			cmd->details = data;
			success = FlowQueue_Enqueue(*sendMsgQueue, cmd);
		}

		if (!success)
		{
			Flow_MemFree((void **)&cmd);
		}
	}

	return success;
}

/**
 * Construct a message for user or device.
 * And post it on flow interface thread.
 */
static bool SendCommand(const Controller *me, const char *msg, Message_Type type)
{
	char *data = NULL;
	bool isDevice = false;
	bool success = false;
	Device_Type deviceType = Device_Actuator;

	switch (type)
	{
		//Allocated memory for messages should be freed by the receiver
		case Message_RelayCommandToActuator:
		{
			isDevice = true;
			success = ConstructRelayCommandForActuator(msg, &data);
			break;
		}
		case Message_SensorStatusToUser:
		{
			success = ConstructSensorStatusMsgForUser(me, &data);
			break;
		}
		case Message_DeviceStatusToUser:
		{
			success = ConstructDeviceStatusMsgForUser(me, &data);
			break;
		}
		case Message_ActuatorStatusToUser:
		{
			success = ConstructActuatorStatusMsgForUser(me, &data);
			break;
		}
		case Message_ResponseToUser:
		{
			success = ConstructResponseForUser(msg, &data);
			break;
		}
		case Message_PingResponseToUser:
		{
			success = ConstructPingResponseForUser(msg, &data);
			break;
		}
		case Message_HeartBeatToUser:
		{
			success = ConstructHeartBeatMsgForUser(me, &data);
			break;
		}
		case Message_UpdateSettingsToSensor:
		{
			isDevice = true;
			deviceType = Device_Sensor;
			success = ConstructSettingsCommandForSensor(me, &data);
			break;
		}
		case Message_UpdateSettingsToActuator:
		{
			isDevice = true;
			success = ConstructSettingsCommandForActuator(me->actuatorConfig, &data);
			break;
		}
		default:
			break;
	}

	if (success)
	{
		if (isDevice)
		{
			if (!PostFlowInterfaceCmdSendMsgToDevice(&me->sendMsgQueue, data, deviceType))
			{
				if (data)
				{
					Flow_MemFree((void **)&data);
				}
				ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Posting device's message to flow interface thread failed");
				success = false;
			}
		}
		else
		{
			if (!PostFlowInterfaceCmdSendMsgToUser(&me->sendMsgQueue, data))
			{
				if (data)
				{
					Flow_MemFree((void **)&data);
				}
				ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Posting user's message to flow interface thread failed");
				success = false;
			}
		}
	}
	return success;
}

/**
 * Get node value in integer format
 */
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

/**
 * Get node value in float format
 */
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

/**
 * Get node value in string format
 */
static bool NodeValueToString(TreeNode root, char* valueToSet, char* nodeName)
{
	TreeNode node = TreeNode_Navigate(root, nodeName);
	if (node)
	{
		char *buf = (char *)TreeNode_GetValue(node);
		if (buf && *buf)
		{
			strcpy(valueToSet,buf);
			return true;
		}
	}
	return false;
}

/**
 * Convert orientation string(ABOVE/BELOW) to orientation type
 * Return true, on successful conversion.
 */
static bool SetOrientation(Orientation_Type *orientation, const char *orientationStr)
{
	if (strcmp(orientationStr, ABOVE_STR) == 0)
	{
		*orientation = Orientation_Above;
		return true;
	}
	else if (strcmp(orientationStr, BELOW_STR) == 0)
	{
		*orientation = Orientation_Below;
		return true;
	}
	return false;
}

/**
 * Reset timer's Period
 */
static void ResetTimerPeriod(FlowTimer timer, const unsigned int timeout)
{
	FlowTimer_Stop(timer);
	FlowTimer_SetPeriod(timer, timeout);
	FlowTimer_Start(timer);
}

/**
 * Convert ON/OFF string to relay status.
 * Return true, on successful conversion.
 */
static bool GetRelayStatus(Relay_Status *relayStatus, const char *data)
{
	bool result = false;

	if (strcmp(ON_STR,data) == 0)
	{
		*relayStatus = Relay_On;
		result = true;
	}
	else if (strcmp(OFF_STR,data) == 0)
	{
		*relayStatus = Relay_Off;
		result = true;
	}
	return result;
}

/**
 * Construct relay command string for actuator
 */
static void ConstructRelayCmdStr(char *relayCmdStr, char *relayName, Relay_Status status)
{
	strcpy(relayCmdStr, relayName);
	strcat(relayCmdStr, "_");
	strcat(relayCmdStr, (status == Relay_On)?ON_STR:OFF_STR);
}

/**
 * Return relay status based on threshold and orientation setting of sensor
 */
static Relay_Status RelayControlLogic(Sensor sensor)
{

	if (((sensor.value <= sensor.threshold) && (sensor.orientation == Orientation_Below)) ||
		((sensor.value > sensor.threshold) && (sensor.orientation == Orientation_Above)))
	{
		return Relay_On;
	}
	return Relay_Off;
}

/**
 * Float comparison upto decimal places decided by FLOAT_COMPARE_PRECISION
 */
bool CompareFloat(float f1,float f2)
{
	return ((int)(f1 * FLOAT_COMPARE_PRECISION) == (int)(f2 * FLOAT_COMPARE_PRECISION));
}

/**
 * Determines whether there is a change in measurement values sent by sensor
 */
static bool IsSensorDataChanged(const Sensor *sensorOld, const Sensor *sensorNew)
{
	unsigned int i;

	for (i = 0; i < NUM_SENSORS; ++i)
	{
		if (!CompareFloat(sensorOld[i].value, sensorNew[i].value))
		{
			return true;
		}
	}
	return false;
}

/**
 * Run actuator logic, which turns ON/OFF relays based on the
 * measurement values sent by sensor, if relay is in auto mode.
 * And send an update to user.
 * Return true, if successfully sent a command to actuator.
 */
static bool ActuatorControlLogic(Controller *me)
{
	unsigned int i;
	bool success = false;

	for (i = 0; i < NUM_RELAYS; ++i)
	{
		if (me->relays[i].mode == Relay_Auto)
		{
			Relay_Status relayStatus;

			relayStatus = RelayControlLogic(me->sensors[i]);

			if (me->relays[i].status != relayStatus)
			{
				char relayCmdStr[MAX_SIZE] = {0};

				ConstructRelayCmdStr(relayCmdStr, me->relays[i].relayName, relayStatus);

				if (SendCommand(me, relayCmdStr, Message_RelayCommandToActuator))
				{
					me->relays[i].status = relayStatus;
					success = true;
				}
			}
		}
	}
	return success;
}

/**
 * Change relay mode to Auto, run actuator logic,
 * and send status update to user.
 * Return true, if mode has changed.
 */
static bool ChangeRelayModeToAuto(Controller *me, Relay_Type type)
{
	if (me->relays[type].mode == Relay_Manual)
	{
		//Relay is in manual mode, changing it to auto.
		//And send status update to user
		me->relays[type].mode = Relay_Auto;
		ActuatorControlLogic(me);
		ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Relay is now automatically controlled" );
		return true;
	}
	return false;
}

/**
 * Check if relay is in Auto mode, if yes, then change it to Manual.
 * Update relay status, and send an update to user.
 * Return true, if there is a change in relay status.
 */
static bool UpdateRelay(Controller *me, Relay_Type type, Relay_Status status, const char *relayStr)
{
	bool success = false;

	if (me->relays[type].mode == Relay_Auto)
	{
		//Relay is in auto mode, changing it to manual.
		//And send status update to user
		me->relays[type].mode = Relay_Manual;
		success = true;
		ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Relay is now manually controlled" );
	}

	if (me->relays[type].status != status)
	{
		//Update relay status
		//And send an update to user
		if (SendCommand(me, relayStr, Message_RelayCommandToActuator))
		{
			me->relays[type].status = status;
			success = true;
		}
	}
	return success;
}

/**
 * Parse KVS config, and update all settings
 */
static bool ParseAndUpdateSettings(const char *data, Controller *me)
{
	unsigned int controllerHeartBeat = 0;
	unsigned int sensorHeartBeat = 0;
	unsigned int actuatorHeartBeat = 0;
	char tempOrientationString[MAX_SIZE], hmdtOrientationString[MAX_SIZE];
	bool success = false;

	if (data)
	{
		TreeNode xmlTreeRoot = TreeNode_ParseXML((uint8_t*)data, strlen(data), true);
		if (xmlTreeRoot)
		{
			if ((NodeValueToFloat(xmlTreeRoot, &me->sensors[Sensor_Temperature].threshold, TEMPERATURE_THRESHOLD_XML_STR)) &&
				(NodeValueToFloat(xmlTreeRoot, &me->sensors[Sensor_Humidity].threshold, HUMIDITY_THRESHOLD_XML_STR)) &&
				(NodeValueToString(xmlTreeRoot, tempOrientationString, TEMPERATURE_ORIENTATION_XML_STR)) &&
				(NodeValueToString(xmlTreeRoot, hmdtOrientationString, HUMIDITY_ORIENTATION_XML_STR)) &&
				(NodeValueToInt(xmlTreeRoot, &controllerHeartBeat, CONTROLLER_HEARTBEAT_XML_STR)) &&
				(NodeValueToInt(xmlTreeRoot, &sensorHeartBeat, SENSOR_HEARTBEAT_XML_STR)) &&
				(NodeValueToInt(xmlTreeRoot, &me->sensors[Sensor_Temperature].readInterval, TEMP_READ_INTERVAL_XML_STR)) &&
				(NodeValueToInt(xmlTreeRoot, &me->sensors[Sensor_Humidity].readInterval, HMDT_READ_INTERVAL_XML_STR)) &&
				(NodeValueToFloat(xmlTreeRoot, &me->sensors[Sensor_Temperature].readDelta, TEMP_READ_DELTA_XML_STR)) &&
				(NodeValueToFloat(xmlTreeRoot, &me->sensors[Sensor_Humidity].readDelta, HMDT_READ_DELTA_XML_STR)) &&
				(NodeValueToInt(xmlTreeRoot, &actuatorHeartBeat, ACTUATOR_HEARTBEAT_XML_STR)))
			{
				if (me->config.heartBeat != controllerHeartBeat)
				{
					//Controller's heartbeat is changed.
					//Reset timers to new heartbeat.
					me->config.heartBeat = controllerHeartBeat;
					ResetTimerPeriod(me->config.heartBeatTimer, me->config.heartBeat);
				}

				if (me->sensorConfig.heartBeat != sensorHeartBeat)
				{
					//Sensor's heartbeat is changed.
					//Reset its expiry timer.
					me->sensorConfig.heartBeat = sensorHeartBeat;
					ResetTimerPeriod(me->sensorTimer, me->sensorConfig.heartBeat * 2);
				}

				if (me->actuatorConfig.heartBeat != actuatorHeartBeat)
				{
					//Actuator's heartbeat is changed.
					//Reset its expiry timer.
					me->actuatorConfig.heartBeat = actuatorHeartBeat;
					ResetTimerPeriod(me->actuatorTimer, me->actuatorConfig.heartBeat * 2);
				}

				if (SetOrientation(&me->sensors[Sensor_Temperature].orientation, tempOrientationString))
				{
					success = SetOrientation(&me->sensors[Sensor_Humidity].orientation, hmdtOrientationString);
				}
			}
			Tree_Delete(xmlTreeRoot);
		}
	}
	return success;
}

/**
 * Parse following events :-
 * 1. HeartBeat message sent by sensor.
 * 2. HeartBeat message sent by actuator.
 * 3. Change in measurements sent by sensor.
 */
static bool ParseEvent(TreeNode root, Controller *me, const char *deviceId)
{
	bool isSensorHeartBeat = false;
	bool isActuatorHeartBeat = false;
	Sensor sensors[NUM_SENSORS];
	Relay relays[NUM_RELAYS];
	char data[MAX_SIZE] = {0};

	if (NodeValueToString(root, data, EVENT_TYPE_XML_STR))
	{
		if (strcmp(SENSOR_STR, data) == 0)
		{
			//Check if message is coming to right sensor device
			if (me->sensorConfig.sensorId && (strcmp(me->sensorConfig.sensorId, deviceId) != 0))
			{
				ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Message received from unknown sensor(%s)", deviceId);
				return false;
			}
			else
			{
				if (!me->sensorConfig.sensorId)
				{
					//If we donot have a sensor Id, take one which first
					//sends message to us
					me->sensorConfig.sensorId = (char *)Flow_MemAlloc(MAX_SIZE);
					if (me->sensorConfig.sensorId)
					{
						strcpy(me->sensorConfig.sensorId, deviceId);
						ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sensor(%s) found", me->sensorConfig.sensorId);
					}
					else
					{
						ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Memory allocation failure for sensor's Id" );
						return false;
					}
				}

				if ((NodeValueToFloat(root, &sensors[Sensor_Temperature].value, EVENT_TEMPERATURE_XML_STR)) &&
					(NodeValueToFloat(root, &sensors[Sensor_Humidity].value, EVENT_HUMIDITY_XML_STR)))
				{
					isSensorHeartBeat = true;
				}
			}
		}
		else if (strcmp(ACTUATOR_STR, data) == 0)
		{
			//Check if message is coming to right actuator device
			if (me->actuatorConfig.actuatorId && (strcmp(me->actuatorConfig.actuatorId, deviceId) != 0))
			{
				ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Message received from unknown actuator(%s)", deviceId);
				return false;
			}
			else
			{
				if (!me->actuatorConfig.actuatorId)
				{
					//If we donot have a actuator Id, take one which first
					//sends message to us
					me->actuatorConfig.actuatorId = (char *)Flow_MemAlloc(MAX_SIZE);
					if (me->actuatorConfig.actuatorId)
					{
						strcpy(me->actuatorConfig.actuatorId, deviceId);
						ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Actuator(%s) found", me->actuatorConfig.actuatorId);
					}
					else
					{
						ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Memory allocation failure for actuator's Id" );
						return false;
					}
				}

				if (NodeValueToString(root, data, EVENT_RELAY_1_XML_STR))
				{
					if (GetRelayStatus(&relays[Relay_Heater].status, data))
					{
						isActuatorHeartBeat = true;
					}
				}

				if (NodeValueToString(root, data, EVENT_RELAY_2_XML_STR))
				{
					if (GetRelayStatus(&relays[Relay_Fan].status, data))
					{
						isActuatorHeartBeat = true;
					}
					else
					{
						isActuatorHeartBeat = false;
					}
				}
				else
				{
					isActuatorHeartBeat = false;
				}
			}
		}
	}

	if (isSensorHeartBeat)
	{
		unsigned int i;

		//Sensor is alive, reset its expiry timer
		FlowTimer_Reset(me->sensorTimer);

		if (me->sensorConfig.isAlive == false)
		{
			me->sensorConfig.isAlive = true;
			SendCommand(me, NULL, Message_DeviceStatusToUser);
			ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sensor(%s) is alive now", me->sensorConfig.sensorId);
		}

		if (IsSensorDataChanged(me->sensors, sensors))
		{
			for (i = 0; i < NUM_SENSORS; ++i)
			{
				me->sensors[i].value = sensors[i].value;
			}

			SendCommand(me, NULL, Message_SensorStatusToUser);
		}

		if (ActuatorControlLogic(me))
		{
			//There has been a change in relay status,
			//hence, send an update to user.
			SendCommand(me, NULL, Message_ActuatorStatusToUser);
		}

		return true;
	}

	if (isActuatorHeartBeat)
	{
		unsigned int i;

		//Actuator is alive, reset its expiry timer
		FlowTimer_Reset(me->actuatorTimer);

		if (me->actuatorConfig.isAlive == false)
		{
			me->actuatorConfig.isAlive = true;
			SendCommand(me, NULL, Message_DeviceStatusToUser);
			ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Actuator(%s) is alive now", me->actuatorConfig.actuatorId);
		}

		for (i = 0; i < NUM_RELAYS; ++i)
		{
			if (me->relays[i].status != relays[i].status)
			{
				char relayCmdStr[MAX_SIZE] = {0};

				ConstructRelayCmdStr(relayCmdStr, me->relays[i].relayName, me->relays[i].status);

				SendCommand(me, relayCmdStr, Message_RelayCommandToActuator);
			}
		}
		return true;
	}
	return false;
}

/**
 * Parse following commands :-
 * 1. RETRIEVE_SETTINGS command sent by user.
 * 2. Relay status change command sent by user.
 * Send relay status update to user if changed.
 */
static bool ParseCommand(TreeNode root, Controller *me)
{
	char data[MAX_SIZE] = {0};
	bool isStatusChanged = false;

	if (NodeValueToString(root, data, COMMAND_INFO_XML_STR))
	{
		if (strcmp(RETRIEVE_SETTINGS_STR, data) == 0)
		{
			//Ask for KVS config and update ourself
			me->isUserUpdate = true;
			if (!PostFlowInterfaceCmdGetSetting(&me->sendMsgQueue))
			{
				ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Posting RETRIEVE_SETTINGS command to flow interface thread failed");
			}
		}
		else if (strcmp(RELAY_1_ON_STR, data) == 0)
		{
			//Send a reponse to user for successful reception of command
			SendCommand(me, RELAY_1_ON_STR, Message_ResponseToUser);

			//Update relay's status and mode
			isStatusChanged = UpdateRelay(me, Relay_Heater, Relay_On, RELAY_1_ON_STR);
		}
		else if (strcmp(RELAY_2_ON_STR, data) == 0)
		{
			//Send a reponse to user for successful reception of command
			SendCommand(me, RELAY_2_ON_STR, Message_ResponseToUser);

			//Update relay's status and mode
			isStatusChanged = UpdateRelay(me, Relay_Fan, Relay_On, RELAY_2_ON_STR);
		}
		else if (strcmp(RELAY_1_OFF_STR, data) == 0)
		{
			//Send a reponse to user for successful reception of command
			SendCommand(me, RELAY_1_OFF_STR, Message_ResponseToUser);

			//Update relay's status and mode
			isStatusChanged = UpdateRelay(me, Relay_Heater, Relay_Off, RELAY_1_OFF_STR);
		}
		else if (strcmp(RELAY_2_OFF_STR, data) == 0)
		{
			//Send a reponse to user for successful reception of command
			SendCommand(me, RELAY_2_OFF_STR, Message_ResponseToUser);

			//Update relay's status and mode
			isStatusChanged = UpdateRelay(me, Relay_Fan, Relay_Off, RELAY_2_OFF_STR);
		}
		else if (strcmp(RELAY_1_AUTO_STR, data) == 0)
		{
			//Send a reponse to user for successful reception of command
			SendCommand(me, RELAY_1_AUTO_STR, Message_ResponseToUser);

			//Change relay to auto, and run actuator logic
			isStatusChanged = ChangeRelayModeToAuto(me, Relay_Heater);
		}
		else if (strcmp(RELAY_2_AUTO_STR, data) == 0)
		{
			//Send a reponse to user for successful reception of command
			SendCommand(me, RELAY_2_AUTO_STR, Message_ResponseToUser);

			//Change relay to auto, and run actuator logic
			isStatusChanged = ChangeRelayModeToAuto(me, Relay_Fan);
		}
		else if (strcmp(PING_STR, data) == 0)
		{
			char timeStr[MAX_SIZE] = {0};

			if (NodeValueToString(root, timeStr, COMMAND_APP_TIME_XML_STR))
			{
				SendCommand(me, timeStr, Message_PingResponseToUser);
			}
			else
			{
				return false;
			}
		}
		else
		{
			SendCommand(me, UNKNOWN_COMMAND_STR, Message_ResponseToUser);
			ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Received unknown command from user" );
		}

		if (isStatusChanged)
		{
			SendCommand(me, NULL, Message_ActuatorStatusToUser);
		}
	}
	else
	{
		return false;
	}
	return true;
}

/**
 * Parse all messages received by controller
 */
static bool ParseMessage(const ReceivedMessage *receivedMsg, Controller *me)
{
	bool success = false;
	TreeNode xmlTreeRoot = TreeNode_ParseXML((uint8_t*)receivedMsg->data, strlen(receivedMsg->data), true);

	if (xmlTreeRoot)
	{
		if (strcmp(EVENT_STR,TreeNode_GetName(xmlTreeRoot)) == 0)
		{
			//Received an event from a device
			if (ParseEvent(xmlTreeRoot, me, receivedMsg->sendorId))
			{
				success = true;
			}
		}
		else if (strcmp(COMMAND_STR,TreeNode_GetName(xmlTreeRoot)) == 0)
		{
			//Received a command from user
			if (strcmp(me->userId, receivedMsg->sendorId) == 0)
			{
				if (ParseCommand(xmlTreeRoot, me))
				{
					success = true;
				}
			}
		}
		Tree_Delete(xmlTreeRoot);
	}
	return success;
}

/**
 * Sensor's timer callback, which is getting called if controller
 * donot receive sensor's heartbeat for some fixed time. Hence
 * mark sensor as dead and send this message to user.
 */
static void SensorHeartBeatTimer(FlowTimer timer, void *context)
{
	if (context)
	{
		Controller *me = (Controller* )context;

		if (me->sensorConfig.isAlive == true)
		{
			me->sensorConfig.isAlive = false;
			SendCommand(me, NULL, Message_DeviceStatusToUser);
			ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sensor's heartbeat expiry" );
		}
	}
}

/**
 * Actuator's timer callback, which is getting called if controller
 * donot receive actuator's heartbeat for some fixed time. Hence
 * mark actuator as dead and send this message to user.
 */
static void ActuatorHeartBeatTimer(FlowTimer timer, void *context)
{
	if (context)
	{
		Controller *me = (Controller* )context;

		if (me->actuatorConfig.isAlive == true)
		{
			me->actuatorConfig.isAlive = false;
			SendCommand(me, NULL, Message_DeviceStatusToUser);
			ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Actuator's heartbeat expiry" );
		}
	}
}

/**
 * Controller's timer callback, which is getting called on every expiry
 * of controller's heartbeat. Here we are sending a heartbeat message to
 * user, which shows that controller is alive.
 */
static void ControllerHeartBeatTimer(FlowTimer timer, void *context)
{
	if (context)
	{
		FlowQueue* receiveMsgQueue = (FlowQueue* )context;

		if (!PostControllerEventHeartBeat(receiveMsgQueue))
		{
			ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Posting Controller's heartbeat event failed");
		}
	}
}

/**
 * Start sensor's heartbeat expiry timer
 */
static void StartSensorHeartBeatTimer(Controller *me)
{
	me->sensorTimer = FlowTimer_New("SensorHeartBeatTimer", SENSOR_HEARTBEAT_EXPIRY, true, SensorHeartBeatTimer, (void *)me);
	if (me->sensorTimer == NULL)
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Sensor timer creation failed");
		return;
	}

	FlowTimer_Start(me->sensorTimer);
}

/**
 * Start actuator's heartbeat expiry timer
 */
static void StartActuatorHeartBeatTimer(Controller *me)
{
	me->actuatorTimer = FlowTimer_New("ActuatorHeartBeatTimer", ACTUATOR_HEARTBEAT_EXPIRY, true, ActuatorHeartBeatTimer, (void *)me);
	if (me->actuatorTimer == NULL)
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Actuator timer creation failed");
		return;
	}

	FlowTimer_Start(me->actuatorTimer);
}

/**
 * Start controller's heartbeat timer.
 * Return true/false on timer's creation success/failure.
 * Based on return value, take decision on starting expiry timers.
 */
static bool StartControllerHeartBeatTimer(Controller *me)
{
	me->config.heartBeatTimer = FlowTimer_New("ControllerHeartBeatTimer", me->config.heartBeat, true, ControllerHeartBeatTimer, (void *)&me->receiveMsgQueue);
	if (me->config.heartBeatTimer == NULL)
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Controller timer creation failed");
		return false;
	}

	FlowTimer_Start(me->config.heartBeatTimer);
	return true;
}

/**
 * Start all timers and send updated settings to devices.
 */
static void StartTimersAndUpdateSettings(Controller *me)
{
	if (StartControllerHeartBeatTimer(me))
	{
		if (me->sensorConfig.sensorId)
		{
			StartSensorHeartBeatTimer(me);
			SendCommand(me, NULL, Message_UpdateSettingsToSensor);
			ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sending settings to sensor(%s)", me->sensorConfig.sensorId);
		}

		if (me->actuatorConfig.actuatorId)
		{
			StartActuatorHeartBeatTimer(me);
			SendCommand(me, NULL, Message_UpdateSettingsToActuator);
			ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sending settings to actuator(%s)", me->actuatorConfig.actuatorId);
		}
	}
}

/**
 * Controller thread's event handler
 */
static void ControllerEventHandler(Controller *me)
{
	ControllerEvent *event = NULL;

	for (;;)
	{
		event = FlowQueue_DequeueWaitFor(me->receiveMsgQueue,QUEUE_WAITING_TIME);
		if (event != NULL)
		{
			switch (event->evtType)
			{
				case ControllerEvent_HeartBeat:
				{
					SendCommand(me, NULL, Message_HeartBeatToUser);
					FreeEvent(event);
					break;
				}
				case ControllerEvent_SettingSuccess:
				{
					//KVS config read successful, try parsing the content
					if (ParseAndUpdateSettings(event->details, me))
					{
						//Check if we are asked for KVS config because of reception
						//of RETRIEVE_SETTINGS command from user
						if (me->isUserUpdate)
						{
							//Successfully updated the settings.
							//Send a RETRIEVE_SETTINGS_SUCCESS message to user
							me->isUserUpdate = false;
							SendCommand(me, "RETRIEVE_SETTINGS_SUCCESS", Message_ResponseToUser);
							if (me->sensorConfig.sensorId)
							{
								SendCommand(me, NULL, Message_UpdateSettingsToSensor);
								ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sending settings to sensor(%s)", me->sensorConfig.sensorId);
							}
							if (me->actuatorConfig.actuatorId)
							{
								SendCommand(me, NULL, Message_UpdateSettingsToActuator);
								ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sending settings to actuator(%s)",me->actuatorConfig.actuatorId);
							}
						}
						else
						{
							//KVS config is read first time after booting.
							//Start all timers.
							StartTimersAndUpdateSettings(me);
						}
					}
					else
					{
						//KVS config parsing is failed.
						//Send a RETRIEVE_SETTINGS_FAILURE message to user
						me->isUserUpdate = false;
						SendCommand(me, "RETRIEVE_SETTINGS_FAILURE", Message_ResponseToUser);
						ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Error in parsing settings" );
					}
					FreeEvent(event);
					break;
				}
				case ControllerEvent_SettingFailure:
				{
					char *data = NULL;
					//KVS config is not present. Create one.
					ConstructSetting(me, &data);
					if (!PostFlowInterfaceCmdSetSetting(&me->sendMsgQueue, data))
					{
						Flow_MemFree((void **)&data);
						ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Posting set settings command to flow interface thread failed");
					}

					//KVS config is set, start all timers.
					StartTimersAndUpdateSettings(me);

					FreeEvent(event);
					break;
				}
				case ControllerEvent_ReceivedMessage:
				{
					ReceivedMessage *receivedMsg;

					receivedMsg = (ReceivedMessage *)event->details;
					ParseMessage(receivedMsg, me);
					Flow_MemFree((void **)&receivedMsg->data);
					Flow_MemFree((void **)&receivedMsg->sendorId);
					FreeEvent(event);
					break;
				}
				default:
				{
					ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Received unknown event" );
					FreeEvent(event);
					break;
				}
			}
		}
	}
}

void ControllerThread(FlowThread thread, void *taskParameters)
{
	Controller *me = taskParameters;

	if (!PostFlowInterfaceCmdGetSetting(&me->sendMsgQueue))
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Posting get settings command to flow interface thread failed");
	}

	ControllerEventHandler(me);
}

