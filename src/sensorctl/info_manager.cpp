/*
 * sensorctl
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <glib.h>
#include <sensor_internal.h>
#include <sensorctl_log.h>
#include "info_manager.h"

bool info_manager::process(int argc, char *argv[])
{
	sensor_type_t type;

	if (argc == 2) {
		usage();
		return false;
	}

	type = get_sensor_type(argv[2]);
	if (type == UNKNOWN_SENSOR)
		return false;

	sensor_t *sensors;
	int count;

	sensord_get_sensor_list(type, &sensors, &count);
	sensor_info(sensors, count);

	free(sensors);
	return true;
}

void info_manager::sensor_info(sensor_t *sensors, int count)
{
	sensor_t sensor;
	char *vendor;
	char *name;
	float min_range;
	float max_range;
	float resolution;
	int min_interval;
	int fifo_count;
	int max_batch_count;

	for (int i = 0; i < count; ++i) {
		sensor = sensors[i];

		name = const_cast<char *>(sensord_get_name(sensor));
		vendor = const_cast<char *>(sensord_get_vendor(sensor));
		sensord_get_max_range(sensor, &max_range);
		sensord_get_min_range(sensor, &min_range);
		sensord_get_resolution(sensor, &resolution);
		sensord_get_min_interval(sensor, &min_interval);
		sensord_get_fifo_count(sensor, &fifo_count);
		sensord_get_max_batch_count(sensor, &max_batch_count);

		PRINT("-------sensor[%d] information-------\n", i);
		PRINT("vendor : %s\n", vendor);
		PRINT("name : %s\n", name);
		PRINT("min_range : %f\n", min_range);
		PRINT("max_range : %f\n", max_range);
		PRINT("resolution : %f\n", resolution);
		PRINT("min_interval : %d\n", min_interval);
		PRINT("fifo_count : %d\n", fifo_count);
		PRINT("max_batch_count : %d\n", max_batch_count);
		PRINT("--------------------------------\n");
	}
}

void info_manager::usage(void)
{
	PRINT("usage: sensorctl info <sensor_type>\n");
	PRINT("\n");

	usage_sensors();
}

