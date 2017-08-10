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

#include "log.h"
#include "tester.h"
#include "test_bench.h"

#define TEST_AUTO "auto"

class tester_auto : public tester {
public:
	tester_auto(const char *name);
	virtual ~tester_auto();

	bool setup(sensor_type_t type, int argc, char *argv[]);
	bool teardown(void);

	bool run(int argc, char *argv[]);
	void usage(void);
};

tester_auto::tester_auto(const char *name)
: tester(name)
{
}

tester_auto::~tester_auto()
{
}

bool tester_auto::setup(sensor_type_t type, int argc, char *argv[])
{
	test_option::filter = "(?!manual|skip)[\\w\\.]+";

	if (!test_option::set_options(argc, argv)) {
		usage();
		return false;
	}

	return true;
}

bool tester_auto::teardown(void)
{
	return true;
}

bool tester_auto::run(int argc, char *argv[])
{
	test_bench::run_all_testcases();
	return true;
}

void tester_auto::usage(void)
{
	_N("Usage : sensorctl test auto [--help] [--list] [--filter=<regex>]\n");
	_N("                            [--verbose] [--shuffle] [--repeat]\n");
	_N("                            [--output]\n");

	_N("Examples:\n");
	_N("  sensorctl test auto --list\n");
	_N("  sensorctl test auto --filter=accelerometer.interval*\n");
	_N("  sensorctl test auto --filter=accelerometer.start\n");
	_N("  sensorctl test auto --filter=accelerometer.verify\n");
	_N("  sensorctl test auto --filter=ipc.socket*\n");
	_N("  sensorctl test auto --shuffle\n");
	_N("  sensorctl test auto --verbose\n");
	_N("  sensorctl test auto --output=results.log\n");
}

REGISTER_TESTER(TEST_AUTO, tester_auto);
