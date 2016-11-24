/*
 * sensorctl
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include <stdio.h>
#include <string.h>
#include <sensor_internal.h>
#include <sensorctl_log.h>
#include "tester.h"
#include "tester_manager.h"
#include "tester_sensor.h"

#define NAME_MAX_TEST 32
#define ARGC_BASE 3 /* e.g. {sensorctl, test, accelerometer} */

bool tester_manager::process(int argc, char *argv[])
{
	int option_count;
	char *options[8];
	sensor_type_t type;
	int i;

	if (argc == 2) {
		usage();
		return false;
	}

	/* 1. get sensor type */
	type = get_sensor_type(argv[2]);
	if (type == UNKNOWN_SENSOR) {
		_E("ERROR : failed to process injector\n");
		return false;
	}

	/* 2. set up injector */
	tester_interface *tester = new tester_sensor();
	tester->init();

	/* 3. test sensor with options */
	option_count = argc - ARGC_BASE;
	for (i = 0; i < option_count; ++i) {
		options[i] = new char[NAME_MAX_TEST];
		strncpy(options[i], argv[ARGC_BASE+i], sizeof(argv[ARGC_BASE+i]));
	}

	tester->test(type, option_count, options);

	return true;
}

void tester_manager::usage(void)
{
	PRINT("usage: sensorctl test <sensor_type> [interval] [event_count] [test_count]\n\n");

	usage_sensors();

	PRINT("interval_ms:\n");
	PRINT("  interval. default value is 100ms.\n");
	PRINT("event count(n):\n");
	PRINT("  test sensor until it gets n event. default is 999999(infinitly).\n");
	PRINT("test count(n):\n");
	PRINT("  test sensor in n times repetitively, default is 1.\n\n");
}

