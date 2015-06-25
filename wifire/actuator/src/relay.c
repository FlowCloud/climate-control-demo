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


#include "relay.h"

#include "peripheral/ports/plib_ports.h"

#ifdef USE_LED_AS_RELAY
	#include "ui_control.h"
#endif

#define NUM_OF_RELAYS	(2)

#ifndef USE_LED_AS_RELAY
//chipkit pin # 2 and 3
//on grove board that would be port D2 and D3

#define PORT1	(PORT_CHANNEL_E)
#define PIN1	(PORTS_BIT_POS_8)

#define PORT2	(PORT_CHANNEL_D)
#define PIN2	(PORTS_BIT_POS_0)

static unsigned int relay_port[NUM_OF_RELAYS] = {PORT1, PORT2};
static unsigned int relay_pin[NUM_OF_RELAYS] = {PIN1, PIN2};

void Relay_Init(void)
{
	int i;
	for (i = 0; i < NUM_OF_RELAYS; i++)
	{
		PLIB_PORTS_PinModePerPortSelect(PORTS_ID_0, relay_port[i], relay_pin[i], PORTS_PIN_MODE_DIGITAL);
		PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, relay_port[i], relay_pin[i]);
	}
}

void Relay_Set(unsigned int relayNum, bool state)
{
	if (relayNum < NUM_OF_RELAYS)
	{
		PLIB_PORTS_PinWrite(PORTS_ID_0, relay_port[relayNum], relay_pin[relayNum], state);
	}
}


#else
void Relay_Init(void)
{
	return;
}

void Relay_Set(unsigned int relayNum, bool state)
{
	if (relayNum < NUM_OF_RELAYS)
	{
		UIControl_SetLEDMode(relayNum+1, (state == true)?UILEDMode_On:UILEDMode_Off);
	}
}
#endif








