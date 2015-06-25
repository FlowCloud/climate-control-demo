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

#include "controller.h"
#include "flow/core/flow_time.h"
#include "flow/core/flow_memalloc.h"
#define ON_STR "ON"
#define OFF_STR "OFF"
#define ALIVE_STR "ALIVE"
#define DEAD_STR "DEAD"
#define AUTO_STR "AUTO"
#define MANUAL_STR "MANUAL"
#define ABOVE_STR "ABOVE"
#define BELOW_STR "BELOW"

#define INT_STR_SIZE (10)
#define TAG_SIZE (100)

struct tm *gmtime_r(const time_t *timep, struct tm *result);

static bool isRelayOn(Relay_Status status)
{
	return status == Relay_On;
}

static bool isRelayAuto(Relay_Mode mode)
{
	return mode == Relay_Auto;
}

static bool isOrientationAbove(Orientation_Type orientation)
{
	return orientation == Orientation_Above;
}

static void FreeMemory(char *data)
{
	if (data)
	{
		Flow_MemFree((void **)&data);
	}
}

static bool CreateTagString(char **tag, char *tmpTag)
{
	bool success = false;

	if (*tag)
	{
		*tag = (char *)Flow_MemRealloc(*tag, strlen(*tag) + strlen(tmpTag) + 1);
		if (*tag)
		{
			strcat(*tag, tmpTag);
			success = true;
		}
	}
	else
	{
		*tag = (char *)Flow_MemAlloc(strlen(tmpTag) + 1);
		if (*tag)
		{
			strcpy(*tag, tmpTag);
			success = true;
		}
	}
	return success;
}

bool ConstructSettingsCommandForSensor(const Controller *me, char **data)
{
	unsigned int msgSize = 0;
	unsigned int i;
	char *readIntervalTag = NULL;
	char *readDeltaTag = NULL;
	char tmpTag[TAG_SIZE];
	char heartBeat[INT_STR_SIZE];
	bool success = true;
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<command>"
								"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
								"<info>UPDATE_SETTINGS</info>"
								"<settings>"
									"<HeartBeat>%s</HeartBeat>"
									"%s"
									"%s"
								"</settings>"
							"</command>";

	for (i = 0; (i < NUM_SENSORS) && success; ++i)
	{
		sprintf(tmpTag, me->sensors[i].readIntervalTagString, me->sensors[i].readInterval);
		success = CreateTagString(&readIntervalTag, tmpTag);

		if (success)
		{
			sprintf(tmpTag, me->sensors[i].readDeltaTagString, me->sensors[i].readDelta);
			success = CreateTagString(&readDeltaTag, tmpTag);
		}
	}

	if (success)
	{
		sprintf(heartBeat, "%u", me->sensorConfig.heartBeat);

		msgSize = strlen(msgXML)
					+ strlen(readIntervalTag)
					+ strlen(readDeltaTag) + 1;

		*data = (char *)Flow_MemAlloc(msgSize);

		if (*data)
		{
			time_t time;
			Flow_GetTime(&time);
			struct tm timeNow;
			gmtime_r(&time, &timeNow);

			snprintf(*data, msgSize, msgXML,
							timeNow.tm_year + 1900,
							timeNow.tm_mon + 1,
							timeNow.tm_mday,
							timeNow.tm_hour,
							timeNow.tm_min,
							timeNow.tm_sec,
							heartBeat,
							readIntervalTag,
							readDeltaTag);
		}
		else
		{
			success = false;
		}
	}

	FreeMemory(readDeltaTag);
	FreeMemory(readIntervalTag);
	return success;
}

bool ConstructSettingsCommandForActuator(const ActuatorConfig config, char **data)
{
	unsigned int msgSize = 0;
	char heartBeat[INT_STR_SIZE];
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<command>"
								"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
								"<info>UPDATE_SETTINGS</info>"
								"<settings>"
									"<HeartBeat>%s</HeartBeat>"
								"</settings>"
							"</command>";

	sprintf(heartBeat, "%u", config.heartBeat);

	msgSize = strlen(msgXML) + strlen(heartBeat) + 1;

	*data = (char *)Flow_MemAlloc(msgSize);

	if (*data)
	{
		time_t time;
		Flow_GetTime(&time);
		struct tm timeNow;
		gmtime_r(&time, &timeNow);

		snprintf(*data, msgSize, msgXML,
						timeNow.tm_year + 1900,
						timeNow.tm_mon + 1,
						timeNow.tm_mday,
						timeNow.tm_hour,
						timeNow.tm_min,
						timeNow.tm_sec,
						heartBeat);
		return true;
	}
	return false;
}

bool ConstructResponseForUser(const char *response, char **data)
{
	unsigned int msgSize = 0;
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<response>"
								"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
								"<info>%s</info>"
							"</response>";

	msgSize = strlen(msgXML) + strlen(response) + 1;

	*data = (char *)Flow_MemAlloc(msgSize);

	if (*data)
	{
		time_t time;
		Flow_GetTime(&time);
		struct tm timeNow;
		gmtime_r(&time, &timeNow);

		snprintf(*data, msgSize, msgXML,
						timeNow.tm_year + 1900,
						timeNow.tm_mon + 1,
						timeNow.tm_mday,
						timeNow.tm_hour,
						timeNow.tm_min,
						timeNow.tm_sec,
						response);
		return true;
	}
	return false;
}

bool ConstructPingResponseForUser(const char *response, char **data)
{
	unsigned int msgSize = 0;
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<response>"
								"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
								"<info>PING</info>"
								"<app_time>%s</app_time>"
							"</response>";

	msgSize = strlen(msgXML) + strlen(response) + 1;

	*data = (char *)Flow_MemAlloc(msgSize);

	if (*data)
	{
		time_t time;
		Flow_GetTime(&time);
		struct tm timeNow;
		gmtime_r(&time, &timeNow);

		snprintf(*data, msgSize, msgXML,
						timeNow.tm_year + 1900,
						timeNow.tm_mon + 1,
						timeNow.tm_mday,
						timeNow.tm_hour,
						timeNow.tm_min,
						timeNow.tm_sec,
						response);
		return true;
	}
	return false;
}
bool ConstructRelayCommandForActuator(const char *status, char **data)
{
	unsigned int msgSize = 0;
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<command>"
								"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
								"<info>%s</info>"
							"</command>";

	msgSize = strlen(msgXML) + strlen(status) + 1;

	*data = (char *)Flow_MemAlloc(msgSize);

	if (*data)
	{
		time_t time;
		Flow_GetTime(&time);
		struct tm timeNow;
		gmtime_r(&time, &timeNow);

		snprintf(*data, msgSize, msgXML,
						timeNow.tm_year + 1900,
						timeNow.tm_mon + 1,
						timeNow.tm_mday,
						timeNow.tm_hour,
						timeNow.tm_min,
						timeNow.tm_sec,
						status);
		return true;
	}
	return false;
}

bool ConstructDeviceStatusMsgForUser(const Controller *me, char **data)
{
	unsigned int msgSize = 0;
	const char *sensorStatus = me->sensorConfig.isAlive?ALIVE_STR:DEAD_STR;
	const char *actuatorStatus = me->actuatorConfig.isAlive?ALIVE_STR:DEAD_STR;
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<event>"
								"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
								"<type>DeviceStatus</type>"
								"<info>"
									"<Sensor>%s</Sensor>"
									"<Actuator>%s</Actuator>"
								"</info>"
							"</event>";

	msgSize = strlen(msgXML) + strlen(sensorStatus) + strlen(actuatorStatus) + 1;

	*data = (char *)Flow_MemAlloc(msgSize);

	if (*data)
	{
		time_t time;
		Flow_GetTime(&time);
		struct tm timeNow;
		gmtime_r(&time, &timeNow);

		snprintf(*data, msgSize, msgXML,
						timeNow.tm_year + 1900,
						timeNow.tm_mon + 1,
						timeNow.tm_mday,
						timeNow.tm_hour,
						timeNow.tm_min,
						timeNow.tm_sec,
						sensorStatus,
						actuatorStatus);
		return true;
	}
	return false;
}

bool ConstructSensorStatusMsgForUser(const Controller *me, char **data)
{
	unsigned int msgSize = 0;
	unsigned int i;
	char *sensorTag = NULL;
	char tmpTag[TAG_SIZE];
	bool success = true;
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<event>"
								"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
								"<type>Measurement</type>"
								"<info>%s</info>"
							"</event>";

	for (i = 0; (i < NUM_SENSORS) && success; ++i)
	{
		sprintf(tmpTag, me->sensors[i].sensorTagString, me->sensors[i].value);
		success = CreateTagString(&sensorTag, tmpTag);
	}

	if (success)
	{
		msgSize = strlen(msgXML) + strlen(sensorTag) + 1;

		*data = (char *)Flow_MemAlloc(msgSize);

		if (*data)
		{
			time_t time;
			Flow_GetTime(&time);
			struct tm timeNow;
			gmtime_r(&time, &timeNow);

			snprintf(*data, msgSize, msgXML,
							timeNow.tm_year + 1900,
							timeNow.tm_mon + 1,
							timeNow.tm_mday,
							timeNow.tm_hour,
							timeNow.tm_min,
							timeNow.tm_sec,
							sensorTag);
		}
		else
		{
			success = false;
		}
	}

	FreeMemory(sensorTag);
	return success;
}

bool ConstructActuatorStatusMsgForUser(const Controller *me, char **data)
{
	unsigned int msgSize = 0;
	unsigned int i;
	char *relayTag = NULL;
	char tmpTag[TAG_SIZE];
	bool success = true;
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<event>"
								"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
								"<type>RelayStatus</type>"
								"<info>%s</info>"
							"</event>";

	for (i = 0; (i < NUM_RELAYS) && success; ++i)
	{
		sprintf(tmpTag, me->relays[i].relayTagString,
						isRelayAuto(me->relays[i].mode)?AUTO_STR:MANUAL_STR,
						isRelayOn(me->relays[i].status)?ON_STR:OFF_STR);
		success = CreateTagString(&relayTag, tmpTag);
	}

	if (success)
	{
		msgSize = strlen(msgXML) + strlen(relayTag) + 1;

		*data = (char *)Flow_MemAlloc(msgSize);

		if (*data)
		{
			time_t time;
			Flow_GetTime(&time);
			struct tm timeNow;
			gmtime_r(&time, &timeNow);

			snprintf(*data, msgSize, msgXML,
							timeNow.tm_year + 1900,
							timeNow.tm_mon + 1,
							timeNow.tm_mday,
							timeNow.tm_hour,
							timeNow.tm_min,
							timeNow.tm_sec,
							relayTag);
		}
		else
		{
			success = false;
		}
	}

	FreeMemory(relayTag);
	return success;
}

bool ConstructSetting(const Controller *me, char **data)
{
	unsigned int msgSize = 0;
	unsigned int i;
	char *thresholdTag = NULL;
	char *orientationTag = NULL;
	char *readIntervalTag = NULL;
	char *readDeltaTag = NULL;
	char tmpTag[TAG_SIZE];
	char controllerHeartBeat[INT_STR_SIZE];
	char sensorHeartBeat[INT_STR_SIZE];
	char actuatorHeartBeat[INT_STR_SIZE];
	bool success = true;
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<ControllerConfig>"
								"<version>1.0</version>"
								"%s"
								"%s"
								"<HeartBeat>%s</HeartBeat>"
								"<SensorConfig>"
									"<HeartBeat>%s</HeartBeat>"
									"%s"
									"%s"
								"</SensorConfig>"
								"<ActuatorConfig>"
									"<HeartBeat>%s</HeartBeat>"
								"</ActuatorConfig>"
							"</ControllerConfig>";

	for (i = 0; (i < NUM_SENSORS) && success; ++i)
	{
		sprintf(tmpTag, me->sensors[i].thresholdTagString, me->sensors[i].threshold);
		success = CreateTagString(&thresholdTag, tmpTag);

		if (success)
		{
			char *orientation;

			orientation = isOrientationAbove(me->sensors[i].orientation)?ABOVE_STR:BELOW_STR;

			sprintf(tmpTag, me->sensors[i].orientationTagString, orientation);
			success = CreateTagString(&orientationTag, tmpTag);

			if (success)
			{
				sprintf(tmpTag, me->sensors[i].readIntervalTagString, me->sensors[i].readInterval);
				success = CreateTagString(&readIntervalTag, tmpTag);

				if (success)
				{
					sprintf(tmpTag, me->sensors[i].readDeltaTagString, me->sensors[i].readDelta);
					success = CreateTagString(&readDeltaTag, tmpTag);
				}
			}
		}
	}

	if (success)
	{
		sprintf(controllerHeartBeat, "%u", me->config.heartBeat);
		sprintf(sensorHeartBeat, "%u", me->sensorConfig.heartBeat);
		sprintf(actuatorHeartBeat, "%u", me->actuatorConfig.heartBeat);

		msgSize = strlen(msgXML)
					+ strlen(thresholdTag)
					+ strlen(orientationTag)
					+ strlen(readIntervalTag)
					+ strlen(readDeltaTag)
					+ strlen(controllerHeartBeat)
					+ strlen(sensorHeartBeat)
					+ strlen(actuatorHeartBeat) + 1;

		*data = (char *)Flow_MemAlloc(msgSize);

		if (*data)
		{
			snprintf(*data, msgSize, msgXML,
						thresholdTag,
						orientationTag,
						controllerHeartBeat,
						sensorHeartBeat,
						readIntervalTag,
						readDeltaTag,
						actuatorHeartBeat);
		}
		else
		{
			success = false;
		}
	}

	FreeMemory(readDeltaTag);
	FreeMemory(readIntervalTag);
	FreeMemory(orientationTag);
	FreeMemory(thresholdTag);
	return success;
}

bool ConstructHeartBeatMsgForUser(const Controller *me, char **data)
{
	unsigned int i;
	unsigned int msgSize = 0;
	char *sensorTag = NULL, *relayTag = NULL;
	char tmpTag[TAG_SIZE];
	bool success = true;
	const char *sensorStatus = me->sensorConfig.isAlive?ALIVE_STR:DEAD_STR;
	const char *actuatorStatus = me->actuatorConfig.isAlive?ALIVE_STR:DEAD_STR;
	char msgXML[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
						"<event>"
							"<time type=\"datetime\">%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
							"<type>HeartBeat</type>"
							"<info>"
								"%s"
								"%s"
								"<Sensor>%s</Sensor>"
								"<Actuator>%s</Actuator>"
							"</info>"
						"</event>";


	for (i = 0; (i < NUM_SENSORS) && success; ++i)
	{
		sprintf(tmpTag, me->sensors[i].sensorTagString, me->sensors[i].value);
		success = CreateTagString(&sensorTag, tmpTag);
	}

	for (i = 0; (i < NUM_RELAYS) && success; ++i)
	{
		sprintf(tmpTag, me->relays[i].relayTagString,
						isRelayAuto(me->relays[i].mode)?AUTO_STR:MANUAL_STR,
						isRelayOn(me->relays[i].status)?ON_STR:OFF_STR);
		success = CreateTagString(&relayTag, tmpTag);
	}


	if (success)
	{
		msgSize = strlen(msgXML) + strlen(sensorTag) + strlen(relayTag) + 1;

		*data = (char *)Flow_MemAlloc(msgSize);

		if (*data)
		{
			time_t time;
			Flow_GetTime(&time);
			struct tm timeNow;
			gmtime_r(&time, &timeNow);

			snprintf(*data, msgSize, msgXML,
							timeNow.tm_year + 1900,
							timeNow.tm_mon + 1,
							timeNow.tm_mday,
							timeNow.tm_hour,
							timeNow.tm_min,
							timeNow.tm_sec,
							sensorTag,
							relayTag,
							sensorStatus,
							actuatorStatus);
		}
		else
		{
			success = false;
		}
	}

	FreeMemory(relayTag);
	FreeMemory(sensorTag);
	return success;
}

