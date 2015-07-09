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


