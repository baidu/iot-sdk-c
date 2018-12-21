#ifndef DHT11_H_INCLUDED
#define DHT11_H_INCLUDED

#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "driver/gpio.h"


#define DHT_ReadInterval			1500
#define __DHT_Delay_Setup			2000

enum DHT_Status_t
{
	DHT_Ok,
	DHT_Error_Humidity,
	DHT_Error_Temperature,
	DHT_Error_Checksum,
	DHT_Error_Timeout
};

void DHT_Setup(int);
void DHT_Read(double *Temperature, double *Humidity);

#endif