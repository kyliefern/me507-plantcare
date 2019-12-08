/*
	\headerfile main.h
 */
// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - external variable definitions
// In the appropriate section

#ifndef _MAIN_H_
#define _MAIN_H_

// includes for the project PlantCare

#include "Arduino_FreeRTOS.h"
#include "Adafruit_seesaw.h"
#include <SPI.h>
//#include "LS7366RLib.h"

// end of includes

// function definitions for the project PlantCare

void TaskLight(void *pvParameters);
void TaskWater(void *pvParameters);

// no code added below this line
#endif /* _MAIN_H_ */
