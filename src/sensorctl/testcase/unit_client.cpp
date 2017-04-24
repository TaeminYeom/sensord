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
#include <sensor_utils.h>

#include <client/sensor_manager.h>
#include <client/sensor_listener.h>
#include <ipc_client.h>
#include <channel_handler.h>

#include "log.h"
#include "mainloop.h"
#include "test_bench.h"

TESTCASE(sensor_api_get_default_sensor, get_sensor_p_1)
{
	int err;
	sensor_t sensor;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	return true;
}

TESTCASE(sensor_api_get_sensors, get_sensor_p_2)
{
	int err;
	int count;
	sensor_t *sensors;

	err = sensord_get_sensors(ACCELEROMETER_SENSOR, &sensors, &count);
	ASSERT_EQ(err, 0);
	ASSERT_GT(count, 0);

	free(sensors);

	return true;
}

TESTCASE(sensor_api_connect, connect_p_1)
{
	int err;
	int handle;
	sensor_t sensor;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ASSERT_GT(handle, 0);

	err = sensord_disconnect(handle);
	ASSERT_EQ(err, 1);

	return true;
}

static bool called = false;

static void basic_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	if (test_option::full_log == false) {
		while (true) {}
	}
	_I("[%llu] %f %f %f\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

TESTCASE(sensor_api_all, all_p_1)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;
	sensor_t *list;
	int count;

	called = false;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	err = sensord_get_sensors(ALL_SENSOR, &list, &count);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ASSERT_EQ(err, 0);

	ret = sensord_register_event(handle, 1, 100, 100, basic_cb, NULL);
	ASSERT_TRUE(ret);

	ret = sensord_start(handle, 0);
	ASSERT_TRUE(ret);

	ret = sensord_change_event_interval(handle, 0, 100);
	ASSERT_TRUE(ret);

	ret = sensord_change_event_max_batch_latency(handle, 0, 100);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_unregister_event(handle, 1);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	free(list);

	return true;
}

static bool m_start = false;

static void start_cb(sensord_provider_h provider, void *user_data)
{
	m_start = true;
	_N("START\n");
}

static void stop_cb(sensord_provider_h provider, void *user_data)
{
	m_start = false;
	_N("STOP\n");
}

static void interval_cb(sensord_provider_h provider, unsigned int interval_ms, void *user_data)
{
	_N("Interval : %d\n", interval_ms);
}

static gboolean publish(gpointer gdata)
{
	if (!m_start) return TRUE;

	sensord_provider_h *provider = reinterpret_cast<sensord_provider_h *>(gdata);

	sensor_data_t data;
	data.accuracy = 3;
	data.timestamp = sensor::utils::get_timestamp();
	data.value_count = 3;
	data.values[0] = 1;
	data.values[1] = 2;
	data.values[2] = 3;

	_N("[%llu] %f %f %f\n", data.timestamp, data.values[0], data.values[1], data.values[2]);
	sensord_provider_publish(provider, data);
	return TRUE;
}

/* TODO: change it from manual test to auto-test */
TESTCASE(sensor_api_mysensor_provider, provider_p_1)
{
	const char *uri = "http://example.org/mysensor/mysensor";
	const char *name = "mysensor";
	const char *vendor = "tizen";

	int err = 0;
	sensor_t sensor;
	sensord_provider_h provider;

	err = sensord_create_provider(uri, &provider);
	ASSERT_EQ(err, 0);

	err = sensord_provider_set_name(provider, name);
	ASSERT_EQ(err, 0);
	err = sensord_provider_set_vendor(provider, vendor);
	ASSERT_EQ(err, 0);
	err = sensord_provider_set_range(provider, 0.0f, 1.0f);
	ASSERT_EQ(err, 0);
	err = sensord_provider_set_resolution(provider, 0.01f);
	ASSERT_EQ(err, 0);

	err = sensord_add_provider(provider);
	ASSERT_EQ(err, 0);

	err = sensord_provider_set_start_cb(provider, start_cb, NULL);
	ASSERT_EQ(err, 0);
	err = sensord_provider_set_stop_cb(provider, stop_cb, NULL);
	ASSERT_EQ(err, 0);
	err = sensord_provider_set_set_interval_cb(provider, interval_cb, NULL);
	ASSERT_EQ(err, 0);

	err = sensord_get_default_sensor_by_uri(uri, &sensor);
	ASSERT_EQ(err, 0);

	g_timeout_add_seconds(1, publish, provider);
	mainloop::run();

	err = sensord_remove_provider(provider);
	ASSERT_EQ(err, 0);
	err = sensord_destroy_provider(provider);
	ASSERT_EQ(err, 0);

	return true;
}

TESTCASE(sensor_api_mysensor_listener, listener_p_1)
{
	const char *uri = "http://example.org/mysensor/mysensor";
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor_by_uri(uri, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ASSERT_EQ(err, 0);

	ret = sensord_register_event(handle, 1, 100, 100, basic_cb, NULL);
	ASSERT_TRUE(ret);

	ret = sensord_start(handle, 0);
	ASSERT_TRUE(ret);

	ret = sensord_change_event_interval(handle, 0, 100);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_unregister_event(handle, 1);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

static void add_mysensor(void)
{
	const char *uri = "http://example.org/mysensor/mysensor";
	const char *name = "mysensor";
	const char *vendor = "tizen";

	sensord_provider_h provider;

	sensord_create_provider(uri, &provider);
	sensord_provider_set_name(provider, name);
	sensord_provider_set_vendor(provider, vendor);
	sensord_provider_set_range(provider, 0.0f, 1.0f);
	sensord_provider_set_resolution(provider, 0.01f);

	sensord_add_provider(provider);

	sensord_remove_provider(provider);
	sensord_destroy_provider(provider);
}

static bool added = false;

static void added_cb(const char *uri, void *user_data)
{
	_I("ADDED[%s]\n", uri);
	added = true;
}

static void removed_cb(const char *uri, void *user_data)
{
	_I("REMOVED[%s]\n", uri);
	if (added)
		mainloop::stop();
}

TESTCASE(sensor_api_mysensor_cb, mysensor_cb_p_1)
{
	int ret;

	ret = sensord_add_sensor_added_cb(added_cb, NULL);
	ASSERT_EQ(ret, 0);
	ret = sensord_add_sensor_removed_cb(removed_cb, NULL);
	ASSERT_EQ(ret, 0);

	add_mysensor();

	mainloop::run();

	ret = sensord_remove_sensor_added_cb(added_cb);
	ASSERT_EQ(ret, 0);
	ret = sensord_remove_sensor_removed_cb(removed_cb);
	ASSERT_EQ(ret, 0);

	return true;
}
