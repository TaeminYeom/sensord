/*
 * sensorctl
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include "info.h"

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <sensor_internal.h>

#include "log.h"

#define INFO_ARGC 3 /* e.g. {sensorctl, info, accelerometer} */

bool info_manager::run(int argc, char *argv[])
{
	sensor_type_t type;
	sensor_t *sensors;
	int count;

	if (argc < INFO_ARGC) {
		usage();
		return false;
	}

	type = get_sensor_type(argv[2]);
	RETVM_IF(type == UNKNOWN_SENSOR, false, "Wrong argument : %s\n", argv[2]);

	sensord_get_sensor_list(type, &sensors, &count);
	show_info(sensors, count);

	if (sensors) {
		free(sensors);
		sensors = NULL;
	}
	return true;
}

void info_manager::show_info(sensor_t *sensors, int count)
{
	sensor_type_t type;
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

		sensord_get_type(sensor, &type);
		name = const_cast<char *>(sensord_get_name(sensor));
		vendor = const_cast<char *>(sensord_get_vendor(sensor));
		sensord_get_max_range(sensor, &max_range);
		sensord_get_min_range(sensor, &min_range);
		sensord_get_resolution(sensor, &resolution);
		sensord_get_min_interval(sensor, &min_interval);
		sensord_get_fifo_count(sensor, &fifo_count);
		sensord_get_max_batch_count(sensor, &max_batch_count);

		_N("-------sensor[%d] information-------\n", i);
		_N("type            : %#x\n", type);
		_N("vendor          : %s\n", vendor);
		_N("name            : %s\n", name);
		_N("min_range       : %f\n", min_range);
		_N("max_range       : %f\n", max_range);
		_N("resolution      : %f\n", resolution);
		_N("min_interval    : %d\n", min_interval);
		_N("fifo_count      : %d\n", fifo_count);
		_N("max_batch_count : %d\n", max_batch_count);
		_N("--------------------------------\n");
	}
}

void info_manager::usage(void)
{
	_N("usage: sensorctl info <sensor_type>\n\n");

	usage_sensors();
}
