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

#include "log.h"
#include "mainloop.h"
#include "test_bench.h"


typedef Deleter<char> Deleter_char;
static bool called = false;
static int count = 0;

static void event_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	_I("[%llu] %f %f %f\n", data->timestamp, data->values[0], data->values[1], data->values[2]);

	if (count++ > 3)
		mainloop::stop();
}

TESTCASE(sensor_listener, get_default_sensor_p_1)
{
	int err;
	sensor_t sensor;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	return true;
}

TESTCASE(sensor_listener, get_sensors_p_1)
{
	int err;
	int count;
	sensor_t *sensors = nullptr;

	err = sensord_get_sensors(ACCELEROMETER_SENSOR, &sensors, &count);
	ASSERT_EQ(err, 0);
	ASSERT_FREE((count < 0), sensors);
	ASSERT_GT(count, 0);

	free(sensors);

	return true;
}

TESTCASE(sensor_listener, connect_p_1)
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

TESTCASE(sensor_listener, all_api_p_1)
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

	ret = sensord_register_event(handle, 1, 100, 100, event_cb, NULL);
	ASSERT_FREE(((ret != true) && list), list);
	ASSERT_TRUE(ret);

	ret = sensord_start(handle, 0);
	ASSERT_FREE(((ret != true) && list), list);
	ASSERT_TRUE(ret);

	ret = sensord_change_event_interval(handle, 0, 100);
	ASSERT_FREE(((ret != true) && list), list);
	ASSERT_TRUE(ret);

	ret = sensord_change_event_max_batch_latency(handle, 0, 100);
	ASSERT_FREE(((ret != true) && list), list);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensord_stop(handle);
	ASSERT_FREE(((ret != true) && list), list);
	ASSERT_TRUE(ret);

	ret = sensord_unregister_event(handle, 1);
	ASSERT_FREE(((ret != true) && list), list);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_FREE(((ret != true) && list), list);
	ASSERT_TRUE(ret);

	free(list);

	return true;
}

TESTCASE(sensor_listener, bad_unregister_stop_order_p_1)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ret = sensord_register_event(handle, 1, 100, 100, event_cb, NULL);
	ret = sensord_start(handle, 0);
	ret = sensord_change_event_interval(handle, 0, 100);

	mainloop::run();

	/* [TEST] Unregister event before stop */
	ret = sensord_unregister_event(handle, 1);
	ASSERT_TRUE(ret);

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, bad_disconnect_p_1)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ret = sensord_register_event(handle, 1, 100, 100, event_cb, NULL);
	ret = sensord_start(handle, 0);
	ret = sensord_change_event_interval(handle, 0, 100);

	mainloop::run();

	/* [TEST] Unregistering event is not called */

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, bad_disconnect_p_2)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ret = sensord_register_event(handle, 1, 100, 100, event_cb, NULL);
	ret = sensord_start(handle, 0);
	ret = sensord_change_event_interval(handle, 0, 100);

	mainloop::run();

	ret = sensord_unregister_event(handle, 1);
	ASSERT_TRUE(ret);

	/* [TEST] stopping sensor is not called */

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, set_get_attribute_int_1)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	sensor_t sensor = NULL;
	int attr = 1;
	int value = -1;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	err = sensord_set_attribute_int(handle, attr, 1);
	ASSERT_EQ(err, 0);

	err = sensord_get_attribute_int(handle, attr, &value);
	ASSERT_EQ(err, 0);

	ASSERT_EQ(value, 1);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, set_get_attribute_int_2)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	sensor_t sensor = NULL;
	int attr = 20;
	int value = -1;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	err = sensord_set_attribute_int(handle, attr, 1);
	ASSERT_EQ(err, 0);

	err = sensord_get_attribute_int(handle, attr, &value);
	ASSERT_EQ(err, 0);

	ASSERT_EQ(value, 1);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, set_get_attribute_int_3)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	sensor_t sensor = NULL;
	int attr = 20;
	int value = -1;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);

	err = sensord_set_attribute_int(handle, attr, 1);
	ASSERT_EQ(err, 0);

	for (int i = 0 ; i < 10; i ++) {
		err = sensord_get_attribute_int(handle, attr, &value);
		ASSERT_EQ(err, 0);
	}

	ASSERT_EQ(value, 1);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, get_attribute_int_1)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	sensor_t sensor = NULL;
	int attr = 100;
	int value = -1;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);

	// attr 100 value is never set in these tests.
	err = sensord_get_attribute_int(handle, attr, &value);
	ASSERT_EQ(err, -5);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

#define TEST_STRING "TESTTESTTEST"
#define TEST_STRING_LEN 13

TESTCASE(sensor_listener, set_attribute_string_1)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	sensor_t sensor = NULL;
	int attr = 1;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	err = sensord_set_attribute_str(handle, attr, TEST_STRING, TEST_STRING_LEN);
	ASSERT_EQ(err, 0);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, set_get_attribute_string_1)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	Deleter_char value;
	int len = 0;
	sensor_t sensor = NULL;
	int attr = 1;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	err = sensord_set_attribute_str(handle, attr, TEST_STRING, TEST_STRING_LEN);
	ASSERT_EQ(err, 0);

	err = sensord_get_attribute_str(handle, attr, &(value.get()), &len);
	ASSERT_EQ(err, 0);
	ASSERT_EQ(len, TEST_STRING_LEN);
	ASSERT_EQ(strncmp(value.get(), TEST_STRING, len), 0);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

#define BUF_SIZE 4000
TESTCASE(sensor_listener, set_get_attribute_string_2)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	Deleter_char value;
	int len = 0;
	sensor_t sensor = NULL;
	int attr = 1;
	char attr_value[BUF_SIZE] = {1, };
	attr_value[BUF_SIZE - 1] = 1;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	err = sensord_set_attribute_str(handle, attr, attr_value, BUF_SIZE);
	ASSERT_EQ(err, 0);

	err = sensord_get_attribute_str(handle, attr, &(value.get()), &len);
	ASSERT_EQ(err, 0);
	ASSERT_EQ(len, BUF_SIZE);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, set_get_attribute_string_3)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	Deleter_char value;
	int len = 0;
	sensor_t sensor = NULL;
	int attr = 1;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	err = sensord_set_attribute_str(handle, attr, TEST_STRING, TEST_STRING_LEN);
	ASSERT_EQ(err, 0);

	for (int i = 0; i < 10; i++) {
		err = sensord_get_attribute_str(handle, attr, &(value.get()), &len);
		ASSERT_EQ(err, 0);
		ASSERT_EQ(len, TEST_STRING_LEN);
		ASSERT_EQ(strncmp(value.get(), TEST_STRING, len), 0);
	}

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, set_get_get_attribute_string_1)
{
	int err;
	bool ret;
	int handle;
	Deleter_char value;
	Deleter_char value2;
	int len = 0;
	sensor_t sensor;
	int attr = 1;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	err = sensord_set_attribute_str(handle, attr, TEST_STRING, TEST_STRING_LEN);
	ASSERT_EQ(err, 0);

	err = sensord_get_attribute_str(handle, attr, &(value.get()), &len);
	ASSERT_EQ(err, 0);
	ASSERT_EQ(len, TEST_STRING_LEN);
	ASSERT_EQ(strncmp(value.get(), TEST_STRING, len), 0);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	len = 0;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);

	err = sensord_get_attribute_str(handle, attr, &(value2.get()), &len);
	ASSERT_EQ(err, 0);
	ASSERT_EQ(len, TEST_STRING_LEN);
	ASSERT_EQ(strncmp(value2.get(), TEST_STRING, len), 0);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(sensor_listener, get_attribute_string_2)
{
	int err;
	bool ret;
	int handle;
	Deleter_char value;
	int len;
	sensor_t sensor;
	int attr = 100;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);

	// attr 100 value is never set in these tests.
	err = sensord_get_attribute_str(handle, attr, &(value.get()), &len);
	ASSERT_EQ(err, -EIO);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

#define SENSOR_SHIFT_TYPE 16
TESTCASE(sensor_listener, get_data_list)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;
	sensor_type_t type;

	called = false;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
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

void sensor_attribute_int_changed_callback(sensor_t sensor, int attribute, int value, void *data)
{
	_I("[ATTRIBUTE INT CHANGED] attribute : %d, value : %d\n", attribute, value);
}

TESTCASE(skip_sensor_listener, register_attribute_int_changed)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ASSERT_EQ(err, 0);

	ret = sensord_register_attribute_int_changed_cb(handle, sensor_attribute_int_changed_callback, NULL);
	ASSERT_TRUE(ret);

	ret = sensord_start(handle, 0);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_unregister_attribute_int_changed_cb(handle);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

static int attribute = 1; // 1 is SENSOR_ATTRIBUTE_AXIS_ORIENTATION of sensor_attribute_e in sensor.h
static int attribute_value = 0;

static gboolean change_attribute_int(gpointer gdata)
{
	int *handle = reinterpret_cast<int *>(gdata);

	sensord_set_attribute_int(*handle, attribute, attribute_value);

	_N("[ SET ATTRIBUTE INT ] attribute %d, value : %d\n", attribute, attribute_value);

	g_timeout_add_seconds(1, change_attribute_int, handle);

	attribute_value ? attribute_value = 0 : attribute_value = 1;

	return FALSE;
}

TESTCASE(skip_sensor_listener, attribute_int_changer)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ASSERT_EQ(err, 0);

	ret = sensord_start(handle, 0);
	ASSERT_TRUE(ret);

	g_timeout_add_seconds(1, change_attribute_int, &handle);
	mainloop::run();

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

void sensor_attribute_str_changed_callback(sensor_t sensor, int attribute, const char *value, int len, void *data)
{
	_I("[ATTRIBUTE STR CHANGED] attribute : %d, value : %s, len : %d\n", attribute, value, len);
}

TESTCASE(skip_sensor_listener, register_attribute_str_changed)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ASSERT_EQ(err, 0);

	ret = sensord_register_attribute_str_changed_cb(handle, sensor_attribute_str_changed_callback, NULL);
	ASSERT_TRUE(ret);

	ret = sensord_start(handle, 0);
	ASSERT_TRUE(ret);

	mainloop::run();

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_unregister_attribute_str_changed_cb(handle);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

static const char *attribute_value_str1 = "test_str_1";
static const char *attribute_value_str2 = "test_str_2";
static const char *attribute_value_str = attribute_value_str1;

static gboolean change_attribute_str(gpointer gdata)
{
	int *handle = reinterpret_cast<int *>(gdata);
	int len = strlen(attribute_value_str) + 1;
	sensord_set_attribute_str(*handle, attribute, attribute_value_str, len);

	_N("[ SET ATTRIBUTE STR ] attribute %d, value : %s, len : %d\n", attribute, attribute_value_str, len);

	g_timeout_add_seconds(1, change_attribute_str, handle);

	if (attribute_value_str == attribute_value_str1) {
		attribute_value_str = attribute_value_str2;
	} else {
		attribute_value_str = attribute_value_str1;
	}

	return FALSE;
}

TESTCASE(skip_sensor_listener, attribute_str_changer)
{
	int err;
	bool ret;
	int handle;
	sensor_t sensor;

	called = false;

	err = sensord_get_default_sensor(ACCELEROMETER_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	ASSERT_EQ(err, 0);

	ret = sensord_start(handle, 0);
	ASSERT_TRUE(ret);

	g_timeout_add_seconds(1, change_attribute_str, &handle);
	mainloop::run();

	ret = sensord_stop(handle);
	ASSERT_TRUE(ret);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(skip_sensor_listener, light_sensor_set_attribute_int_1)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	sensor_t sensor = NULL;
	int attr = 2;
	int value = 0;

	err = sensord_get_default_sensor(LIGHT_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	err = sensord_set_attribute_int(handle, attr, 2);
	ASSERT_EQ(err, 0);

	err = sensord_get_attribute_int(handle, attr, &value);
	ASSERT_EQ(err, 0);
	ASSERT_EQ(value, 2);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(skip_sensor_listener, light_sensor_get_attribute_int_1)
{
	int err = 0;
	bool ret = true;
	int handle = 0;
	sensor_t sensor = NULL;
	int attr = 2;
	int value = 0;

	err = sensord_get_default_sensor(LIGHT_SENSOR, &sensor);
	ASSERT_EQ(err, 0);

	handle = sensord_connect(sensor);
	err = sensord_get_attribute_int(handle, attr, &value);
	ASSERT_EQ(err, 0);
	ASSERT_EQ(value, 2);

	ret = sensord_disconnect(handle);
	ASSERT_TRUE(ret);

	return true;
}
