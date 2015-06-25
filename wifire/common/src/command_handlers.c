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

/*! \file command_handlers.c
 *  \brief Callback functions for handling CLI commands
 */

#include "command_handlers.h"
#include "app.h"
#include "user.h"
#include "flow/messaging/flow_messaging.h"
#include "string_builder.h"
#include "device_serial.h"
#include "ui_control.h"
#include "config_store.h"
#include "flow/core/flow_memalloc.h"
#include "tcpip/src/system/system_mapping.h"
#include "flow/flow_console.h"

static bool _ResetToConfigurationMode = false;

#ifdef FLOW_PIC32MZ_ETHERNET
	void BSP_VBUSSwitchDisable(void)
	{
		/* Disable the VBUS switch */
		ANSELBbits.ANSB5 = 0;
		TRISBbits.TRISB5 = 0;
		LATBbits.LATB5 = 0;
	}
#endif

bool CommandHandlers_ClearActivityLog(void)
{
	bool result = false;
	if (Flow_ActivityLogErase())
		result = true;
	if(!result)
		SYS_ERROR(SYS_ERROR_ERROR, "command_handlers: Error, Could not clear device Activity Log");
	return result;
}

static void ResetTimeoutTask(FlowTaskID taskID, void *clientArg)
{
	FlowConsole_Puts("Forcing restart...");
	vTaskDelay(200 / portTICK_PERIOD_MS);
	APP_SoftwareReset(_ResetToConfigurationMode);
}

int CommandHandlers_CLI_ShowWiFireDetails(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
	if (pCmdIO)
	{
		const void* cmdIoParam = pCmdIO->cmdIoParam;
		if (ConfigStore_LoggingSettings_Read() && ConfigStore_LoggingSettings_IsValid())
		{
			// Device Name
			char *deviceName = (char *) ConfigStore_GetDeviceName();
			if (deviceName && strlen(deviceName))
				(*pCmdIO->pCmdApi->print)(cmdIoParam, "Device Name:\t\t%s" LINE_TERM, deviceName);
			else
				(*pCmdIO->pCmdApi->msg)(cmdIoParam, "Device Name:\t\tunknown" LINE_TERM);

			// Device Type
			char *deviceType = (char *) ConfigStore_GetDeviceType();
			if (deviceType && strlen(deviceType))
				(*pCmdIO->pCmdApi->print)(cmdIoParam, "Device Type:\t\t%s" LINE_TERM, deviceType);
			else
				(*pCmdIO->pCmdApi->msg)(cmdIoParam, "Device Type:\t\tunknown" LINE_TERM);

			// Device MAC address
			char *mac = (char *) ConfigStore_GetMacAddress();
			if (mac && strlen(mac))
				(*pCmdIO->pCmdApi->print)(cmdIoParam, "MAC:\t\t\t%s" LINE_TERM, mac);
			else
				(*pCmdIO->pCmdApi->msg)(cmdIoParam, "MAC:\t\t\tunknown" LINE_TERM);

			char deviceSerialNumber[SERIAL_NUMBER_BUFFER_LEN];
			DeviceSerial_GetCpuSerialNumberHexString (deviceSerialNumber, SERIAL_NUMBER_BUFFER_LEN);
			(*pCmdIO->pCmdApi->print)(cmdIoParam, "MCU Serial number:\t%s" LINE_TERM, deviceSerialNumber);
			// Microchip DeviceID
			uint32_t devID = DEVID & 0x0FFFFFFF;
			(*pCmdIO->pCmdApi->print)(cmdIoParam, "MCU Device ID:\t\t0x%08X" LINE_TERM, devID);

			// Microchip DeviceID
			uint32_t revision = (DEVID&0xF0000000) >> 28;
			(*pCmdIO->pCmdApi->print)(cmdIoParam, "MCU Device Revision:\tRev %d" LINE_TERM, revision);

			// SoftAP SSID setting
			char *softApSSID = (char *) ConfigStore_GetSoftAPSSID();
			if (softApSSID && strlen(softApSSID))
				(*pCmdIO->pCmdApi->print)(cmdIoParam, "SoftAP SSID:\t\t%s" LINE_TERM, softApSSID);
			else
				(*pCmdIO->pCmdApi->msg)(cmdIoParam, "SoftAP SSID:\t\tnot set" LINE_TERM);

			// SoftAP password setting
			char *softApPassword = (char *) ConfigStore_GetSoftAPPassword();
			if (softApPassword && strlen(softApPassword))
				(*pCmdIO->pCmdApi->print)(cmdIoParam, "SoftAP Password:\t%s" LINE_TERM, softApPassword);
			else
				(*pCmdIO->pCmdApi->msg)(cmdIoParam, "SoftAP Password:\tnot set" LINE_TERM);

			(*pCmdIO->pCmdApi->msg)(cmdIoParam, LINE_TERM LINE_TERM);
		}
		else
		{
			(*pCmdIO->pCmdApi->msg)(cmdIoParam, "Error, device's configuration is invalid. Please reset the configuration using the 'factory_reset' command" LINE_TERM);
		}
	}
	return true;
}

int CommandHandlers_CLI_ShowSoftwareVersions(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
	const void* cmdIoParam = pCmdIO->cmdIoParam;
	//display external version
	const char *version = ClimateControl_GetExternalSoftwareVersion();
	if (version && strlen(version))
	{
		(*pCmdIO->pCmdApi->print)(cmdIoParam, "External version:\t\t%s" LINE_TERM, version);
	}
	else
	{
		(*pCmdIO->pCmdApi->msg)(cmdIoParam, "External version:\t\tunknown" LINE_TERM);
	}
	//display Internal version
	version = ClimateControl_GetInternalSoftwareVersion();
	if (version && strlen(version))
	{
		(*pCmdIO->pCmdApi->print)(cmdIoParam, "Internal version:\t\t%s" LINE_TERM, version);
	}
	else
	{
		(*pCmdIO->pCmdApi->msg)(cmdIoParam, "Internal version:\t\tunknown" LINE_TERM);
	}
	return true;
}

bool CommandHandlers_ResetHandler(bool resetToConfigurationMode)
{
	FlowScheduler_ScheduleTask(ResetTimeoutTask, NULL, 10, false);
	APP_SoftwareReset(resetToConfigurationMode);
	return true;
}
