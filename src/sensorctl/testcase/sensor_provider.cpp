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
#define MYSENSOR_BATCH_URI "http://example.org/sensor/general/mysensor/mysensor-batch"

#define MYSENSOR_NAME "mysensor"
#define MYSENSOR_BATCH_NAME "mysensor-batch"
#define MYSENSOR_VENDOR "tizen"

#define NUMBER_OF_EVENT 100

static bool started = false;
static bool added = false;
static bool called = false;

static void event_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	_I("[%llu] %f %f %f\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

static void events_cb(sensor_t sensor, unsigned int event_type, sensor_data_t datas[], int events_count, void *user_data)
{
	for (int i = 0 ; i < events_count; i++) {
		_I("[%llu]", datas[i].timestamp);
		for (int j = 0; j < datas[i].value_count; j++)
			_I(" %f", datas[i].values[j]);
		_I("\n");
	}
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

static gboolean publish_batch_event(gpointer gdata)
{
	if (!started) {
		_N("[ WAITING ] ...\n");
		return TRUE;
	}
	sensord_provider_h *provider = reinterpret_cast<sensord_provider_h *>(gdata);

	sensor_data_t data[NUMBER_OF_EVENT];

	for (int i = 0 ; i < NUMBER_OF_EVENT; i++) {
		data[i].accuracy = 3;
		data[i].timestamp = sensor::utils::get_timestamp();
		data[i].value_count = 3;
		data[i].values[0] = i;
		data[i].values[1] = i;
		data[i].values[2] = i;
	}
	sensord_provider_publish_events(provider, data, NUMBER_OF_EVENT);
	_N("[ PUBLISH ] %d events\n", NUMBER_OF_EVENT);
	g_timeout_add_seconds(1, publish_batch_event, provider);
	return FALSE;
}

static void add_mysensor(void)
{
	int err = 0;
	sensord_provider_h provider = nullptr;

	err = sensord_create_provider(MYSENSOR_URI, &provider);
	EXPECT_EQ(err, 0);
	err = sensord_provider_set_name(provider, MYSENSOR_NAME);
	EXPECT_EQ(err, 0);
	err = sensord_provider_set_vendor(provider, MYSENSOR_VENDOR);
	EXPECT_EQ(err, 0);
	err = sensord_provider_set_range(provider, 0.0f, 1.0f);
	EXPECT_EQ(err, 0);
	err = sensord_provider_set_resolution(provider, 0.01f);
	EXPECT_EQ(err, 0);
	err = sensord_add_provider(provider);
	EXPECT_EQ(err, 0);
	err = sensord_remove_provider(provider);
	EXPECT_EQ(err, 0);
	err = sensord_destroy_provider(provider);
	EXPECT_EQ(err, 0);
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

/* TODO: change it from manual test to auto-test */
TESTCASE(skip_sensor_provider, mysensor_batch_p)
{
	int err = 0;
	sensor_t sensor;
	sensord_provider_h provider;

	err = sensord_create_provider(MYSENSOR_BATCH_URI, &provider);
	ASSERT_EQ(err, 0);

	err = sensord_provider_set_name(provider, MYSENSOR_BATCH_NAME);
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

	err = sensord_get_default_sensor_by_uri(MYSENSOR_BATCH_URI, &sensor);
	ASSERT_EQ(err, 0);

	g_timeout_add_seconds(1, publish_batch_event, provider);
	mainloop::run();

	err = sensord_remove_provider(provider);
	ASSERT_EQ(err, 0);
	err = sensord_destroy_provider(provider);
	ASSERT_EQ(err, 0);

	return true;
}


/* TODO: change it from manual test to auto-test */
TESTCASE(skip_sensor_provider, mysensor_batch_with_listener_p_1)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor_by_uri(MYSENSOR_BATCH_URI, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ASSERT_EQ(err, 0);

	ret = sensord_register_events(handle, 1, 100, events_cb, NULL);
	ASSERT_TRUE(ret);

	ret = sensord_start(handle, 0);
	ASSERT_TRUE(ret);

	ret = sensord_change_event_interval(handle, 0, 100);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_unregister_events(handle, 1);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(skip_sensor_provider, mysensor_batch_events_once)
{
	int err = 0;
	bool ret = false;
	sensor_t sensor;
	sensord_provider_h provider;

	err = sensord_create_provider(MYSENSOR_BATCH_URI, &provider);
	ASSERT_EQ(err, 0);

	err = sensord_provider_set_name(provider, MYSENSOR_BATCH_NAME);
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

	err = sensord_get_default_sensor_by_uri(MYSENSOR_BATCH_URI, &sensor);
	ASSERT_EQ(err, 0);

	int client_handle;
	sensor_t client_sensor;
	err = sensord_get_default_sensor_by_uri(MYSENSOR_BATCH_URI, &client_sensor);
	ASSERT_EQ(err, 0);
	client_handle = sensord_connect(client_sensor);
	ASSERT_EQ(err, 0);

	ret = sensord_start(client_handle, 0);
	ASSERT_TRUE(ret);

	sensor_data_t data[NUMBER_OF_EVENT];
	for (int i = 0 ; i < NUMBER_OF_EVENT; i++) {
		data[i].accuracy = 3;
		data[i].timestamp = sensor::utils::get_timestamp();
		data[i].value_count = 3;
		data[i].values[0] = i;
		data[i].values[1] = i;
		data[i].values[2] = i;
	}
	err = sensord_provider_publish_events(provider, data, NUMBER_OF_EVENT);
	ASSERT_EQ(err, 0);

	ret = sensord_stop(client_handle);
	ASSERT_TRUE(ret);
	ret = sensord_disconnect(client_handle);
	ASSERT_TRUE(ret);

	mainloop::run();

	err = sensord_remove_provider(provider);
	ASSERT_EQ(err, 0);
	err = sensord_destroy_provider(provider);
	ASSERT_EQ(err, 0);

	return true;
}

TESTCASE(skip_sensor_provider, mysensor_batch_p_without_publish)
{
	int err = 0;
	sensor_t sensor;
	sensord_provider_h provider;

	err = sensord_create_provider(MYSENSOR_BATCH_URI, &provider);
	ASSERT_EQ(err, 0);

	err = sensord_provider_set_name(provider, MYSENSOR_BATCH_NAME);
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

	err = sensord_get_default_sensor_by_uri(MYSENSOR_BATCH_URI, &sensor);
	ASSERT_EQ(err, 0);

	mainloop::run();

	err = sensord_remove_provider(provider);
	ASSERT_EQ(err, 0);
	err = sensord_destroy_provider(provider);
	ASSERT_EQ(err, 0);

	return true;
}

#define SENSOR_SHIFT_TYPE 16
TESTCASE(skip_sensor_provider, mysensor_get_data_list)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;
	sensor_type_t type;

	called = false;

	err = sensord_get_default_sensor_by_uri(MYSENSOR_BATCH_URI, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);

	sensord_get_type(sensor, &type);
	ASSERT_EQ(err, 0);

	ret = sensord_start(handle, 0);
	ASSERT_TRUE(ret);

	sensor_data_t* data_list = NULL;
	int count = 0;
	unsigned int data_id = type << SENSOR_SHIFT_TYPE | 0x1;

	ret = sensord_get_data_list(handle, data_id, &data_list, &count);
	ASSERT_TRUE(ret);
	ASSERT_EQ(count, NUMBER_OF_EVENT);

	for (int i = 0 ; i < count; i++) {
		_I("[%llu]", data_list[i].timestamp);
		for (int j = 0; j < data_list[i].value_count; j++)
			_I(" %f", data_list[i].values[j]);
		_I("\n");
	}
	free(data_list);

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_unregister_events(handle, 1);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(skip_sensor_provider, mysensor_get_data)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;
	sensor_type_t type;

	called = false;

	err = sensord_get_default_sensor_by_uri(MYSENSOR_BATCH_URI, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);

	sensord_get_type(sensor, &type);
	ASSERT_EQ(err, 0);

	ret = sensord_start(handle, 0);
	ASSERT_TRUE(ret);

	sensor_data_t data ;
	unsigned int data_id = type << SENSOR_SHIFT_TYPE | 0x1;

	ret = sensord_get_data(handle, data_id, &data);
	ASSERT_TRUE(ret);

	_I("[%llu]", data.timestamp);
	for (int j = 0; j < data.value_count; j++)
		_I(" %f", data.values[j]);
	_I("\n");

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_unregister_events(handle, 1);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}