// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - external variable definitions
// In the appropriate section

#ifndef _MAIN_H_
#define _MAIN_H_

//add your includes for the project PlantCare here

#include "Arduino_FreeRTOS.h"
#include "Adafruit_seesaw.h"
#include <SPI.h>
#include "LS7366RLib.h"

//end of add your includes here

//add your function definitions for the project PlantCare here

void TaskLight(void *pvParameters);
void TaskWater(void *pvParameters);

//Do not add code below this line
#endif /* _MAIN_H_ */
