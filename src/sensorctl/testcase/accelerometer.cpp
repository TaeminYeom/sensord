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

static void basic_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	EXPECT_GT(data->timestamp, 0);
	EXPECT_NEAR(data->values[0], 0, 19.6);
	EXPECT_NEAR(data->values[1], 0, 19.6);
	EXPECT_NEAR(data->values[2], 0, 19.6);

	mainloop::stop();
}

TESTCASE(accelerometer_basic, start_stop_p)
{
	bool ret;
	int handle;

	sensor_info info(ACCELEROMETER_SENSOR, 0,
			100, 1000, SENSOR_OPTION_ALWAYS_ON, basic_cb, NULL);

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

TESTCASE(accelerometer_basic, get_data_p)
{
	bool ret;
	int handle;
	sensor_data_t data;

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

static unsigned long long prev_prev_ts;
static unsigned long long prev_ts;
static int event_count;

static void accel_regular_interval_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	int prev_gap;
	int current_gap;
	if (prev_prev_ts == 0) {
		prev_prev_ts = data->timestamp;
		return;
	}

	if (prev_ts == 0) {
		prev_ts = data->timestamp;
		return;
	}

	prev_gap = prev_ts - prev_prev_ts;
	current_gap = data->timestamp - prev_ts;

	EXPECT_NEAR(current_gap, prev_gap, 10000);
	prev_prev_ts = prev_ts;
	prev_ts = data->timestamp;

	if (event_count++ > 3)
		mainloop::stop();
}

TESTCASE(accelerometer_interval, regular_interval_p)
{
	bool ret;
	int handle;
	prev_prev_ts = 0;
	prev_ts = 0;
	event_count = 0;

	sensor_info info(ACCELEROMETER_SENSOR, 0,
			100, 1000, SENSOR_OPTION_ALWAYS_ON, accel_regular_interval_cb, NULL);

	ret = sensor_adapter::start(info, handle);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensor_adapter::stop(info, handle);
	ASSERT_TRUE(ret);

	return true;
}

static void accel_interval_100ms_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	if (prev_ts == 0) {
		prev_ts = data->timestamp;
		return;
	}

	/* 100ms + 20ms(error) */
	EXPECT_LE(data->timestamp - prev_ts, 120000);
	prev_ts = data->timestamp;

	if (event_count++ > 3)
		mainloop::stop();
}

TESTCASE(accelerometer_interval, 100ms_interval_p)
{
	bool ret;
	int handle;

	prev_ts = 0;
	event_count = 0;

	sensor_info info(ACCELEROMETER_SENSOR, 0,
			100, 1000, SENSOR_OPTION_ALWAYS_ON, accel_interval_100ms_cb, NULL);

	ret = sensor_adapter::start(info, handle);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensor_adapter::stop(info, handle);
	ASSERT_TRUE(ret);

	return true;
}
