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


#ifndef QUEUEWRAPPER_C
#define	QUEUEWRAPPER_C

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Wrapper for Freertos Queue implementation
 */
#include "queue_wrapper.h"

QueueHandle QueueCreate(unsigned int noOfItems, unsigned int sizeOfItem)
{
	return xQueueCreate(noOfItems, sizeOfItem);
}

void QueueDelete(QueueHandle queue)
{
	vQueueDelete(queue);
}

bool QueueSend(QueueHandle queue, const void *data)
{
	BaseType_t ret;
	ret = xQueueSend(queue, data, 0);
	if (ret == pdTRUE)
		return true;
	else // could  be full
		return false;
}

bool QueueReceive(QueueHandle queue, void * data, int msec)
{
	BaseType_t ret;
	TickType_t ticks;

	if (msec == -1)
		ticks = portMAX_DELAY;
	else
		ticks = msec/portTICK_RATE_MS;

	ret = xQueueReceive(queue, data, ticks);
	if (ret == pdTRUE)
		return true;
	else 
		return false;
}

unsigned int QueueNumOfItems(QueueHandle queue)
{
	return uxQueueMessagesWaiting(queue);
}


#ifdef	__cplusplus
}
#endif

#endif	/* QUEUEWRAPPER_C */

