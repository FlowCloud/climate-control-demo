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
