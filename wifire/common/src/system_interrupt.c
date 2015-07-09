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


