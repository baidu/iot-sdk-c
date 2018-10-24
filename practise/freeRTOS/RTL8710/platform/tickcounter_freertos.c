// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/gballoc.h"
#include <stdint.h>
#include <time.h>

#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

#ifndef CONFIG_FREERTOS_HZ
   #define CONFIG_FREERTOS_HZ 1000
#endif


typedef struct TICK_COUNTER_INSTANCE_TAG
{
    uint32_t original_tick_count;
} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
	TICK_COUNTER_INSTANCE* result = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (result == NULL)
    {
		LogError("Failed creating tick counter");
    }
	else
	{
		// TODO
		//result->original_tick_count = xxx_system_ticks();
	}
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
	if (tick_counter != NULL)
    {
		free(tick_counter);
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
    int result;

	if (tick_counter == NULL || current_ms == NULL)
    {
		LogError("Invalid Arguments.\r\n");
        result = __FAILURE__;
    }
    else
    {
		// TODO
		//*current_ms = (tickcounter_ms_t)( \
			((uint32_t)(xxx_system_ticks() - tick_counter->original_tick_count)) * 1000.0 / CONFIG_FREERTOS_HZ );
        result = 0;
    }

    return result;
}
