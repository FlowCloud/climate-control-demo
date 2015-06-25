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

#include <flow/flowmessaging.h>
#include <string.h>

#include "controller.h"
#include "controller_logging.h"
#include "version.h"
#include "flow_interface.h"
#include "flow_interface_func.h"

#define CONTROLLER_CONFIG_NAME "ControllerConfig"
#define SENSOR_DEVICE_TYPE "ClimateControlDemoSensor"
#define ACTUATOR_DEVICE_TYPE "ClimateControlDemoActuator"

FlowQueue *_receiveMsgQueue;

typedef struct
{
	char url[MAX_SIZE];
	char key[MAX_SIZE];
	char secret[MAX_SIZE];
	char deviceType[MAX_SIZE];
	char deviceMACAddress[MAX_SIZE];
	char deviceSerialNumber[MAX_SIZE];
	char deviceName[MAX_SIZE];
	char devRegKey[MAX_SIZE];
}RegistrationData;

static void FreeCmd(FlowInterfaceCmd *cmd)
{
	if (cmd->details)
	{
		Flow_MemFree((void **)&cmd->details);
	}
	Flow_MemFree((void **)&cmd);
}

/**
 * Send an event to controller thread for success/failure of reading KVS config
 * data - Device setting read from datastore.
 *        In case of any failure, user should free any memory allocated to it.
 *        Otherwise, would be freed by receiver if successfully added to queue.
 */
static bool PostControllerEventSetting(const FlowQueue* receiveMsgQueue, char *data)
{
	bool success = false;
	ControllerEvent *event = NULL;

	//Allocated memory should be freed by the receiver
	event = (ControllerEvent *)Flow_MemAlloc(sizeof(ControllerEvent));
	if (event)
	{
		if (data)
		{
			event->evtType = ControllerEvent_SettingSuccess;
		}
		else
		{
			event->evtType = ControllerEvent_SettingFailure;
		}

		event->details = data;
		success = FlowQueue_Enqueue(*receiveMsgQueue, event);

		if (!success)
		{
			Flow_MemFree((void **)&event);
		}
	}

	return success;
}

/**
 * Send an event to controller thread on reception of any message.
 * data - Pointer to received message, should be copied for future usage.
 * sendorId - Pointer to user/device ID, should be copied for future usage.
 * datasize - Received message size.
 */
static bool PostControllerEventReceivedMessage(const FlowQueue* receiveMsgQueue, const char *data, const char *sendorId, const unsigned int datasize)
{
	bool success = false;
	ControllerEvent *event = NULL;
	ReceivedMessage *receivedMsg = NULL;

	event = (ControllerEvent *)Flow_MemAlloc(sizeof(ControllerEvent));

	if (event)
	{
		event->evtType = ControllerEvent_ReceivedMessage;
		receivedMsg = (ReceivedMessage *)Flow_MemAlloc(sizeof(ReceivedMessage));

		if (receivedMsg)
		{
			//Allocate one extra byte for data, as the datasize we got from
			//FlowMessagingMessage_GetContentLength() doesn't include null terminator
			receivedMsg->data = (char *)Flow_MemAlloc(datasize + 1);
			receivedMsg->sendorId = (char *)Flow_MemAlloc(strlen(sendorId) + 1);

			if (receivedMsg->data && receivedMsg->sendorId)
			{
				strncpy(receivedMsg->data, data, datasize);
				receivedMsg->data[datasize] = '\0';
				strcpy(receivedMsg->sendorId, sendorId);
				event->details = receivedMsg;
				success = FlowQueue_Enqueue(*receiveMsgQueue, event);
			}

			if (!success)
			{
				if (receivedMsg->data)
				{
					Flow_MemFree((void **)&receivedMsg->data);
				}

				if (receivedMsg->sendorId)
				{
					Flow_MemFree((void **)&receivedMsg->sendorId);
				}

				Flow_MemFree((void **)&receivedMsg);
			}
		}

		if (!success)
		{
			Flow_MemFree((void **)&event);
		}
	}

	return success;
}

/**
 * Callback registered for reception of messages
 */
void MessageReceivedCallBack(FlowMessagingMessage message)
{
	char *data = NULL, *sendorId = NULL;
	unsigned datasize= 0;

	data = FlowMessagingMessage_GetContent(message);
	datasize = FlowMessagingMessage_GetContentLength(message);
	sendorId = FlowMessagingMessage_GetSenderUserID(message);

	if (!sendorId)
	{
		//Message is not sent by user, hence it must be a device
		sendorId = FlowMessagingMessage_GetSenderDeviceID(message);
	}

	if (!PostControllerEventReceivedMessage(_receiveMsgQueue, data, sendorId, datasize))
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Posting message received event to controller thread failed");
	}

	ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Received message = %s",data);
}

/**
 * Save device registration settings in a configuration file
 */
static void SetConfigurationFile(const RegistrationData *regData)
{
	FILE *configFile = fopen("flow_controller.cnf", "w");

	if (configFile)
	{
		fprintf(configFile, "Server_Address=%s\n", regData->url);
		fprintf(configFile, "Auth_Key=%s\n", regData->key);
		fprintf(configFile, "Secret_Key=%s\n", regData->secret);
		fprintf(configFile, "Device_Type=%s\n", regData->deviceType);
		fprintf(configFile, "MAC_Addr=%s\n", regData->deviceMACAddress);
		fprintf(configFile, "Serial_Num=%s\n", regData->deviceSerialNumber);
		fprintf(configFile, "Device_Name=%s\n", regData->deviceName);
		fprintf(configFile, "Devreg_Key=%s\n", regData->devRegKey);
		fclose(configFile);
	}
}

/**
 * Read device resgitration settings from configuration file.
 * And if not found, ask user for settings.
 */
static void GetConfigData(RegistrationData *regData)
{
	FILE *configFile = fopen("flow_controller.cnf", "r");

	if (!configFile)
	{
		printf("Creating configuration file..\n");

		printf("Please provide your own configuration values:\n");
		/* user input for Server Address */
		printf("Enter Server Address: ");
		scanf("%s", regData->url);
		/* user input for Auth Key */
		printf("Enter Auth Key: ");
		scanf("%s", regData->key);
		/* user input for Secret Key */
		printf("Enter Secret Key: ");
		scanf("%s", regData->secret);
		/* user input for Device Type */
		printf("Enter Device Type:");
		scanf("%s", regData->deviceType);
		/* user input for MAC Address */
		printf("Enter Device MAC Address:");
		scanf("%s", regData->deviceMACAddress);
		/* user input for Serial Number */
		printf("Enter Device Serial Number:");
		scanf("%s", regData->deviceSerialNumber);
		/* user input for device name */
		printf("Enter Device Name:");
		scanf("%s", regData->deviceName);
		/* user input for Registration token */
		printf("Enter Device Registration Token:");
		scanf("%s", regData->devRegKey);

		SetConfigurationFile(regData);
	}
	else
	{
		fscanf(configFile, "Server_Address=%s\n", regData->url);
		fscanf(configFile, "Auth_Key=%s\n", regData->key);
		fscanf(configFile, "Secret_Key=%s\n", regData->secret);
		fscanf(configFile, "Device_Type=%s\n", regData->deviceType);
		fscanf(configFile, "MAC_Addr=%s\n", regData->deviceMACAddress);
		fscanf(configFile, "Serial_Num=%s\n", regData->deviceSerialNumber);
		fscanf(configFile, "Device_Name=%s\n", regData->deviceName);
		fscanf(configFile, "Devreg_Key=%s\n", regData->devRegKey);
		fclose(configFile);
	}
}

/**
 * Flow interface thread :-
 * 1. Registers a message callback for any message received.
 * 2. Handles all the commands coming from controller thread.
 */
void FlowInterfaceThread(FlowThread thread, void *taskParameters)
{
	Controller *me = taskParameters;

	FlowInterfaceCmd *cmd = NULL;

	_receiveMsgQueue = &me->receiveMsgQueue;
	RegisterCallbackForReceivedMsg(MessageReceivedCallBack);

	for (;;)
	{
		cmd = FlowQueue_DequeueWaitFor(me->sendMsgQueue,QUEUE_WAITING_TIME);
		if (cmd != NULL)
		{
			switch (cmd->cmdType)
			{
				case FlowInterfaceCmd_SendMessageToUser:
				{
					SendMessage(me->userId, cmd->details,SendMessage_ToUser);
					FreeCmd(cmd);
					break;
				}
				case FlowInterfaceCmd_SendMessageToActuator:
				{
					if (me->actuatorConfig.actuatorId)
					{
						SendMessage(me->actuatorConfig.actuatorId, cmd->details,SendMessage_ToDevice);
						ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sending relay command to actuator" );
					}
					else
					{
						ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Actuator id is NOT known to us" );
					}
					FreeCmd(cmd);
					break;
				}
				case FlowInterfaceCmd_SendMessageToSensor:
				{
					if (me->sensorConfig.sensorId)
					{
						SendMessage(me->sensorConfig.sensorId, cmd->details,SendMessage_ToDevice);
						ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sending update settings to sensor" );
					}
					else
					{
						ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Sensor id is NOT known to us" );
					}
					FreeCmd(cmd);
					break;
				}
				case FlowInterfaceCmd_GetSetting:
				{
					char *data = NULL;

					if (GetSetting(CONTROLLER_CONFIG_NAME, &data))
					{
						if (!PostControllerEventSetting(&me->receiveMsgQueue, data))
						{
							Flow_MemFree((void **)&data);
							ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Posting settings event to controller thread failed");
						}
					}
					else
					{
						if (!PostControllerEventSetting(&me->receiveMsgQueue, NULL))
						{
							ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Posting settings event to controller thread failed");
						}
					}
					FreeCmd(cmd);
					break;
				}
				case FlowInterfaceCmd_SetSetting:
				{
					if (!SetSetting(CONTROLLER_CONFIG_NAME, cmd->details))
					{
						ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Error in saving settings" );
					}
					FreeCmd(cmd);
					break;
				}
				default:
				{
					ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Received unknown command" );
					FreeCmd(cmd);
					break;
				}
			}
		}
	}
}

bool InitializeFlowInterface(Controller *me)
{
	RegistrationData regData;

	GetConfigData(&regData);

	if (InitialiseLibFlowMessaging((const char *)regData.url, (const char *)regData.key,(const char *)regData.secret))
	{
		if (RegisterDevice(regData.deviceType,
						regData.deviceMACAddress,
						regData.deviceSerialNumber,
						DEVICE_SOFTWARE_VERSION,
						regData.deviceName,
						regData.devRegKey))
		{
			if (GetUserId(me->userId))
			{
				GetDeviceId(SENSOR_DEVICE_TYPE, me->userId, &me->sensorConfig.sensorId);
				GetDeviceId(ACTUATOR_DEVICE_TYPE, me->userId, &me->actuatorConfig.actuatorId);
				printf("Flow Interface initialized successfully\n");
				return true;
			}
		}
	}

	printf("Flow Interface initialization failed\n");
	return false;
}

