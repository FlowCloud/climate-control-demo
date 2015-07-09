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
/*! \file app.c
 *  \brief WiFire application. Starts in SoftAP for local configuration or interactive mode
 */

// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "device_serial.h"
#include "time.h"
#include "user.h"
// Flow CLI command handlers
#include "command_handlers.h"

// User interface
#include "ui_control.h"

// configuration-store  APIs
#include "config_store.h"

// Flow library APIs
#include "flow/flowcore.h"
#include "flow/flowmessaging.h"
#include "flow_task_priority.h"
#include "flow/core/flow_threading.h"
#include "activitylog_errors.h"
#include "flow/flow_console.h"

#include "app_core.h"
#include "activitylog.h"

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Function Prototypes
// *****************************************************************************
// *****************************************************************************
void SYS_AppTask(FlowThread thread, void *taskParameters);

extern bool CorrectCommandLength(const char* userCommandText, const char* commandText);

//TODO: Remove this dummy implementation once this dependency from libappbase is fixed.
void APP_SetDeviceIsOnline(bool deviceOnline)
{
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine
// *****************************************************************************
// *****************************************************************************
void APP_Tasks_InteractiveMode( void )
{

	// Debounce switch presses for rebooting to configuration mode
	if (CheckForConfigurationModeRebootButtonPress())
	{
		CommandHandlers_ResetHandler(true);
	}

}
/*********************************************************************
 * Function:        void SYS_AppTask(FlowThread thread, void *taskParameters)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Manage access to the on-chip flash memory
 *
 * Side Effects:    None
 *
 * Overview:        This function implements the on-chip flash behaviour.
 *
 * Note:            None
 ********************************************************************/
void SYS_AppTask(FlowThread thread, void *taskParameters)
{
		/* Create device */
		char deviceSerialNumber[SERIAL_NUMBER_BUFFER_LEN];
		DeviceSerial_GetCpuSerialNumberHexString (deviceSerialNumber, SERIAL_NUMBER_BUFFER_LEN);
		FlowMemoryManager memoryManager = FlowMemoryManager_New();
		ConfigStore_SetDeviceType((char*)ClimateControl_GetDeviceType());
		FlowConsole_Printf("\n\rDevice Type '%s'\n\r\n\r", (char*)ConfigStore_GetDeviceType());
		FlowConsole_Printf("\n\rs/w version Type '%s'\n\r\n\r", (char*)ClimateControl_GetExternalSoftwareVersion());
		if (FlowClient_LoginAsDevice((char*)ConfigStore_GetDeviceType(), (char*)ConfigStore_GetMacAddress(), deviceSerialNumber, NULL, (char*)ClimateControl_GetExternalSoftwareVersion(), (char*)ConfigStore_GetDeviceName(), (char*)ConfigStore_GetRegistrationKey()))
		{
			Flow_ActivityLogWrite(FlowActivityLogLevel_Information, FlowActivityLogCategory_Startup, ActivityLogErrorCode_none, "Device registered successfully");
		}
		else
		{
			FlowConsole_Puts("Device registration failed...\n\r");
			FlowErrorType errorType = Flow_GetLastError();
			if (errorType >= FlowError_BadRequest_Min && errorType <= FlowError_BadRequest_Unknown)
				errorType = FlowError_InvalidArgument;
			if (errorType == FlowError_InvalidArgument || errorType == FlowError_MethodUnavailable || errorType == FlowError_Unauthorised)
			{
					FlowNVS_Set("core.remembermetoken", NULL, 0);
					APP_SoftwareReset(true);
			}
			while (1)
				vTaskDelay(1000/portTICK_RATE_MS);
		}

		FlowMemoryManager_Free(&memoryManager);
		UIControl_SetUIState(AppUIState_InteractiveConnectedToFlow);
		UserSetup();

		// Connected to Flow
		Flow_ActivityLogSystemMode(FlowActivityLogCategory_SystemRuntime);

		FlowConsole_Puts("\n\r\n\r----------------------------------------------");
		FlowConsole_Puts("\n\rFlow climate control app booting completed. Running...\n\r");
		FlowConsole_Puts("\n\r\n\r\n\r");
		CLICommand_Init();
		while (1)
		{
			/* Call the application's tasks routine */
			APP_Tasks_InteractiveMode();

			vTaskDelay( 100 / portTICK_RATE_MS);
		}
}
/*******************************************************************************
 End of File
 */
