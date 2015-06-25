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
#include <string.h>
#include <flow/core/xmltree.h>
#include "flow/flowcore.h"
#include "flow/flowmessaging.h"
#include "flow_interface.h"
#include "climate_control_logging.h"

#define MESSAGING_EXPIRY_SECONDS	(15)
#define PAGE_SIZE					(20)

char *ClimateControl_FindDevice(const char *deviceType)
{
	char *deviceId = NULL;
	FlowMemoryManager memoryManager = FlowMemoryManager_New();
	if (memoryManager)
	{
		FlowDevice loggedInDevice = FlowClient_GetLoggedInDevice(memoryManager);
		if (loggedInDevice)
		{
			FlowUser owner = FlowDevice_RetrieveOwner(loggedInDevice);
			FlowDevices myDevices = FlowUser_RetrieveOwnedDevices(owner, PAGE_SIZE);
			if (myDevices != NULL)
			{
				int index = 0;
				ClimateControl_Log(ClimateControlLogLevel_Debug,
									DEBUG_PREFIX "Number Of devices registered under this account = %d",
									FlowDevices_GetCount(myDevices)
								);
				for (index = 0; index < FlowDevices_GetCount(myDevices); ++index)
				{
					FlowDevice thisDevice = FlowDevices_GetItem(myDevices, index);
					char* currentDeviceType = NULL;
					if (FlowDevice_HasDeviceType(thisDevice))
					{
						currentDeviceType = FlowDevice_GetDeviceType(thisDevice);
						if (currentDeviceType && (strcmp(deviceType, currentDeviceType) == 0))
						{
							deviceId =  FlowString_Duplicate(FlowDevice_GetDeviceID(thisDevice));
							ClimateControl_Log(ClimateControlLogLevel_Debug,
										DEBUG_PREFIX "Device Type = %s", currentDeviceType);
							break;
						}
					}
				}
			}
			else
			{
				ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Error occurred while retrieving devices");
				ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "ERROR: code %d", FlowThread_GetLastError());
			}
		}
		else
		{
			ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Failed to get logged in device.");
		}

		FlowMemoryManager_Free(&memoryManager);
	}
	else
	{
		ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Failed to create memory manager.");
	}

	if(!deviceId)
	{
		ClimateControl_Log(ClimateControlLogLevel_Debug,
										DEBUG_PREFIX "No controller found in user's account"
									);
	}
	return deviceId;
}

bool ClimateControl_SendMessage(char* deviceId, char* msg)
{
	bool success = false;
	if (deviceId && msg)
	{
		FlowMemoryManager memoryManager = FlowMemoryManager_New();
		if (memoryManager)
		{
			if(FlowMessaging_SendMessageToDevice(deviceId, "text/plain", msg, strlen(msg), PAGE_SIZE))
			{
				success  = true;
			}
			else
			{
				ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Failed to send message.");
				ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "ERROR: code %d", FlowThread_GetLastError());
			}

			FlowMemoryManager_Free(&memoryManager);
		}
		else
		{
			ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Failed to create memory manager.");
		}
	}
	return success;
}

void RegisterCallback(FlowMessaging_MessageReceivedCallBack callback)
{
	/* Register asynchronous-message callback to receive incoming messages */
	FlowMessaging_SetMessageReceivedListenerForDevice(callback);
}
