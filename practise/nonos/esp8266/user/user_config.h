#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define ESP_DEBUG

#define ESP_DEBUG
#define USE_OPTIMIZE_PRINTF

#ifdef USE_INTER_LED
	#undef ESP_DEBUG
#endif

#ifdef ESP_DEBUG
	#define ESP_DBG os_printf
#else
	#define ESP_DBG
#endif

#endif