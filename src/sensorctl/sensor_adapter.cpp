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

#include "sensor_adapter.h"

#include <sensor_internal.h>

#include "log.h"
#include "mainloop.h"
#include "test_bench.h"

#define SENSOR_EVENT(type) ((type) << 16 | 0x1)

bool sensor_adapter::is_batch_mode = false;

bool sensor_adapter::is_supported(sensor_type_t type)
{
	sensor_t sensor;

	int ret = sensord_get_default_sensor(type, &sensor);
	if (ret == 0)
		return true;

	return false;
}

int sensor_adapter::get_count(sensor_type_t type)
{
	sensor_t *sensors = nullptr;
	int count = 0;

	if (sensord_get_sensors(type, &sensors, &count) == 0)
		free(sensors);

	return count;
}

bool sensor_adapter::get_handle(sensor_info info, int &handle)
{
	int err;
	int count;
	sensor_t *sensors = NULL;

	err = sensord_get_sensors(info.type, &sensors, &count);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensors[info.index]);
	ASSERT_FREE((handle < 0), sensors);
	ASSERT_GE(handle, 0);

	free(sensors);
	sensors = NULL;

	return true;
}

bool sensor_adapter::start(sensor_info info, int &handle)
{
	sensor_t *sensors = NULL;
	int count;
	int err;
	bool ret;

	err = sensord_get_sensors(info.type, &sensors, &count);
	ASSERT_EQ(err, 0);

	ASSERT_FREE((info.index >= count), sensors);
	ASSERT_LT(info.index, count);

	ASSERT_FREE((info.index < 0), sensors);
	ASSERT_GE(info.index, 0);

	handle = sensord_connect(sensors[info.index]);
	ASSERT_FREE((handle < 0), sensors);
	ASSERT_GE(handle, 0);

	if (is_batch_mode) {
		ret = sensord_register_events(handle, SENSOR_EVENT(info.type), info.batch_latency, info.events_cb, NULL);
	} else {
		ret = sensord_register_event(handle, SENSOR_EVENT(info.type), info.interval, info.batch_latency, info.cb, NULL);
	}
	ASSERT_TRUE(ret);

	ret = sensord_start(handle, info.powersave);
	ASSERT_FREE((ret != true), sensors);
	ASSERT_TRUE(ret);

	free(sensors);
	sensors = NULL;

	return true;
}

bool sensor_adapter::stop(sensor_info info, int handle)
{
	bool ret;

	ret = sensord_stop(handle);
	EXPECT_TRUE(ret);

	if (is_batch_mode) {
		ret = sensord_unregister_events(handle, SENSOR_EVENT(info.type));
	} else {
		ret = sensord_unregister_event(handle, SENSOR_EVENT(info.type));
	}
	EXPECT_TRUE(ret);

	ret = sensord_disconnect(handle);
	EXPECT_TRUE(ret);

	return true;
}

bool sensor_adapter::change_interval(int handle, int interval)
{
	return true;
}

bool sensor_adapter::change_batch_latency(int handle, int batch_latency)
{
	return true;
}

bool sensor_adapter::change_powersave(int handle, int powersave)
{
	return true;
}

bool sensor_adapter::set_attribute(int handle, int attribute, int value)
{
	bool ret;

	ret = sensord_set_attribute_int(handle, attribute, value);
	ASSERT_TRUE(ret);

	return true;
}

bool sensor_adapter::set_attribute(int handle, int attribute, char *value, int size)
{
	int ret;

	ret = sensord_set_attribute_str(handle, attribute, value, size);

	return ((ret == 0) ? true : false);
}

bool sensor_adapter::get_data(int handle, sensor_type_t type, sensor_data_t &data)
{
	bool ret;

	ret = sensord_get_data(handle, SENSOR_EVENT(type), &data);
	ASSERT_TRUE(ret);

	return true;
}

bool sensor_adapter::flush(int handle)
{
	bool ret;

	ret = sensord_flush(handle);
	ASSERT_TRUE(ret);

	return true;
}

