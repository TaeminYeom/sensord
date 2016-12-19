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

#include "tester.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sensor_internal.h>

#include "log.h"
#include "macro.h"
#include "mainloop.h"
#include "test_bench.h"
#include "sensor_adapter.h"

#define TESTER_ARGC 3 /* e.g. {sensorctl, test, accelerometer} */

#define MAX_COUNT 999999
#define DEFAULT_INTERVAL 100
#define DEFAULT_LATENCY 0
#define DEFAULT_POWERSAVE_OPTION SENSOR_OPTION_ALWAYS_ON

static sensor_type_t type;
static int interval;
static int latency;
static int powersave;
static int repeat;

static int event_count = 0;

tester_manager::tester_manager()
{
}

tester_manager::~tester_manager()
{
}

bool tester_manager::run(int argc, char *argv[])
{
	if (argc < TESTER_ARGC) {
		usage();
		return false;
	}

	if (!setup(argc, argv)) {
		usage();
		return false;
	}

	test_bench::run_all_testcase();

	return true;
}

bool tester_manager::setup(int argc, char *argv[])
{
	if (strncmp(argv[2], "auto", 4) == 0)
		return setup_auto(argc, argv);

	return setup_manual(argc, argv);
}

bool tester_manager::setup_auto(int argc, char *argv[])
{
	if (argc > 5)
		repeat = atoi(argv[5]);

	test_option::show_full_log(true);
	test_option::set_options(argc, argv);

	return true;
}

bool tester_manager::setup_manual(int argc, char *argv[])
{
	type = get_sensor_type(argv[2]);
	RETVM_IF(type == UNKNOWN_SENSOR, false, "Invalid argument\n");

	interval = DEFAULT_INTERVAL;
	latency = DEFAULT_LATENCY;
	powersave = DEFAULT_POWERSAVE_OPTION;
	event_count = 0;

	if (argc >= TESTER_ARGC + 1)
		interval = atoi(argv[TESTER_ARGC]);
	if (argc >= TESTER_ARGC + 2)
		latency = atoi(argv[TESTER_ARGC + 1]);
	if (argc >= TESTER_ARGC + 3)
		powersave = atoi(argv[TESTER_ARGC + 2]);

	test_option::show_full_log(true);
	test_option::set_group("skip_manual");
	/* test_option::set_options(argc, argv); */

	return true;
}

void tester_manager::stop(void)
{
	if (mainloop::is_running())
		mainloop::stop();
}

void tester_manager::usage(void)
{
	_N("usage: sensorctl test auto [group] [log]\n");
	_N("usage: sensorctl test <sensor_type> [interval] [batch_latency] [event_count] [test_count]\n");

	usage_sensors();

	_N("auto:\n");
	_N("  test sensors automatically.\n");
	_N("group:\n");
	_N("  a group name(or a specific word contained in the group name).\n");
	_N("log:\n");
	_N("  enable(1) or disable(0). default value is 1.\n");
	_N("sensor_type: specific sensor\n");
	_N("  test specific sensor manually.\n");
	_N("interval_ms:\n");
	_N("  interval. default value is 100ms.\n");
	_N("batch_latency_ms:\n");
	_N("  batch_latency. default value is 1000ms.\n");
	_N("event count(n):\n");
	_N("  test sensor until it gets n event. default is 999999(infinitly).\n");
	_N("test count(n):\n");
	_N("  test sensor in n times repetitively, default is 1.\n\n");
}

/* manual test case */
static void test_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	if (event_count >= MAX_COUNT) {
		mainloop::stop();
		return;
	}

	_N("%llu ", data->timestamp);
	for (int i = 0; i < data->value_count; ++i)
		_N(" %10f", data->values[i]);
	_N("\n");
}

TESTCASE(skip_manual, sensor_test)
{
	int handle;
	bool ret;
	int index = 0;
	sensor_data_t data;

	if (sensor_adapter::get_count(type) > 1) {
		_N("There are more than 2 sensors. please enter the index : ");
		scanf("%d", &index);
	}

	sensor_info info(type, index, interval, latency, powersave, test_cb, NULL);

	ret = sensor_adapter::start(info, handle);
	ASSERT_TRUE(ret);

	ret = sensor_adapter::get_data(handle, type, data);
	EXPECT_TRUE(ret);

	mainloop::run();

	ret = sensor_adapter::stop(info, handle);
	ASSERT_TRUE(ret);

	return true;
}

