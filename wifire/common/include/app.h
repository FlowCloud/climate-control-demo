/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
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
//DOM-IGNORE-END

#ifndef _APP_H
#define _APP_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"
#ifdef TCPIP_IF_MRF24W
#include "driver/wifi/mrf24w/drv_wifi.h"
#include "driver/wifi/mrf24w/src/drv_wifi_easy_config.h"
#endif
#include "activitylog.h"

#include "flow/core/flow_threading.h"
#include "flow_task_priority.h"

#define	MAC_ADDRESS_LENGTH			(12)
#define SERIAL_NUMBER_BUFFER_LEN		(17)
#define CLIENTID_CHAR_LENGTH		(5)

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behaviour of the application at various times.
*/

typedef enum
{
   /* The app mounts the disk */
    APP_MOUNT_DISK = 0,

    /* In this state, the application waits for the initialization of the TCP/IP stack
     * to complete. */
    APP_TCPIP_WAIT_INIT,

    /* In this state the application starts the Soft AP*/
    APP_WIFI_SOFTAP,

    /* In this state, the application opens the driver. */
    APP_TCPIP_TRANSACT,

    /* The application does nothing in the idle state. */
    APP_STATE_IDLE,

    //
    APP_USERIO_LED_DEASSERTED,

    APP_USERIO_LED_ASSERTED,

    APP_TCPIP_ERROR,

} APP_STATES;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{

    SYS_FS_HANDLE           fileHandle;

    /* Application's current state */
    APP_STATES              state;

} APP_DATA;



// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */
void APP_Tasks ( void );


/*******************************************************************************
  Function:
    void SYS_StartTasks ( void )

  Summary:
    Function to start all RTOS tasks (including system-task).

  Description:
    This is the main RTOS function that starts all concurrent tasks. The function
    creates the required tasks and invokes the scheduler. The scheduler then manages
    the cooperative multi-tasking between different RTOS tasks.
    This function call is block whilst the scheduler continues to run.

  Remarks:
    This routine must be called after SYS_Initialize() routine.
*/
void SYS_StartTasks(void);

// SoftAP SSID
bool APP_GetSoftAPSSID(uint8_t *buff, uint32_t buffSize);

// Check if App is in configuration (SoftAP) or interactive mode
bool APP_IsRunningInConfigurationMode(void);

#endif /* _APP_H */
/*******************************************************************************
 End of File
 */
