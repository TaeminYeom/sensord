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

#include "log.h"
#include "mainloop.h"
#include "test_bench.h"

#define MYSENSOR_URI "http://example.org/sensor/general/mysensor/mysensor"
#define MYSENSOR_NAME "mysensor"
#define MYSENSOR_VENDOR "tizen"

static bool started = false;
static bool added = false;
static bool called = false;

static void event_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	_I("[%llu] %f %f %f\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

static void start_cb(sensord_provider_h provider, void *user_data)
{
	started = true;
	_N("START\n");
}

static void stop_cb(sensord_provider_h provider, void *user_data)
{
	started = false;
	_N("STOP\n");
}

static void interval_cb(sensord_provider_h provider, unsigned int interval_ms, void *user_data)
{
	_N("Interval : %d\n", interval_ms);
}

static gboolean publish(gpointer gdata)
{
	if (!started) return TRUE;

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

static void add_mysensor(void)
{
	sensord_provider_h provider;

	sensord_create_provider(MYSENSOR_URI, &provider);
	sensord_provider_set_name(provider, MYSENSOR_NAME);
	sensord_provider_set_vendor(provider, MYSENSOR_VENDOR);
	sensord_provider_set_range(provider, 0.0f, 1.0f);
	sensord_provider_set_resolution(provider, 0.01f);

	sensord_add_provider(provider);
	sensord_remove_provider(provider);

	sensord_destroy_provider(provider);
}

static void added_cb(const char *uri, void *user_data)
{
	_I("[  ADDED  ] %s\n", uri);
	added = true;
}

static void removed_cb(const char *uri, void *user_data)
{
	_I("[ REMOVED ] %s\n", uri);
	if (added)
		mainloop::stop();
}

TESTCASE(sensor_provider, check_uri)
{
	int err;
	sensord_provider_h provider;

	const char *uri_p1 = "http://example.org/sensor/general/mysensor_type/mysensor";
	const char *uri_p2 = "http://developer.samsung.com/sensor/general/mysensor_type/mysensor";
	const char *uri_n1 = "http://tizen.org/sensor/general/accelerometer/mysensor";
	const char *uri_n2 = "http://tizen.org/mysensor/general/accelerometer/mysensor";
	const char *uri_n3 = "http:/example.org/sensor/general/mysensor_type/mysensor";
	const char *uri_n5 = "http://example.org/sensor/general/mysensor_type";
	const char *uri_n4 = "http://example.org/sensor/general/mysensor_type/mysensor/mysensor";

	err = sensord_create_provider(uri_p1, &provider);
	EXPECT_EQ(err, 0);
	err = sensord_create_provider(uri_p2, &provider);
	EXPECT_EQ(err, 0);
	err = sensord_create_provider(uri_n1, &provider);
	EXPECT_EQ(err, -EINVAL);
	err = sensord_create_provider(uri_n2, &provider);
	EXPECT_EQ(err, -EINVAL);
	err = sensord_create_provider(uri_n3, &provider);
	EXPECT_EQ(err, -EINVAL);
	err = sensord_create_provider(uri_n4, &provider);
	EXPECT_EQ(err, -EINVAL);
	err = sensord_create_provider(uri_n5, &provider);
	EXPECT_EQ(err, -EINVAL);

	return true;
}

/* TODO: change it from manual test to auto-test */
TESTCASE(skip_sensor_provider, mysensor_added_removed_cb_p_1)
{
	int ret = sensord_add_sensor_added_cb(added_cb, NULL);
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

/* TODO: change it from manual test to auto-test */
TESTCASE(skip_sensor_provider, mysensor_p)
{
	int err = 0;
	sensor_t sensor;
	sensord_provider_h provider;

	err = sensord_create_provider(MYSENSOR_URI, &provider);
	ASSERT_EQ(err, 0);

	err = sensord_provider_set_name(provider, MYSENSOR_NAME);
	ASSERT_EQ(err, 0);
	err = sensord_provider_set_vendor(provider, MYSENSOR_VENDOR);
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
	err = sensord_provider_set_interval_changed_cb(provider, interval_cb, NULL);
	ASSERT_EQ(err, 0);

	err = sensord_get_default_sensor_by_uri(MYSENSOR_URI, &sensor);
	ASSERT_EQ(err, 0);

	g_timeout_add_seconds(1, publish, provider);
	mainloop::run();

	err = sensord_remove_provider(provider);
	ASSERT_EQ(err, 0);
	err = sensord_destroy_provider(provider);
	ASSERT_EQ(err, 0);

	return true;
}

/* TODO: change it from manual test to auto-test */
TESTCASE(skip_sensor_provider, mysensor_with_listener_p_1)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor_by_uri(MYSENSOR_URI, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ASSERT_EQ(err, 0);

	ret = sensord_register_event(handle, 1, 100, 100, event_cb, NULL);
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

