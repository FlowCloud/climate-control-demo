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


#include "app.h"
#include "command_handlers.h"
#include "system_command.h"
#include <string.h>
#include "flow/flow_console.h"

#define NUMBER_OF_APP_COMMANDS	(2)

//command table
const SYS_CMD_DESCRIPTOR  ClimateControlAppCmdTbl[]=
{
	{"wifire_details",	 CommandHandlers_CLI_ShowWiFireDetails, ": Display board information"},
	{"app_versions",	 CommandHandlers_CLI_ShowSoftwareVersions,   ": Display external and internal version of the climate control app"},
};

int CommandShow(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
	int result = 0;
	unsigned int i = 0;
	bool success = false;
	if (pCmdIO)
	{
		const void* cmdIoParam = pCmdIO->cmdIoParam;

		if (argc == 2)
		{
			if (argv[1])
			{
				for (i = 0; i < NUMBER_OF_APP_COMMANDS; ++i)
				{
					if (strcmp(argv[1], ClimateControlAppCmdTbl[i].cmdStr) == 0)
					{
						ClimateControlAppCmdTbl[i].cmdFnc(pCmdIO, argc, argv);
						success = true;
					}
				}

			}
		}
		else
		{
			FlowConsole_Puts("Flow climate control app supported:\n\r\t\t\t");
			for (i = 0; i < NUMBER_OF_APP_COMMANDS; ++i)
			{
				//check if it is the last command, if yes don't print comma
				if (i < (NUMBER_OF_APP_COMMANDS - 1))
				{
					(*pCmdIO->pCmdApi->print)(cmdIoParam, "%s,", ClimateControlAppCmdTbl[i].cmdStr);
				}
				else
				{
					(*pCmdIO->pCmdApi->print)(cmdIoParam, "%s"LINE_TERM, ClimateControlAppCmdTbl[i].cmdStr);
				}
			}
			FlowConsole_Puts(LINE_TERM);
		}
	}
	if (!success)
		result = -1;
	return result;
}
void CLICommand_Init()
{
	SYS_CMD_ADDGRP(ClimateControlAppCmdTbl, sizeof(ClimateControlAppCmdTbl)/sizeof(*ClimateControlAppCmdTbl), "climate_control_app", ": Climate Control App Commands");
}
