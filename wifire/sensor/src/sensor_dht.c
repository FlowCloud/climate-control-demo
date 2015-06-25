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
	//Inspired by DHT library under MIT license by Adafruit industries

*****************************************************************************/

#include "sensor_dht.h"
#include "FreeRTOS.h"
#include "task.h"
#include "peripheral/ports/plib_ports.h"
/*
 * All of this calculations are for 200MHZ processor
 * coretickrate = (sys frequency/2) = (200000000/2) =  100000000
 * for 1 sec(10^6 microseconds) number of ticks  = 100000000
 * therefore for 1 microsecond number of ticks  = (100000000)/(1000000) = 100
 */
#define F_CPU  (200000000UL)
#define CORETIMER_TICKS_PER_MICROSECOND		(F_CPU / 2 / 1000000UL)

/* When sensor is ready to communicate with the device,
 * sensor first pulls the pin low for 80 microseconds and then high for 80 microseconds
 * TIMEOUT=2000 will give us 81 microseconds so that we can immediately return if something is wrong.
 */
#define TIMEOUT (F_CPU/100000)	    // timeout is of 81 microseconds.
#define PULL_PIN_LOW_TIMEOUT (F_CPU/450) //timeout is of 18ms
#define PULL_PIN_HIGH_TIMEOUT (F_CPU/222222) //timeout is of 38microseconds.
#define SENSOR_DATA_BUFFER_SIZE (5)                //this is the number of bytes sensor sends to the device in response.
#define SENSOR_DATA_BUFFER_SIZE_IN_BITS		(SENSOR_DATA_BUFFER_SIZE * 8) // number of bits in the data buffer
//if voltage length stays high for more than 40 microsecond then
//sensor has sent 1 else 0
#define VOLTAGE_LENGTH        (40)  //microseconds

// from the DHT11 datasheet
#define DHT11_MIN_TEMPERATURE	(0) // degree celsius
#define DHT11_MAX_TEMPERATURE	(50)// degree celsius
#define DHT11_MIN_HUMIDITY		(20)// %RH
#define DHT11_MAX_HUMIDITY		(90)// %RH

#define HIGH (0x1)
#define LOW  (0x0)
#define DHT_SENSOR_PORT PORT_CHANNEL_B
#define DHT_SENSOR_PIN PORTS_BIT_POS_9

///Got it from timer.h as we are getting lots of build errors if we include timer.h
unsigned int __attribute__((nomips16)) ReadCoreTimer(void);

typedef enum
{
    SENSOR_OK,
    SENSOR_ERROR_TIMEOUT,
}SENSOR_RETURN_TYPE_T;

static SENSOR_RETURN_TYPE_T read(uint8_t *bits);

static float cacheTemperature = 0.0f;

//this is a dummy implementation
void Sensor_DHT_Init(void)
{

}

bool Sensor_DHT_ReadTemperature(float* const temperature)
{
	// return cached value
	*temperature = cacheTemperature;
	return true;
}

bool Sensor_DHT_ReadHumidity(float* const humidity)
{
	return Sensor_DHT_ReadHumidityAndTemperature(humidity, &cacheTemperature);
}

bool Sensor_DHT_ReadHumidityAndTemperature(float* const humidity, float* const temperature)
{
	uint8_t bits[SENSOR_DATA_BUFFER_SIZE]; // buffer to receive data from the sensor.
	SENSOR_RETURN_TYPE_T rv = read(bits);
	if (rv != SENSOR_OK)
	{
		return false;
	}
	// Test checksum
	// bits[1] && bits[3] both 0
	uint8_t sum = bits[0] + bits[2];
	if (bits[4] != sum)
	{
		return false;
	}
	//check if the returned temperature/humidity are out of the specified limits
	if ( bits[2] < DHT11_MIN_TEMPERATURE
		|| bits[2] > DHT11_MAX_TEMPERATURE
		|| bits[0] < DHT11_MIN_HUMIDITY
		|| bits[0] > DHT11_MAX_HUMIDITY)
	{
		return false;
	}
	*humidity = bits[0]; // bits[1] == 0;
	*temperature = bits[2]; // bits[3] == 0;
	return true;
}

/* @brief- communicate with the sensor and read the values from the sensor.
 * @return - SENSOR_OK or SENSOR_ERROR_TIMEOUT if reading the value from the sensor
 * is successful or unsuccessful respectively.
 */
static SENSOR_RETURN_TYPE_T read(uint8_t *bits)
{
	uint8_t mask = 128;
	uint8_t idx = 0;
	uint8_t i = 0;
	volatile unsigned int loopCnt;
	// EMPTY BUFFER
	memset(bits, 0, SENSOR_DATA_BUFFER_SIZE);

	/*sensor reading operation needs to be atomic i.e task switching should not happen
	 this function will prevent swapping of the tasks without disabling the interrupts.
	 */

	//TBD: need to a find way to reduce error without locking
	//vTaskSuspendAll();

	// To request sample from sensor we need to pull pin high for 18 miliseconds
	//and then low for 20-40 microseconds.
	PLIB_PORTS_PinModePerPortSelect(PORTS_ID_0, DHT_SENSOR_PORT, DHT_SENSOR_PIN, PORTS_PIN_MODE_DIGITAL);
	PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, DHT_SENSOR_PORT, DHT_SENSOR_PIN);
	SENSOR_PIN_WRITE(LOW);
	loopCnt = PULL_PIN_LOW_TIMEOUT; //this is 18 millisecond delay
	while (loopCnt--);

	SENSOR_PIN_WRITE(HIGH);
	loopCnt = PULL_PIN_HIGH_TIMEOUT; //this is 38 microsecond delay
	while (loopCnt--);

	PLIB_PORTS_PinDirectionInputSet(PORTS_ID_0, DHT_SENSOR_PORT, DHT_SENSOR_PIN);

	loopCnt = TIMEOUT;
	while (SENSOR_PIN_READ() == LOW)
	{
		if (--loopCnt == 0)
		{
			//xTaskResumeAll();
			return SENSOR_ERROR_TIMEOUT;
		}
	}

	loopCnt = TIMEOUT;
	while (SENSOR_PIN_READ() == HIGH)
	{
		if (--loopCnt == 0)
		{
			//xTaskResumeAll();
			return SENSOR_ERROR_TIMEOUT;
		}
	}
	// READ THE OUTPUT - 40 BITS => 5 BYTES(amount of data received from sensor).
	for (i = 0; i < SENSOR_DATA_BUFFER_SIZE_IN_BITS; i++)
	{
		loopCnt = TIMEOUT;
		while (SENSOR_PIN_READ() == LOW)
		{
			if (--loopCnt == 0)
			{
				//xTaskResumeAll();
				return SENSOR_ERROR_TIMEOUT;
			}
		}

		unsigned long t = ReadCoreTimer(); //Returns the current Core Timer value.

		loopCnt = TIMEOUT;
		while (SENSOR_PIN_READ() == HIGH)
		{
			if (--loopCnt == 0)
			{
				//xTaskResumeAll();
				return SENSOR_ERROR_TIMEOUT;
			}
		}

		if (((ReadCoreTimer() - t) / CORETIMER_TICKS_PER_MICROSECOND) > VOLTAGE_LENGTH)
		{
			//sensor has sent high bit i.e. 1
			bits[idx] |= mask;
		}
		mask >>= 1;
		if (mask == 0) // next byte?
		{
			mask = 128;
			idx++;
		}
	}

	//resume task switching.
	//xTaskResumeAll();
	return SENSOR_OK;
}
