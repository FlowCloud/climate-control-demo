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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "flow/core/core.h"
#include "flow/messaging/flow_messaging.h"
#include "version.h"

typedef struct {
	const char* cmdName;
	void (*func)(void);	//Command handler
	const char* doc;
}ConsoleCmdTable;

#define MAX_SIZE (100)
#define ARRAY_SIZE(arr)	(sizeof(arr)/sizeof(arr[0]))


static void GetDeviceRegKey(void);
static void GetVersions(void);
static void AvailableCommands(void);
static void ExitConsole(void);

static ConsoleCmdTable cmd_table[] =
	{
		{ "show devreg_key", GetDeviceRegKey, "Get device registration key"},
		{ "show versions", GetVersions, "Show application and flow libraries versions"},
		{ "help", AvailableCommands, "Show available commands"},
		{ "exit", ExitConsole, "Exits the interpreter"},
	};

static bool GetValueForKey(const char *key, char *value)
{
	char temp[MAX_SIZE];

	FILE *configFile = fopen("flow_controller.cnf", "r");

	if (configFile)
	{
		while (fscanf(configFile, "%[^=]=%s\n", temp, value) == 2)
		{
			if (strcmp(key, temp) == 0)
			{
				fclose(configFile);
				return true;
			}
		}
		fclose(configFile);
	}
	else
	{
		printf("Configuration file not found\n");
	}
	return false;
}

static void GetDeviceRegKey(void)
{
	char devRegKey[MAX_SIZE];

	if (GetValueForKey("Devreg_Key", devRegKey))
	{
		printf("Device Registration Key: %s\n",devRegKey);
	}
	else
	{
		printf("Device Registration Key NOT found\n");
	}
}

static void GetVersions(void)
{
	printf("\tlibflowcore:\t(v%s)\n", FlowCore_GetVersion());
	printf("\tlibflowmessaging:\t(v%s)\n", FlowMessaging_GetVersion());
	printf("\tapplication(public):\t(v%s)\n",DEVICE_SOFTWARE_VERSION);
	printf("\tapplication(internal):\t(v%s)\n",INTERNAL_SOFTWARE_VERSION);
}

static void AvailableCommands(void)
{
	int i=ARRAY_SIZE(cmd_table);

	while (i--)
	{
		ConsoleCmdTable cmd = cmd_table[i];
		printf("%20s\t- %s\n", cmd.cmdName, cmd.doc);
	}
}

static void ExitConsole(void)
{
	printf("Exiting Interactive Mode\n");
	exit(0);
}

static void parse(char *cmd)
{
	const char* tok = strtok(cmd, "\n(,);");
	if (!tok)
	{
		return;
	}

	int i=ARRAY_SIZE(cmd_table);
	while (i--)
	{
		ConsoleCmdTable cur = cmd_table[i];
		if (!strcmp(tok, cur.cmdName))
		{
			cur.func();
			return;
		}
	}

	puts("Command Not Found");
}

void StartConsole(void)
{
	printf("Entering Interactive Mode\n");
	while (1)
	{
		char cmd[MAX_SIZE] = {0};
		unsigned i = 0;

		printf("%c ",'>');
		fflush(stdout);

		while (1)
		{
			char c;
			c = fgetc(stdin);

			if (c != EOF)
			{
				if ((c != '\n') && (c != '\r') && (i < MAX_SIZE - 1))
				{
					cmd[i] = c;
					i++;
				}
				else
				{
					break;
				}
			}
		}
		parse(cmd);
	}
}

