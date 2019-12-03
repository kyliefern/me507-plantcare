/*
 * @file main.cpp
 *
 *  Created on: Nov 7, 2019
 *      Author: kyliefern
 */

#include "main.h"
#include "Arduino_FreeRTOS.h"
#include "Adafruit_seesaw.h"
#include <SPI.h>
#include "LS7366RLib.h"

void setup()
{
	xTaskCreate(TaskLight, (const portCHAR *) "Light",
				128, NULL, 1, NULL);
	xTaskCreate(TaskWater, (const portCHAR *) "Water",
				128, NULL, 2, NULL);
}

void loop()
{
	// empty
}

void TaskLight(void *pvParameters)
{
	(void) pvParameters;
	// setup
	uint16_t limit = 500; // photo cell limit
	volatile uint16_t count = 0; // light count
	LS7366R Encoder(2); // encoder object
	Encoder.ResetEncoder();
	volatile uint32_t EncoderVal = Encoder.ReadEncoder();
	uint32_t target = 1500;
	pinMode(5, OUTPUT); // motor pins
	pinMode(6, OUTPUT);
	digitalWrite(5, LOW); // motor off
	digitalWrite(6, LOW);
	volatile uint8_t state = 0;

	for(;;)
	{
		// loop
		if (state == 0)
		{
			// checking light
			volatile uint16_t light = analogRead(3);
			if (light > limit)
			{
				count++;
			} else
			{
				count = 0;
			}
			if (count > 2000)
			{
				state = 1;
			}
		}
		else if (state == 1)
		{
			// shading
			while (EncoderVal != target)
			{
				EncoderVal = Encoder.ReadEncoder();
				int duty = 255*(target - EncoderVal)/target;
				if (duty < 0)
				{
					digitalWrite(6, LOW);
					analogWrite(5, abs(duty));
				}
				digitalWrite(5, LOW);
				analogWrite(6, duty);
			}
			digitalWrite(5, LOW);
			digitalWrite(6, LOW);
			for (uint16_t x=0; x < 3600; x++)
			{
				delay(1000);
			}
			state = 0;
		}
		vTaskDelay(100);
	}
}

void TaskWater(void *pvParameters)
{
	(void) pvParameters;
	// setup
	Adafruit_seesaw ss; // moisture sensor object
	uint16_t caplimit = 500; // moisture limit
	pinMode(9, OUTPUT); // valve pins
	pinMode(10, OUTPUT);
	digitalWrite(9, LOW); // valve closed
	digitalWrite(10, LOW);
	volatile uint8_t state = 0;

	for(;;)
	{
		// loop
		if (state == 0)
		{
			// checking moisture
			uint16_t capread = ss.touchRead(0);
			if (capread > caplimit)
			{
				state = 1;
			}
		}
		else if (state == 1)
		{
			// watering
			analogWrite(10, 255); // open valve
			// check flow meter
		}
		vTaskDelay(100);
	}
}
