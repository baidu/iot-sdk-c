/*
* Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.
* The ASF licenses this file to You under the Apache License, Version 2.0
* (the "License"); you may not use this file except in compliance with
* the License.  You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "esp_types.h"
#include "user_config.h"

#include "iothub_mqtt_client_sample.h"


static  void ICACHE_FLASH_ATTR wifi_handle_event_cb(System_Event_t *evt)
{
	ESP_DBG("event %x\n", evt->event);

	switch (evt->event) {
		case EVENT_STAMODE_CONNECTED:
			break;
		case EVENT_STAMODE_DISCONNECTED:
			ESP_DBG("disconnect from ssid %s, reason %d\n",
			evt->event_info.disconnected.ssid,
			evt->event_info.disconnected.reason);
			break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
			ESP_DBG("mode: %d -> %d\n",
			evt->event_info.auth_change.old_mode,
			evt->event_info.auth_change.new_mode);
			break;
		case EVENT_STAMODE_GOT_IP:
			os_printf("heap %d\n",system_get_free_heap_size());			
			
			iothub_mqtt_client_run();


			break;
		case EVENT_SOFTAPMODE_STACONNECTED:
			ESP_DBG("station: " MACSTR "join, AID = %d\n",
			MAC2STR(evt->event_info.sta_connected.mac),
			evt->event_info.sta_connected.aid);
			break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			ESP_DBG("station: " MACSTR "leave, AID = %d\n",
			MAC2STR(evt->event_info.sta_disconnected.mac),
			evt->event_info.sta_disconnected.aid);
			break;
		default:
			break;
	}
}




void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}

void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
   // Wifi configuration 
   char ssid[32] = "Honor8"; 
   char password[64] = "243587123"; 
   struct station_config stationConf; 
   
   os_memset(stationConf.ssid, 0, 32);
   os_memset(stationConf.password, 0, 64);
   //need not mac address
   stationConf.bssid_set = 0; 
   
   //Set ap settings 
   memcpy(&stationConf.ssid, ssid, 32); 
   memcpy(&stationConf.password, password, 64); 
   wifi_station_set_config(&stationConf); 

}

void ICACHE_FLASH_ATTR
user_init(void)
{
	ESP_DBG("SDK version:%s\n", system_get_sdk_version());
	system_print_meminfo();
	wifi_set_event_handler_cb(wifi_handle_event_cb);
	wifi_set_opmode(STATION_MODE);
    user_set_station_config();
	wifi_set_sleep_type(NONE_SLEEP_T);
}
