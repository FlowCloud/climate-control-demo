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


#include "send_message.h"
#include "climate_control_logging.h"
#include "flow_interface.h"
#include "queue_wrapper.h"


#define FLOW_RETRY_TIMES				(3)

void SendMessageThread(FlowThread thread, void *taskParameters)
{
	ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Send message thread started");
	QueueHandle* sendMessageQueue = taskParameters;
	while (sendMessageQueue)
	{
		MeasurementMsg msg;
		if (QueueReceive(sendMessageQueue, &msg, -1))
		{
			unsigned int startTick = FlowTimer_GetTickCount();
			unsigned int i;
			// retry 3 times if send message fails
			for (i = 1; i <= FLOW_RETRY_TIMES; i++)
			{
				if (ClimateControl_SendMessage(msg.deviceId, msg.details))
				{
					break;
				}
				ClimateControl_Log(ClimateControlLogLevel_Error, ERROR_PREFIX "Sending msg failed");
			}
			ClimateControl_Log(ClimateControlLogLevel_Debug, DEBUG_PREFIX "Send Message q(%d/%d) %d t",
									QueueNumOfItems(sendMessageQueue),
									SEND_MESSAGE_QUEUE_SIZE,
									FlowTimer_GetTickCount()-startTick);
			Flow_MemFree((void**)&(msg.details));
			Flow_MemFree((void**)&(msg.deviceId));
		}
	}
}
