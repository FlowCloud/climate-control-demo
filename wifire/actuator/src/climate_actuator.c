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


#include "actuator.h"
#include "app.h"
#include "relay.h"
#include "flow/flowcore.h"
#include "flow/flowmessaging.h"
#include "flow_task_priority.h"
#include <math.h>
#include "queue_wrapper.h"
#include "climate_control_logging.h"
#include "send_message.h"
#include "flow_interface.h"

#define CLIMATE_ACTUATOR_CMD_QUEUE_SIZE     (10)
#define DEFAULT_HEART_BEAT_PERIOD			(15*1000) //millisecond
#define MAX_SIZE                            (256)
#define ON_STR								"ON"
#define OFF_STR								"OFF"
#define CONTROLLER_DEVICE_TYPE				"ClimateControlDemoController"
#define UPDATE_SETTINGS_STR					"UPDATE_SETTINGS"

typedef enum
{
	ClimateActuatorCmd_ControllerCmd,
	ClimateActuatorCmd_SendHeartBeat,
}ClimateActuatorCmd_Type;

typedef struct
{
	ClimateActuatorCmd_Type cmdType;
	ControllerCmd *details;
}ClimateActuatorCmd;

static const Relay_Config relayConfig[] =
{
	{"RELAY_1_ON", Relay_1, Relay_On},
	{"RELAY_1_OFF", Relay_1, Relay_Off},
	{"RELAY_2_ON", Relay_2, Relay_On},
	{"RELAY_2_OFF", Relay_2, Relay_Off},
};

struct tm *gmtime_r(const time_t *timep, struct tm *result);

/*============================================================================*/
/*                     FUNCTIONS (LOCAL)									  */
/*============================================================================*/

static void ClimateActuatorThread(FlowThread thread, void *taskParameters);
static void CommandsHandlerInternal(ClimateActuator* me, ControllerCmd* command);
static ControllerCmd* ParseMsgAndCreateControllerCmd(const char* msgString);
static void CreateAndQueueRelayStateMsg(ClimateActuator* me);
static char* CreateHeartBeatMsg(ClimateActuator *me);
static void SetRelayState(ClimateActuator* me, Relay_Num relayNum, Relay_State state);
static bool NodeValueToInt(TreeNode root, unsigned int* valueToSet, char* nodeName);
static void FreeControllerCmd(ControllerCmd *command);
static ControllerCmd* CreateControllerCmd(char *cmd, const char *msgString, TreeNode xmlTreeRoot);
static int  FindCommand(const char *cmd);

static inline char* GetRelayStatusString(Relay_State state);

static  int FindCommand(const char *cmd)
{
	unsigned int j, relayConfigItems;
	relayConfigItems = sizeof(relayConfig)/ sizeof(relayConfig[0]);
	for (j = 0; j < relayConfigItems; ++j)
	{
		if (strcmp(relayConfig[j].cmd, cmd) == 0)
		{
			return j;
		}
	}
	return -1;
}

static inline char* GetRelayStatusString(Relay_State state)
{
	return (state == Relay_On) ? ON_STR : OFF_STR;
}

static void SetRelayState(ClimateActuator* me, Relay_Num relayNum, Relay_State state)
{
	if (me->relays[relayNum].state != state)
	{
		me->relays[relayNum].state = state;
		ClimateControl_Log(ClimateControlLogLevel_Debug,
				DEBUG_PREFIX "Setting Relay %d state %s",
				relayNum+1,
				GetRelayStatusString(state));

		Relay_Set(relayNum, state == Relay_On);
	}
}

void HeartBeatCmdTimerHandler(FlowTimer timer, void *context)
{
	if (context)
	{
		QueueHandle* cmdQueue = context;
		ClimateActuatorCmd cmd;
		cmd.cmdType = ClimateActuatorCmd_SendHeartBeat;
		cmd.details = NULL;
		if (false == QueueSend(cmdQueue, &cmd))
		{
			ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "ClimateActuator thread can't post in queue %d/%d",
								QueueNumOfItems(cmdQueue),
								CLIMATE_ACTUATOR_CMD_QUEUE_SIZE);
		}
	}
}

static void CreateAndQueueRelayStateMsg(ClimateActuator* me)
{
	MeasurementMsg msg;
	char* msgDetails = CreateHeartBeatMsg(me);
	msg.details = msgDetails;
	msg.deviceId = FlowString_Duplicate(me->controllerId);
	if (msg.details && msg.deviceId)
	{
		ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Sending message");
		if (false == QueueSend(me->sendMessageQueue, &msg))
		{
			ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Enqueing sendMessageQueue failed");
			Flow_MemFree((void **)&msgDetails);
			Flow_MemFree((void **)&msg.deviceId);
		}
		else
		{
			struct tm date;
			time_t time;
			Flow_GetTime(&time);
			gmtime_r(&time, &date);
			ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "Relay_1 = %s, Relay_2 = %s Time = %d:%d:%d",
								GetRelayStatusString(me->relays[Relay_1].state),
								GetRelayStatusString(me->relays[Relay_2].state),
								date.tm_hour,
								date.tm_min,
								date.tm_sec);
		}
	}
	else
	{
		if (msg.details)
		{
			Flow_MemFree((void**)&(msg.details));
		}
		if (msg.deviceId)
		{
			Flow_MemFree((void**)&(msg.deviceId));
		}
		ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Creation of measurement msg failed");
	}

}
static char* CreateHeartBeatMsg(ClimateActuator *me)
{
	char *tempString = NULL;
	time_t time;
	unsigned int i;
	Flow_GetTime(&time);
	struct tm timeNow;
	gmtime_r(&time, &timeNow);
	char tagArray[TAG_ARRAY_SIZE] = {0};
	for (i = 0; i < Number_Of_Relays ; ++i)
	{
		unsigned int len;
		char tag[TAG_SIZE] = {0};
		len = strlen(me->relays[i].xmlTagString) + 5;
		snprintf(tag, len, me->relays[i].xmlTagString, GetRelayStatusString(me->relays[i].state));
		strncat(tagArray, tag, len);
	}
	const char *msgXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<event>"
								"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
								"<type>Actuator</type>"
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
				tagArray
				);
	}
	return tempString;
}

static void CleanUp(ClimateActuator* me)
{
	if (me->sendMessageQueue)
	{
		QueueDelete(me->sendMessageQueue);
	}
	if (me->cmdQueue)
	{
		QueueDelete(me->cmdQueue);
	}
	if (me->climateActuatorThread)
	{
		FlowThread_Free(me->climateActuatorThread);
	}
}

static ControllerCmd* CreateControllerCmd(char *cmd, const char *msgString, TreeNode xmlTreeRoot)
{
	ControllerCmd *controllerCmd  = NULL;
	bool success = false;
	controllerCmd = (ControllerCmd *) Flow_MemAlloc(sizeof (ControllerCmd));
	if (controllerCmd)
	{
		controllerCmd->cmd = NULL;
		controllerCmd->payload = NULL;
		unsigned int len = strlen(cmd);
		controllerCmd->cmd = (char *) Flow_MemAlloc(len + 1);
		if (controllerCmd->cmd)
		{
			strcpy(controllerCmd->cmd, cmd);
			if (strcmp(UPDATE_SETTINGS_STR, cmd) == 0)
			{
				unsigned int heartBeat;
				if (NodeValueToInt(xmlTreeRoot, &heartBeat, "command/settings/HeartBeat"))
				{
					controllerCmd->payload = Flow_MemAlloc(sizeof (ClimateActuatorConfig));
					if (controllerCmd->payload)
					{
						((ClimateActuatorConfig *) controllerCmd->payload)->heartBeat = heartBeat;
						success = true;
					}
				}
			}
			else
			{
				success = true;
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
			//<command>
			//	<time type="datetime">2014-11-28T15:30:09Z</time>
			//	<info>UPDATE_SETTINGS/RELAY_1_ON/RELAY_1_OFF/RELAY_2_ON/RELAY_2_OFF</info>
			//      <settings>
			//            <HeartBeat>1000</HeartBeat>
			//      </seettings>
			//</command>
			//
			TreeNode eventDetails = TreeNode_Navigate(xmlTreeRoot, "command/info");
			if (eventDetails)
			{
				char *buf = (char *)TreeNode_GetValue(eventDetails);
				if (buf && *buf)
				{
					controllerCmd = CreateControllerCmd(buf, msgString, xmlTreeRoot);
				}
			}
			Tree_Delete(xmlTreeRoot);
		}
	}
	return controllerCmd;
}

static void UpdateSettings(ClimateActuator* me, ClimateActuatorConfig* config)
{
	if(me->actuatorConfig.heartBeat != config->heartBeat)
	{
		ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "Changed heartbeat Timer period from %u msec  to %u msec", me->actuatorConfig.heartBeat, config->heartBeat);
		me->actuatorConfig.heartBeat = config->heartBeat;
		if(me->heartBeatTimer)
		{
			FlowTimer_SetPeriod(me->heartBeatTimer, me->actuatorConfig.heartBeat);
		}
	}
}

static void CommandsHandlerInternal(ClimateActuator* me, ControllerCmd* command)
{
	ClimateControl_Log(ClimateControlLogLevel_Info, DEBUG_PREFIX "Actuator cmd %s", command->cmd);

	if (strcmp(UPDATE_SETTINGS_STR, command->cmd) == 0)
	{
		UpdateSettings(me, command->payload);
	}
	else
	{
		int i = FindCommand(command->cmd);
		if (i >= 0)
		{
			SetRelayState(me, relayConfig[i].num, relayConfig[i].state);
		}
		else
		{
			ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Unknown command received:\t%s", command->cmd);
		}
	}
}

static void ClimateActuatorThread(FlowThread thread, void *taskParameters)
{
	// Actuator
	// 1.Wait for cmd.
	// 2.If received ClimateActuatorCmd_SendHeartBeat cmd then create and queue heartbeat msg
	// to send it to the controller.
	// 3.If received ClimateActuatorCmd_ControllerCmd then send it to command handler.

	ClimateActuator* me = taskParameters;
	me->heartBeatTimer = FlowTimer_New("HeartBeatCmdTimerHandler", me->actuatorConfig.heartBeat, true, HeartBeatCmdTimerHandler, (void *)me->cmdQueue);
	if (me->heartBeatTimer == NULL)
	{
		ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Timer creation failed");
		// can't do much return
		return;
	}
	FlowTimer_Start(me->heartBeatTimer);
	while(me)
	{
		//check for any commands
		ClimateActuatorCmd cmd;
		if (false == QueueReceive(me->cmdQueue, &cmd, -1))
		{
			continue;
		}
		switch(cmd.cmdType)
		{
			case ClimateActuatorCmd_SendHeartBeat:
			{
				ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Command received ClimateActuatorCmd_SendHeartBeat");
				CreateAndQueueRelayStateMsg(me);
				break;
			}
			case ClimateActuatorCmd_ControllerCmd:
			{
				ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Command received ClimateActuatorCmd_ControllerCmd");
				CommandsHandlerInternal(me, cmd.details);
				FreeControllerCmd(cmd.details);
				break;
			}
			default:
			{
				ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Unknown command received");
				break;
			}
		}
	}

	if (me->heartBeatTimer)
	{
		FlowTimer_Stop(me->heartBeatTimer);
		FlowTimer_Free(me->heartBeatTimer);
	}
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

static void FreeControllerCmd(ControllerCmd *command)
{
	if (command)
	{
		if(command->cmd)
		{
			Flow_MemFree((void**)&(command->cmd));
		}
		if(command->payload)
		{
			Flow_MemFree((void**)&(command->payload));
		}
		Flow_MemFree((void**)&(command));
	}
}

/*============================================================================*/
/*                     FUNCTIONS (EXPORTED)									  */
/*============================================================================*/

void ClimateActuator_Initialize(ClimateActuator* me)
{
	ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "---------Initialize Climate Actuator------" );
	bool success = false;
	//Try infinitely to look for controller, if there is no controller then there
	//is no point in carrying out rest of the operations
	while (1)
	{
		me->controllerId = ClimateControl_FindDevice(CONTROLLER_DEVICE_TYPE);
		if (me->controllerId)
		{
			ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "Found device of type %s", CONTROLLER_DEVICE_TYPE);
			ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Device id  = %s", me->controllerId);
			break;
		}
		ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "Couldn't find device of type %s", CONTROLLER_DEVICE_TYPE);
		FlowThread_Sleep(NULL, 5);
	}
	me->actuatorConfig.heartBeat = DEFAULT_HEART_BEAT_PERIOD;
	Relay_Init();
	me->sendMessageQueue = QueueCreate(SEND_MESSAGE_QUEUE_SIZE, sizeof (MeasurementMsg));
	me->cmdQueue = QueueCreate(CLIMATE_ACTUATOR_CMD_QUEUE_SIZE, sizeof (ClimateActuatorCmd));
	if (me->sendMessageQueue != 0 && me->cmdQueue != 0)
	{
		me->climateActuatorThread = FlowThread_New("ClimateActuatorThread",
			USER_TASK_PRIORITY,
			configMINIMAL_STACK_SIZE,
			ClimateActuatorThread,
			me);
		if (me->climateActuatorThread != NULL)
		{
			me->sendMessageThread = FlowThread_New("SendMessageThread", USER_TASK_PRIORITY,
				configMINIMAL_STACK_SIZE, SendMessageThread, me->sendMessageQueue);
			if (me->sendMessageThread != NULL)
			{
				success = true;
			}
			else
			{
				ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Creation of send message thread failed");
			}
		}
		else
		{
			ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Creation of climate Actuator thread failed");
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
	//start with Relay_1 Off and Relay_2 Off
	me->relays[Relay_1].state = Relay_Off;
	me->relays[Relay_2].state = Relay_Off;
	Relay_Set(Relay_1, false);
	Relay_Set(Relay_2, false);
	//sending initial status so that controller can compare it with it's stored
	//relays status set by the user and send us a msg if they differ.
	CreateAndQueueRelayStateMsg(me);
	//register callback to receive

	RegisterCallback(MessageReceptionCallback);
	ClimateControl_Log(ClimateControlLogLevel_Info, INFO_PREFIX "---------Initialization of Climate Actuator Done------" );
}

void ClimateActuator_CommandsHandler(ClimateActuator* me, const char* msgString)
{
	ControllerCmd *controllercmd = ParseMsgAndCreateControllerCmd(msgString);
	if (NULL == controllercmd)
	{
		ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Could not parse received message");
		if (msgString)
		{
			ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Unknown message received  =  %s", msgString);
		}
	}
	else
	{
		ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Actuator cmd %s", controllercmd->cmd);
		ClimateActuatorCmd cmd;
		cmd.cmdType = ClimateActuatorCmd_ControllerCmd;
		cmd.details = controllercmd;
		//Queue command if it fails there is nothing we can do.
		if (false == QueueSend(me->cmdQueue, &cmd))
		{
			FreeControllerCmd(controllercmd);
		}
	}
}
