/*!
	\file main.cpp
	\brief Main file containing project tasks.

	This file contains two classes, one to control plant watering and one to
	control plant shading. Used in conjunction with the physical system,
	this provides autonomous care for a house plant.
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
	Serial.begin(9600); // begin serial communication
}

void loop()
{
	// empty
}

/*!
	\brief TaskLight controls plant shading

	This task function checks a photo cell light sensor to determine how long
	the plant has been in the sun. After 2 hours of sun, a DC motor will cover
	the plant with a shade using encoder feedback. The shade is removed after
	one hour.
 */
void TaskLight(void *pvParameters)
{
	/*!
		\param limit The maximum limit of the photo cell sensor reading.
		\param count A counter to track plant time in the sunlight.
		\param EncoderVal The current position of the encoder.
		\param target The target position of the shade above the plant.
		\param state The current state of the light task.
		\param light The current reading from the photo cell.
	 */
	(void) pvParameters;
	Serial.println("Initializing light task");

	pinMode(A3, INPUT_PULLDOWN); // photo cell with pull down
	uint16_t limit = 400; // photo cell limit
	volatile uint32_t count = 0; // sunlight time count
	LS7366R Encoder(2); // encoder object
	Encoder.ResetEncoder(); // reset encoder
	volatile uint32_t EncoderVal = Encoder.ReadEncoder();
	uint8_t target = 112; // 180 degrees
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
			Serial.println("Light task running: 0");

			volatile uint16_t light = analogRead(A3); // photo cell reading
			if (light > limit)
			{
				count++; // increment if sunny
			} else
			{
				count = 0; // zero if not sunny
			}
			if (count > 720000)
			{
				state = 1; // sunlight for an hour straight
			}
		}
		else if (state == 1)
		{
			// shading
			Serial.println("Light task running: 1");

			while (EncoderVal != target) // move shade 180 degrees
			{
				EncoderVal = Encoder.ReadEncoder(); // current encoder value
				int duty = 170*(target - EncoderVal)/target; // 6V duty cycle
				if (duty < 0)
				{
					digitalWrite(6, LOW);
					analogWrite(5, abs(duty)); // negative direction
				}
				digitalWrite(5, LOW);
				analogWrite(6, duty); // positive direction
			}
			digitalWrite(5, LOW); // motor off
			digitalWrite(6, LOW);
			for (uint16_t x=0; x < 3600; x++)
			{
				delay(1000); // shade for an hour
			}
			while (EncoderVal != 0) // move shade back
			{
				EncoderVal = Encoder.ReadEncoder(); // current encoder value
				int duty = 170*EncoderVal/target; // 6V duty cycle
				if (duty < 0)
				{
					digitalWrite(6, LOW);
					analogWrite(5, abs(duty)); // positive direction
				}
				digitalWrite(5, LOW);
				analogWrite(6, duty); // negative direction
			}
			digitalWrite(5, LOW); // motor off
			digitalWrite(6, LOW);
			state = 0;
		}
		vTaskDelay(10);
	}
}

/*!
	\brief TaskWater controls plant watering

	This task function checks a moisture sensor in the plant soil and distributes
	water accordingly. When the plant is determined to be dry, a valve opens. After
	a flow meter measures 0.1 liters of water, the valve closes.
 */
void TaskWater(void *pvParameters)
{
	/*!
		\param caplimit The minimum limit of the moisture sensor reading.
		\param pulses A counter tracking pulses from the flow meter.
		\param lastflowpinstate The last state of the flow meter pin.
		\param count A timeout counter for the water tank level.
		\param capread The current reading from the moisture sensor.
	 */
	(void) pvParameters;
	Serial.println("Initializing water task");

	Adafruit_seesaw ss; // moisture sensor object
	uint16_t caplimit = 250; // moisture limit
	pinMode(9, OUTPUT); // valve pins
	pinMode(10, OUTPUT);
	digitalWrite(9, LOW); // valve closed
	digitalWrite(10, LOW);
	volatile uint16_t pulses = 0; // flow meter pulses
	volatile uint8_t lastflowpinstate; // flow meter pin state
	pinMode(8, INPUT); // flow meter pin
	digitalWrite(8, HIGH);
	lastflowpinstate = digitalRead(8);
	uint16_t count = 0; // flow meter timeout
	pinMode(7, OUTPUT); // red LED pin
	digitalWrite(7, LOW); // red LED off

	volatile uint8_t state = 0;

	for(;;)
	{
		// loop
		if (state == 0)
		{
			// checking moisture
			Serial.println("Water task running: 0");

			uint16_t capread = ss.touchRead(0); // moisture reading
			if (capread < caplimit)
			{
				state = 1; // soil is dry
			}
		}
		else if (state == 1)
		{
			// watering
			Serial.println("Water task running: 1");

			digitalWrite(7, LOW); // turn off red LED
			analogWrite(10, 255); // open valve
			while(pulses <= 45)
			{
				uint8_t x = digitalRead(8);
				if (x == lastflowpinstate)
				{
					// nothing changed
				} else if (x == HIGH)
				{
					//low to high transition
					pulses++;
				}
				lastflowpinstate = x;
				count++;
				if (count >= 1000)
				{
					digitalWrite(7, HIGH); // timeout LED on
					break;
				}
			}
			digitalWrite(10, LOW); // close valve
			delay(5000); // wait 5 seconds
			pulses = 0;
			count = 0;
			state = 0;
		}
		vTaskDelay(10);
	}
}
