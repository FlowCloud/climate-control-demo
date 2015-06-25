/*******************************************************************************
 System Interrupt Source File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2012 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
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

#include <xc.h>
#include <sys/attribs.h>

extern void APPCORE_InterruptHandler_MRF24W_Ext4(void);
extern void APPCORE_InterruptHandler_ETHMAC(void);
extern void APPCORE_InterruptHandler_TMR2(void);
extern void APPCORE_InterruptHandler_NVM(void);

void __ISR(_EXTERNAL_4_VECTOR, ipl3srs) _InterruptHandler_MRF24W_Ext4(void)
{
     APPCORE_InterruptHandler_MRF24W_Ext4();
}

void __ISR(_ETHERNET_VECTOR, ipl5srs) _InterruptHandler_ETHMAC(void)
{
    APPCORE_InterruptHandler_ETHMAC();
}

void __ISR(_TIMER_2_VECTOR, ipl4) _InterruptHandler_TMR2(void)
{
    APPCORE_InterruptHandler_TMR2();
}

void __ISR(_FLASH_CONTROL_VECTOR, ipl4srs) _InterruptHandler_NVM(void)
{
    APPCORE_InterruptHandler_NVM();
}

/*******************************************************************************
 End of File
 */


