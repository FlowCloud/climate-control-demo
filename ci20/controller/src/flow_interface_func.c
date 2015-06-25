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
#include <string.h>
#include <flow/flowmessaging.h>
#include "flow_interface_func.h"

#include "controller_logging.h"

#define DEVICE_ID_SIZE (50)

bool InitialiseLibFlowMessaging(const char *url, const char *key, const char *secret)
{
	if (FlowCore_Initialise())
	{
		FlowCore_RegisterTypes();

		if (FlowMessaging_Initialise())
		{
			if (FlowClient_ConnectToServer(url, key, secret, false))
			{
				return true;
			}
			else
			{
				ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to connect to server.");
			}
		}
		else
		{
			ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Flow Messaging initialization failed.");
		}
	}
	else
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Flow Core initialization failed.");
	}
	return false;
}

bool RegisterDevice(char *deviceType, char *macAddr, char *serialNum, char *version, char *name, char *devRegKey)
{
	FlowMemoryManager memoryManager = FlowMemoryManager_New();

	if (memoryManager)
	{
		if (FlowClient_LoginAsDevice(deviceType, macAddr, serialNum, NULL, version, name, devRegKey))
		{
			FlowMemoryManager_Free(&memoryManager);
			return true;
		}
		else
		{
			ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to login as device.");
		}
		FlowMemoryManager_Free(&memoryManager);
	}
	else
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to create memory manager.");
	}
	return false;
}

bool GetSetting(char *configType, char **data)
{
	FlowMemoryManager memoryManager = FlowMemoryManager_New();

	if (memoryManager)
	{
		FlowDevice device = FlowClient_GetLoggedInDevice(memoryManager);
		if (device)
		{
			FlowSetting flowSetting = FlowDevice_RetrieveSetting(device, configType);
			if (flowSetting && FlowSetting_HasValue(flowSetting))
			{
				char *tmp;

				tmp = FlowSetting_GetValue(flowSetting);
				*data = (char *)Flow_MemAlloc(strlen(tmp) + 1);
				if (*data)
				{
					strcpy(*data,tmp);
					FlowMemoryManager_Free(&memoryManager);
					return true;
				}
			}
			else
			{
				ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to retrieve device setting.");
			}
		}
		else
		{
			ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to get logged in device.");
		}
		FlowMemoryManager_Free(&memoryManager);
	}
	else
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to create memory manager.");
	}
	return false;
}

bool SetSetting(char *configType, char *data)
{
	FlowMemoryManager memoryManager = FlowMemoryManager_New();

	if (memoryManager)
	{
		FlowDevice device = FlowClient_GetLoggedInDevice(memoryManager);
		if (device && FlowDevice_CanRetrieveSettings(device))
		{
			FlowSettings devicesettings = FlowDevice_RetrieveSettings(device, FLOW_DEFAULT_PAGE_SIZE);
			if (devicesettings)
			{
				FlowSettings_SaveSetting(memoryManager, devicesettings, configType, data);
				if (Flow_GetLastError() == FlowError_NoError)
				{
					ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Settings saved = %s",data);
					FlowMemoryManager_Free(&memoryManager);
					return true;
				}
				else
				{
					ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Error in saving settings.");
				}
			}
			else
			{
				ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to retrieve device settings.");
			}
		}
		else
		{
			ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to get logged in device.");
		}
		FlowMemoryManager_Free(&memoryManager);
	}
	else
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to create memory manager.");
	}
	return false;
}

bool GetUserId(char *userId)
{
	FlowMemoryManager memoryManager = FlowMemoryManager_New();

	if (memoryManager)
	{
		FlowDevice device = FlowClient_GetLoggedInDevice(memoryManager);
		if (device)
		{
			FlowID temp;

			temp = FlowUser_GetUserID(FlowDevice_RetrieveOwner(device));
			strcpy(userId, temp);
			FlowMemoryManager_Free(&memoryManager);
			return true;
		}
		else
		{
			ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to get logged in device.");
		}
		FlowMemoryManager_Free(&memoryManager);
	}
	else
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to create memory manager.");
	}
	return false;
}

bool SendMessage(char *id, char *message, SendMessage_Type msgType)
{
	bool success = false;

	FlowMemoryManager memoryManager = FlowMemoryManager_New();

	if (memoryManager)
	{
		switch (msgType)
		{
			case SendMessage_ToUser:
			{
				if (FlowMessaging_SendMessageToUser((FlowID)id, "text/plain", message, strlen(message), 20))
				{
					ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Message sent to user = %s",message);
					success = true;
				}
				else
				{
					ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to send message to user.");
				}
				break;
			}
			case SendMessage_ToDevice:
			{
				if (FlowMessaging_SendMessageToDevice((FlowID)id, "text/plain", message, strlen(message), 20))
				{
					ControllerLog(ControllerLogLevel_Debug, DEBUG_PREFIX "Message sent to device = %s",message);
					success = true;
				}
				else
				{
					ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to send message to device.");
				}
				break;
			}
			default:
			{
				ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Unknown type to send message");
				break;
			}
		}
		FlowMemoryManager_Free(&memoryManager);
	}
	else
	{
		ControllerLog(ControllerLogLevel_Error, ERROR_PREFIX "Failed to create memory manager.");
	}
	return success;
}

void RegisterCallbackForReceivedMsg(FlowMessaging_MessageReceivedCallBack callback)
{
	 FlowMessaging_SetMessageReceivedListenerForDevice(callback);
}

void GetDeviceId(const char *deviceType, const char *userId, char **deviceId)
{
	int index = 0;

	FlowMemoryManager memoryManager = FlowMemoryManager_New();
	if (memoryManager)
	{
		FlowAPI api = FlowClient_GetAPI(memoryManager);

		if (FlowAPI_CanRetrieveUser(api))
		{
			FlowUser user = FlowAPI_RetrieveUser(api, (FlowID)userId);

			FlowDevices myDevices = FlowUser_RetrieveOwnedDevices(user, 20);
			if (myDevices != NULL)
			{
				for (index = 0; index < FlowDevices_GetCount(myDevices); index++)
				{
					FlowDevice thisDevice = FlowDevices_GetItem(myDevices, index);
					if (strcmp(deviceType, FlowDevice_GetDeviceType(thisDevice)) == 0)
					{
						*deviceId = FlowString_Duplicate((char *)FlowDevice_GetDeviceID(thisDevice));
						break;
					}
				}
			}
		}
		FlowMemoryManager_Free(&memoryManager);
	}
}
