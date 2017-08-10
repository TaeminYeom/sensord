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

std::vector<tester *> tester_manager::testers;

tester::tester(const char *name)
: m_name(name)
{
	tester_manager::register_tester(this);
}

tester_manager::tester_manager()
{
}

tester_manager::~tester_manager()
{
}

void tester_manager::register_tester(tester *test)
{
	testers.push_back(test);
}

bool tester_manager::run(int argc, char *argv[])
{
	bool ret;
	sensor_type_t type = ACCELEROMETER_SENSOR;

	if (argc < TESTER_ARGC) {
		usage();
		return false;
	}

	if (strncmp(argv[2], "auto", 4) != 0)
		type = get_sensor_type(argv[2]);

	tester *_tester = get_tester(argv[2]);
	RETVM_IF(!_tester, false, "Cannot find matched tester\n");

	ret = _tester->setup(type, argc, argv);
	RETVM_IF(!ret, false, "Failed to setup injector\n");

	ret = _tester->run(argc, argv);
	RETVM_IF(!ret, false, "Failed to run tester\n");

	ret = _tester->teardown();
	RETVM_IF(!ret, false, "Failed to tear down tester\n");

	return true;
}

tester *tester_manager::get_tester(const char *name)
{
	int count = testers.size();

	for (int i = 0; i < count; ++i) {
		if (strncmp(name, "auto", 4) == 0) {
			if (testers[i]->name() == "auto")
				return testers[i];
		} else {
			if (testers[i]->name() == "manual")
				return testers[i];
		}
	}
	return NULL;
}

void tester_manager::stop(void)
{
	if (mainloop::is_running())
		mainloop::stop();
}

void tester_manager::usage(void)
{
	_N("usage: sensorctl test auto [options] [--help]\n");
	_N("usage: sensorctl test <sensor_type> [--help]\n");

	usage_sensors();

	_N("auto:\n");
	_N("  Run all testcases automatically.\n");
	_N("sensor_type:\n");
	_N("  Run the specific sensor manually.\n");
	_N("help: \n");
	_N("  Prints the synopsis and a list of options.\n");
}

