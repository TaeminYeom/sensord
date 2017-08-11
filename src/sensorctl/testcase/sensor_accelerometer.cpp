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

#include <sensor_internal.h>

#include "log.h"
#include "mainloop.h"
#include "test_bench.h"
#include "sensor_adapter.h"

static void test_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	EXPECT_GT(data->timestamp, 0);
	EXPECT_NEAR(data->values[0], 0, 19.6);
	EXPECT_NEAR(data->values[1], 0, 19.6);
	EXPECT_NEAR(data->values[2], 0, 19.6);

	mainloop::stop();
}

TESTCASE(accelerometer_test, start_stop_p)
{
	bool ret;
	int handle;

	if (!sensor_adapter::is_supported(ACCELEROMETER_SENSOR))
		return true; /* Not Supported */

	sensor_info info(ACCELEROMETER_SENSOR, 0,
			100, 1000, SENSOR_OPTION_ALWAYS_ON, test_cb, NULL);

	ret = sensor_adapter::start(info, handle);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensor_adapter::stop(info, handle);
	ASSERT_TRUE(ret);

	return true;
}

static void get_data_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	mainloop::stop();
}

TESTCASE(accelerometer_test, get_data_p)
{
	bool ret;
	int handle;
	sensor_data_t data;

	if (!sensor_adapter::is_supported(ACCELEROMETER_SENSOR))
		return true; /* Not Supported */

	sensor_info info(ACCELEROMETER_SENSOR, 0,
			100, 1000, SENSOR_OPTION_ALWAYS_ON, get_data_cb, NULL);

	ret = sensor_adapter::start(info, handle);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensor_adapter::get_data(handle, info.type, data);
	ASSERT_TRUE(ret);

	ret = sensor_adapter::stop(info, handle);
	ASSERT_TRUE(ret);

	return true;
}

static unsigned long long time_first;
static unsigned long long time_last;
static int event_count;

static void accel_interval_100ms_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	if (event_count == 0) {
		time_first = data->timestamp;
		event_count++;
		return;
	}

	if (event_count == 10) {
		/* 100ms + 20ms(error) */
		EXPECT_LE((data->timestamp - time_first) / 10, 120000);
		mainloop::stop();
		return;
	}

	event_count++;
}

TESTCASE(accelscope_test, 100ms_interval_p)
{
	bool ret;
	int handle;

	time_first = 0;
	time_last = 0;
	event_count = 0;

	if (!sensor_adapter::is_supported(ACCELEROMETER_SENSOR))
		return true; /* Not Supported */

	sensor_info info(ACCELEROMETER_SENSOR, 0,
			100, 1000, SENSOR_OPTION_ALWAYS_ON, accel_interval_100ms_cb, NULL);

	ret = sensor_adapter::start(info, handle);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensor_adapter::stop(info, handle);
	ASSERT_TRUE(ret);

	return true;
}
