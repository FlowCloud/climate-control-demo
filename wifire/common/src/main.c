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
/*******************************************************************************
  FlowCloud Climate Control Demo App

  File Name:
    main.c

  Summary:
    FlowCloud Climate Control Demo App

  Description:
    This file contains the FlowCloud Climate Control Demo App main function.
 *******************************************************************************/




// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <system/debug/sys_debug.h>
#include <tcpip/src/system/system_debug.h>
#include <app_core.h>
#include "app.h"
#include "user.h"

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

extern void SYS_AppTask(FlowThread thread, void *taskParameters);
extern int CommandShow(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
extern bool CommandHandlers_ResetHandler(bool resetToConfigurationMode);

static APP_info info =
{
	.AppTask = SYS_AppTask,
	.CommandShow = CommandShow,
	.AppCLI_ResetHandler = CommandHandlers_ResetHandler
};


int main ( void )
{
	info.appVersion = (char *)ClimateControl_GetExternalSoftwareVersion();
	info.appName = (char *)ClimateControl_GetAppName();
	return APPCORE_init(&info);
}

/*******************************************************************************
 End of File
*/

