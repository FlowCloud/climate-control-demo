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


#ifndef ACTUATOR_H
#define	ACTUATOR_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "flow/flowcore.h"
#include "flow/flowmessaging.h"
#include "queue_wrapper.h"

#define TAG_ARRAY_SIZE								(300)
#define TAG_SIZE									(70)
#define RELAY_1_XML_TAG  "<Relay_1>%s</Relay_1>"
#define RELAY_2_XML_TAG  "<Relay_2>%s</Relay_2>"

typedef enum
{
	Relay_Off,
	Relay_On
}Relay_State;

typedef enum
{
	Relay_1,
	Relay_2,
	Number_Of_Relays
}Relay_Num;

typedef struct
{
	char *cmd;
	Relay_Num num;
	Relay_State state;
}Relay_Config;


typedef struct
{
	unsigned int heartBeat;
}ClimateActuatorConfig;


typedef struct
{
    char *cmd;
    void *payload;
}ControllerCmd;

typedef struct
{
	Relay_State state;
	char *xmlTagString;
}Relay;

// currently all ClimateActuator datastructure are used in ClimateActuatorThread
// using it in other thread would require synchronization
typedef struct
{

	QueueHandle sendMessageQueue; //Queue between ClimateActuatorThread -> SendMessageThread
	//To send self commands ClimateActuatorThread->ClimateActuatorThread
	QueueHandle cmdQueue;
	FlowThread climateActuatorThread;
	FlowThread sendMessageThread;
	Relay relays[Number_Of_Relays];
	ClimateActuatorConfig actuatorConfig;
	char *controllerId;
	FlowTimer heartBeatTimer;
}ClimateActuator;

/**
 * \memberof ClimateActuator
 * \param
 * \brief Initialize the ClimateActuator
 *
*/
void ClimateActuator_Initialize(ClimateActuator* me);

/**
 * \memberof ClimateActuator
 * \param
 * \brief Handles Actuator Commands
 *
*/
void ClimateActuator_CommandsHandler(ClimateActuator* me, const char* command);

/**
  * \param message from the device
 * \brief callback to receive asynchronus messages
 *
*/
void MessageReceptionCallback(FlowMessagingMessage message);

#ifdef	__cplusplus
}
#endif

#endif	/* ACTUATOR_H */

