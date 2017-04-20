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
#include <sensor_internal.h>

#include "log.h"
#include "mainloop.h"
#include "test_bench.h"
#include "sensor_adapter.h"

static void basic_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	EXPECT_GT(data->timestamp, 0);
	//_N("[   DATA   ] %f\n", data->values[0]);
	mainloop::stop();
}

TESTCASE(all_sensor_test, scenario_basic_p)
{
	int err;
	bool ret;
	int count;
	int handle;
	sensor_t *sensors;
	sensor_type_t type;

	err = sensord_get_sensors(ALL_SENSOR, &sensors, &count);
	ASSERT_EQ(err, 0);

	for (int i = 0; i < count; ++i) {
		sensord_get_type(sensors[i], &type);
		/* TODO */
		_N("[   TYPE   ] %s\n", "UNKNOWN_SENSOR");

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

typedef bool (*process_func_t)(const char *msg, int size, int count);

static pid_t run_process(process_func_t func, const char *msg, int size, int count)
{
	pid_t pid = fork();
	if (pid < 0)
		return -1;

	if (pid == 0) {
		if (!func(msg, size, count))
			_E("Failed to run process\n");
		exit(0);
	}

	return pid;
}

static bool run_echo_command_test(const char *str, int size, int cout)
{
	bool ret = true;
	int handle;
	char buf[4096] = {'1', '1', '1', };

	sensor_info info(ACCELEROMETER_SENSOR, 0,
			100, 1000, SENSOR_OPTION_ALWAYS_ON, basic_cb, NULL);
	sensor_adapter::get_handle(info, handle);

	for (int i = 0; i < 1024; ++i)
		ret &= sensor_adapter::set_attribute(handle, SENSOR_ATTR_ACCELEROMETER_INJECTION, buf, 4096);
	ASSERT_TRUE(ret);

	return true;
}

TESTCASE(echo_command_test, echo_command_p)
{
	pid_t pid;

	for (int i = 0; i < 100; ++i) {
		pid = run_process(run_echo_command_test, NULL, 0, 0);
		EXPECT_GE(pid, 0);
	}

	pid = run_process(run_echo_command_test, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	ASSERT_TRUE(true);

	return true;
}

#if 0
TESTCASE(gyroscope_value_p)
{
	scenario_basic_p(GYROSCOPE_SENSOR);
}

TESTCASE(gravitye_value_p)
{
	scenario_basic_p(GRAVITY_SENSOR);
}

TESTCASE(linear_accel_value_p)
{
	scenario_basic_p(LINEAR_ACCEL_SENSOR);
}

TESTCASE(proximity_value_p)
{
	scenario_basic_p(PROXIMITY_SENSOR);
}

TESTCASE(pressure_value_p)
{
	scenario_basic_p(PRESSURE_SENSOR);
}

TESTCASE(hrm_value_p)
{
	scenario_basic_p(HRM_SENSOR);
}

TESTCASE(hrm_raw_value_p)
{
	scenario_basic_p(HRM_RAW_SENSOR);
}

TESTCASE(hrm_led_green_value_p)
{
	scenario_basic_p(HRM_LED_GREEN_SENSOR);
}

TESTCASE(wrist_up_value_p)
{
	scenario_basic_p(GESTURE_WRIST_UP_SENSOR);
}
#endif
