/*
 * sensorctl
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sensor_internal.h>
#include <sensorctl_log.h>
#include "loopback_manager.h"

#define SHUB_INST_LIB_ADD      ((char)-79)
#define SHUB_INST_LIB_REMOVE   ((char)-78)
#define SHUB_LOOP_BACK_LIB     12
#define DEFAULT_COMMAND_SIZE   3
#define MAX_COMMAND_SIZE       82
#define MAX_DATA_SIZE          84	/* -79, 12, 4byte Delaytime + 78 bytes data */

static void int_to_bytes(int32_t value, int length, char cmd[])
{
	/* Convert to Big-endian */
	for (int i = length - 1; i >= 0; i--) {
		cmd[i] = (value & 0xff);
		value = value >> 8;
	}
}

bool loopback_manager::process(int argc, char *argv[])
{
	if (argc < DEFAULT_COMMAND_SIZE || argc > MAX_COMMAND_SIZE) {
		usage();
		return false;
	}

	if (!strcmp(argv[2], "start")) {
		int handle;
		sensor_t sensor;
		sensor = sensord_get_sensor(CONTEXT_SENSOR);
		handle = sensord_connect(sensor);

		/*
		 *  sensorhub (bytes) command [0~1] delaytime [2~5] data[6~83]
		 *      shell (argv)  command [0~2] delaytime [3]   data[4~81]
		 */
		char test_data[MAX_DATA_SIZE] = {SHUB_INST_LIB_ADD, SHUB_LOOP_BACK_LIB,};
		int_to_bytes(atoi(argv[3]), sizeof(int), &(test_data[2]));

		for (int i = 4; i < argc; i++)
			test_data[i+2] = atoi(argv[i]);

		sensord_set_attribute_str(handle, 0, test_data, sizeof(test_data));
		sensord_disconnect(handle);
		return true;
	}

	if (!strcmp(argv[2], "stop")) {
		int handle;
		sensor_t sensor;
		sensor = sensord_get_sensor(CONTEXT_SENSOR);
		handle = sensord_connect(sensor);

		char test_data[4] = {SHUB_INST_LIB_REMOVE, SHUB_LOOP_BACK_LIB,};

		sensord_set_attribute_str(handle, 0, test_data, sizeof(test_data));
		sensord_disconnect(handle);
		return true;
	}

	usage();

	return false;
}

void loopback_manager::usage(void)
{
	PRINT("usage: sensorctl loopback [start/stop] [14byte sensor char data]\n");
	PRINT("ex: sensorctl loopback start 15000 1 1 19 1 (after 15000ms, wrist up event)\n");
	PRINT("ex: sensorctl loopback stop\n");
	PRINT(" * if not enought 14byte, remain bytes are filled with 0\n");
	PRINT("\n");
}

