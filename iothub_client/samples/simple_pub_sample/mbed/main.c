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

#include <stdio.h>
#include <string.h>
#include "simple_pub_sample.h"

int main(int argc, char* argv[])
{
	if (argc < 5) {
		printf("USAGE: simple_pub_sample <endpoint> <username> <password> <topic> [clientid] [useSsl]\r\n");
		printf("EXAMPLE: simple_pub_sample myendpoint.mqtt.iot.gz.baidubce.com myendpoint/device PAYLijiHPjuRoQE4SeUBw346/q3DhUb2Ht0jdi31IsU= mytopic qnx660client01 false\r\n");
		return -1;
	}
	char* clientid = "qnx660client01";
	if (argc >= 6) {
		clientid = argv[5];
	}
	char useSsl = 0;
	if (argc >= 7 && strcmp("true", argv[6]) == 0) {
		useSsl = 1;
	}
    simple_pub_sample_run(argv[1], argv[2], argv[3], argv[4], clientid, useSsl);
    return 0;
}
