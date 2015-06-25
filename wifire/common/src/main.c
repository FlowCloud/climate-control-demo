/*******************************************************************************
  FlowCloud Climate Control App Demo

  File Name:
    main.c

  Summary:
    FlowCloud Climate Control App

  Description:
    This file contains the Flow Climate Control App main function.
 *******************************************************************************/


// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

//Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


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

