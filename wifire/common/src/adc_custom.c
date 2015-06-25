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
	// Inspired by mgrundler's post on Microchip forum http://www.microchip.com/forums/m783963.aspx

 *******************************************************************************************************/

#include "adc_custom.h"
#include "app.h"

static int convert_AINtoAN(int port);

void adc12_StartSampling(void);

int adc12_SampleReady(int port);

void adc12_Wait(int port);



/*
 *  adc12_GetSample(port)
 *
 *  Arguments:		port (AN0 - AN49)
 *
 *  Returnvalues:	sampledata
 *
 *  Function:		Returns the Sampled Data for the given Port (attention! make sure that the conversation is finished before getting the Data!)
 *
 */
unsigned int adc12_GetSample(int port);

void adc12_Init()
{
	AD1CAL1 = DEVADC1;					// Writing Calibration-Data to ADC-Registers
	AD1CAL2 = DEVADC2;
	AD1CAL3 = DEVADC3;
	AD1CAL4 = DEVADC4;
	AD1CAL5 = DEVADC5;

	AD1CON1 = 0;						// First Clear AD1CON1-Register

	AD1CON1bits.STRGSRC = 1;			// Scan Trigger Src = Global Software Trigger (GSWTRG)

	AD1CON2 = 0;						// Clear AD1CON2-Register
	AD1CON2bits.ADCSEL = 1;				// ADC Clock-Src = SYSCLK (200 MHz)
	AD1CON2bits.ADCDIV = 4;				// Clock Divider 4 means (8*TQ) = 25 MHz
	AD1CON2bits.SAMC = 9;				// S&H-Sample Time = 9TAD

	AD1CON3 = 0;						// Clear AD1CON3-Register

	AD1IMOD = 0;						// All SampleAndHold-Units set to Standard-values
										// Single-ended Inputs with unsigned integer-Format, Standard-Inputs

	AD1GIRQEN1 = 0;						// No Interrupts used
	AD1GIRQEN2 = 0;						// No Interrupts

	AD1CSS1 = 0;						// First Clear all Channel-Scan-Select-Bits (Register 1)
	AD1CSS2 = 0;						// Clear all CSS-Bits of Register 2


	AD1CMPCON1 = 0;						// No Comperator-Functions
	AD1CMPCON2 = 0;
	AD1CMPCON3 = 0;
	AD1CMPCON4 = 0;
	AD1CMPCON5 = 0;
	AD1CMPCON6 = 0;

	AD1FLTR1 = 0;						// No Filter / Oversampling
	AD1FLTR2 = 0;
	AD1FLTR3 = 0;
	AD1FLTR4 = 0;
	AD1FLTR5 = 0;
	AD1FLTR6 = 0;


	AD1TRG1 = 0;						// No Triggers
	AD1TRG2 = 0;						// No Triggers
	AD1TRG3 = 0;						// No Triggers
}


int convert_AINtoAN(int port)
{
	int ret = port;
	switch(port)
	{
		case 0:
			ret = AN45;
			break;
		case 1:
			ret = AN49;
			break;
		case 2:
			ret = AN21;
			break;
		case 3:
			ret = AN10;
			break;
		case 4:
			ret = AN13;
			break;
		case 5:
			ret = AN12;
			break;
		case 6:
			ret = AN3;
			break;
		case 7:
			ret = AN2;
			break;
		case 8:
			ret = AN4;
			break;
		case 9:
			ret = AN1;
			break;
		case 10:
			ret = AN11;
			break;
		case 11:
			ret = AN0;
		case 12:
			ret = AN8;
		default:
			break;
	}
	return ret;
}

void adc12_ConfigPort(int port)
{
	port = convert_AINtoAN(port);
	switch (port)
	{
		case AN0:
			TRISBSET  = 1<<0;
			ANSELBSET = 1<<0;
			AD1CSS1bits.CSS0 = 1;
			AD1TRG1bits.TRGSRC0 = 3;
			break;
		case AN1:
			TRISBSET  = 1<<1;
			ANSELBSET = 1<<1;
			AD1CSS1bits.CSS1 = 1;
			AD1TRG1bits.TRGSRC1 = 3;
			break;
		case AN2:
			TRISBSET  = 1<<2;
			ANSELBSET = 1<<2;
			AD1CSS1bits.CSS2 = 1;
			AD1TRG1bits.TRGSRC2 = 3;
			break;
		case AN3:
			TRISBSET  = 1<<3;
			ANSELBSET = 1<<3;
			AD1CSS1bits.CSS3 = 1;
			AD1TRG1bits.TRGSRC3 = 3;
			break;
		case AN4:
			TRISBSET  = 1<<4;
			ANSELBSET = 1<<4;
			AD1CSS1bits.CSS4 = 1;
			AD1TRG2bits.TRGSRC4 = 3;
			break;
		case AN5:
			TRISBSET  = 1<<10;
			ANSELBSET = 1<<10;
			AD1CSS1bits.CSS5 = 1;
			AD1TRG2bits.TRGSRC5 = 3;
			break;
		case AN6:
			TRISBSET  = 1<<11;
			ANSELBSET = 1<<11;
			AD1CSS1bits.CSS6 = 1;
			AD1TRG2bits.TRGSRC6 = 3;
			break;
		case AN7:
			TRISBSET  = 1<<12;
			ANSELBSET = 1<<12;
			AD1CSS1bits.CSS7 = 1;
			AD1TRG2bits.TRGSRC7 = 3;
			break;
		case AN8:
			TRISBSET  = 1<<13;
			ANSELBSET = 1<<13;
			AD1CSS1bits.CSS8 = 1;
			AD1TRG3bits.TRGSRC8 = 3;
			break;
		case AN9:
			TRISBSET  = 1<<14;
			ANSELBSET = 1<<14;
			AD1CSS1bits.CSS9 = 1;
			AD1TRG3bits.TRGSRC9 = 3;
			break;
		case AN10:
			TRISBSET  = 1<<15;
			ANSELBSET = 1<<15;
			AD1CSS1bits.CSS10 = 1;
			AD1TRG3bits.TRGSRC10 = 3;
			break;
		case AN11:
			TRISGSET  = 1<<9;
			ANSELGSET = 1<<9;
			AD1CSS1bits.CSS11 = 1;
			AD1TRG3bits.TRGSRC11 = 3;
			break;
		case AN12:
			TRISGSET  = 1<<8;
			ANSELGSET = 1<<8;
			AD1CSS1bits.CSS12 = 1;
			break;
		case AN13:
			TRISGSET  = 1<<7;
			ANSELGSET = 1<<7;
			AD1CSS1bits.CSS13 = 1;
			break;
		case AN14:
			TRISGSET  = 1<<6;
			ANSELGSET = 1<<6;
			AD1CSS1bits.CSS14 = 1;
			break;
		case AN15:
			TRISESET  = 1<<7;
			ANSELESET = 1<<7;
			AD1CSS1bits.CSS15 = 1;
			break;
		case AN16:
			TRISESET  = 1<<6;
			ANSELESET = 1<<6;
			AD1CSS1bits.CSS16 = 1;
			break;
		case AN17:
			TRISESET  = 1<<5;
			ANSELESET = 1<<5;
			AD1CSS1bits.CSS17 = 1;
			break;
		case AN18:
			TRISESET  = 1<<4;
			ANSELESET = 1<<4;
			AD1CSS1bits.CSS18 = 1;
			break;
		case AN19:
			TRISCSET  = 1<<4;
			ANSELCSET = 1<<4;
			AD1CSS1bits.CSS19 = 1;
			break;
		case AN20:
			TRISCSET  = 1<<3;
			ANSELCSET = 1<<3;
			AD1CSS1bits.CSS20 = 1;
			break;
		case AN21:
			TRISCSET  = 1<<2;
			ANSELCSET = 1<<2;
			AD1CSS1bits.CSS21 = 1;
			break;
		case AN22:
			TRISCSET  = 1<<1;
			ANSELCSET = 1<<1;
			AD1CSS1bits.CSS22 = 1;
			break;
		case AN23:
			TRISGSET  = 1<<15;
			ANSELGSET = 1<<15;
			AD1CSS1bits.CSS23 = 1;
			break;
		case AN24:
			TRISASET  = 1<<0;
			ANSELASET = 1<<0;
			AD1CSS1bits.CSS24 = 1;
			break;
		case AN25:
			TRISESET  = 1<<8;
			ANSELESET = 1<<8;
			AD1CSS1bits.CSS25 = 1;
			break;
		case AN26:
			TRISESET  = 1<<9;
			ANSELESET = 1<<9;
			AD1CSS1bits.CSS26 = 1;
			break;
		case AN27:
			TRISASET  = 1<<9;
			ANSELASET = 1<<9;
			AD1CSS1bits.CSS27 = 1;
			break;
		case AN28:
			TRISASET  = 1<<10;
			ANSELASET = 1<<10;
			AD1CSS1bits.CSS28 = 1;
			break;
		case AN29:
			TRISASET  = 1<<1;
			ANSELASET = 1<<1;
			AD1CSS1bits.CSS29 = 1;
			break;
		case AN30:
			TRISFSET  = 1<<13;
			ANSELFSET = 1<<13;
			AD1CSS1bits.CSS30 = 1;
			break;
		case AN31:
			TRISFSET  = 1<<12;
			ANSELFSET = 1<<12;
			AD1CSS1bits.CSS31 = 1;
			break;
		case AN32:
			TRISDSET  = 1<<14;
			ANSELDSET = 1<<14;
			AD1CSS2bits.CSS32 = 1;
			break;
		case AN33:
			TRISDSET  = 1<<15;
			ANSELDSET = 1<<15;
			AD1CSS2bits.CSS33 = 1;
			break;
		case AN34:
			TRISASET  = 1<<5;
			ANSELASET = 1<<5;
			AD1CSS2bits.CSS34 = 1;
			break;
		case AN43:
			AD1CSS2bits.CSS43 = 1;
			break;
		case AN44:
			AD1CSS2bits.CSS44 = 1;
			break;
		case AN45:
			TRISBSET  = 1<<5;
			ANSELBSET = 1<<5;
			AD1IMODbits.SH0ALT = 1;
			break;
		case AN46:
			TRISBSET  = 1<<5;
			ANSELBSET = 1<<5;
			AD1IMODbits.SH1ALT = 1;
			break;
		case AN47:
			TRISBSET  = 1<<5;
			ANSELBSET = 1<<5;
			AD1IMODbits.SH2ALT = 1;
			break;
		case AN48:
			TRISBSET  = 1<<5;
			ANSELBSET = 1<<5;
			AD1IMODbits.SH3ALT = 1;
			break;
		case AN49:
			TRISBSET  = 1<<5;
			ANSELBSET = 1<<5;
			AD1IMODbits.SH4ALT = 1;
			break;
		default:
			asm("nop");
	};
};


//
void adc12_Enable(void)
{
	AD1CON1bits.ADCEN = 1;				// Enable ADC
	AD1IMODbits.SH0MOD = 0b10;			//Make the input differential for more accurate read range

	while (AD1CON2bits.ADCRDY == 0);	// Wait for end of Calibration
};


//
void adc12_StartSampling(void)
{
	AD1CON3bits.GSWTRG = 1;
};


//
int adc12_SampleReady(int port)
{
	if (port <= AN31)
	{
		return (AD1DSTAT1 && (1 << port));
	}
	else
	{
		return (AD1DSTAT2 && (1 << (port-31)));
	};
};


//
void adc12_Wait(int port)
{
	if (port <= AN31)
	{
		while ((AD1DSTAT1 && (1 << port)) == 0);
	}
	else
	{
		while ((AD1DSTAT2 && (1 << (port-31))) == 0);
	};
};


//
unsigned int adc12_GetSample(int port)
{
	volatile unsigned int *dataregisters;

	dataregisters = &AD1DATA0;
	return dataregisters[port];
};


//
unsigned int adc12_Read(int port)
{
	port = convert_AINtoAN(port);
	adc12_StartSampling();
	adc12_Wait(port);
	return adc12_GetSample(port);
};
