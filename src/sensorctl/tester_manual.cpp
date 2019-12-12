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

#include <iostream>
#include <getopt.h>

#include "tester.h"
#include "mainloop.h"
#include "log.h"
#include "test_bench.h"
#include "sensor_adapter.h"

#define MAX_COUNT 999999
#define TEST_DEFAULT_INTERVAL 100
#define TEST_DEFAULT_LATENCY 0
#define TEST_DEFAULT_POWERSAVE_OPTION SENSOR_OPTION_ALWAYS_ON

#define TEST_MANUAL "manual"

static sensor_type_t stype;
static int interval;
static int latency;
static int powersave;
//static int repeat;

class tester_manual : public tester {
public:
	tester_manual(const char *name);
	virtual ~tester_manual();

	bool setup(sensor_type_t type, int argc, char *argv[]);
	bool teardown(void);

	bool run(int argc, char *argv[]);
	void usage(void);
};

tester_manual::tester_manual(const char *name)
: tester(name)
{
}

tester_manual::~tester_manual()
{
}

bool tester_manual::setup(sensor_type_t type, int argc, char *argv[])
{
	stype = type;
	interval = TEST_DEFAULT_INTERVAL;
	latency = TEST_DEFAULT_LATENCY;
	powersave = TEST_DEFAULT_POWERSAVE_OPTION;

	if (!test_option::set_options(argc, argv)) {
		usage();
		return false;
	}

	interval = test_option::interval;
	latency = test_option::latency;
	powersave = test_option::powersave;

	if (interval == -1)
		interval = TEST_DEFAULT_INTERVAL;
	if (latency == -1)
		latency = TEST_DEFAULT_LATENCY;
	if (powersave == -1)
		powersave = TEST_DEFAULT_POWERSAVE_OPTION;

	return true;
}

bool tester_manual::teardown(void)
{
	return true;
}

bool tester_manual::run(int argc, char *argv[])
{
	test_option::filter = "manual_test.sensor";
	test_bench::run_all_testcases();

	return true;
}

void tester_manual::usage(void)
{
	_N("Usage : sensorctl test <sensor_type>\n");
	_N("          [--interval=NUMBER] [--batch_latency=NUMBER] [--powersave=TYPE]\n");
	_N("          [--repeat=NUMBER] [--output=FILE_PATH] [--help] [--verbose]\n");

	_N("Examples:\n");
	_N("  sensorctl test accelerometer\n");
	_N("  sensorctl test accelerometer --interval=100 --batch_latency=1000\n");
	_N("  sensorctl test accelerometer --i 100 --b 1000 --p 0\n");
	_N("  sensorctl test accelerometer --i 100 --shuffle\n");
	_N("  sensorctl test accelerometer --i 100 --verbose\n");
	_N("  sensorctl test accelerometer --i 100 --output=results.log\n");
}

static void test_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	_N("%llu ", data->timestamp);
	for (int i = 0; i < data->value_count; ++i)
		_N(" %10f", data->values[i]);
	_N("\n");
}

static void test_events_cb(sensor_t sensor, unsigned int event_type, sensor_data_t datas[], int events_count, void *user_data)
{
	for (int i = 0 ; i < events_count; i++) {
		_N("%llu ", datas[i].timestamp);
		for (int j = 0; j < datas[i].value_count; j++)
			_N(" %10f", datas[i].values[j]);
		_N("\n");
	}
}

TESTCASE(manual_test, sensor)
{
	int handle;
	bool ret;
	int index = 0;
	sensor_data_t data;
	sensor_info info;

	if (sensor_adapter::get_count(stype) > 1) {
		_N("There are more than 2 sensors. please enter the index : ");
		std::cin >> index;
	}

	if (sensor_adapter::is_batch_mode) {
		info = sensor_info(stype, index, interval, latency, powersave, test_events_cb, NULL);
	} else {
		info = sensor_info(stype, index, interval, latency, powersave, test_cb, NULL);
	}

	ret = sensor_adapter::start(info, handle);
	ASSERT_TRUE(ret);

	ret = sensor_adapter::get_data(handle, stype, data);
	EXPECT_TRUE(ret);

	mainloop::run();

	ret = sensor_adapter::stop(info, handle);
	ASSERT_TRUE(ret);

	return true;
}

REGISTER_TESTER(TEST_MANUAL, tester_manual);
