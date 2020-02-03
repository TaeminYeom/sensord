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

#include <unistd.h>
#include <string.h>
#include <sensor_internal.h>

#include "log.h"
#include "mainloop.h"
#include "test_bench.h"
#include "sensor_adapter.h"

/* exception list */
#define SAMSUNG_PEDOMETER "http://samsung.com/sensor/healthinfo/pedometer/samsung_pedometer"

static void basic_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	EXPECT_GT(data->timestamp, 0);
	_N(_WHT "[   DATA   ] %f\n" _RST, data->values[0]);
	mainloop::stop();
}

static bool skip_sensor(sensor_t sensor)
{
	const char *uri = sensord_get_uri(sensor);

	if (strncmp(uri, SAMSUNG_PEDOMETER, sizeof(SAMSUNG_PEDOMETER)) == 0)
		return true;

	return false;
}

TESTCASE(sensor_basic, all_sensor_p)
{
	int err, count, handle;
	bool ret;
	sensor_t *sensors = nullptr;
	sensor_type_t type;

	err = sensord_get_sensors(ALL_SENSOR, &sensors, &count);
	ASSERT_EQ(err, 0);

	for (int i = 0; i < count; ++i) {
		if (skip_sensor(sensors[i]))
			continue;

		_W("[ SENSOR %d ] %s\n", i, sensord_get_uri(sensors[i]));

		sensord_get_type(sensors[i], &type);
		sensor_info info(type, 0, 100, 1000, SENSOR_OPTION_ALWAYS_ON, basic_cb, NULL);

		ret = sensor_adapter::start(info, handle);
		EXPECT_TRUE(ret);

		mainloop::run();

		ret = sensor_adapter::stop(info, handle);
		EXPECT_TRUE(ret);
	}

	free(sensors);

	return true;
}
