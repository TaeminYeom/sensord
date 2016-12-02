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
#include "dbus_util.h"
#include "injector.h"
#include "injector_manager.h"

#define NAME_MAX_TEST 32
#define ARGC_BASE 4 /* e.g. {sensorctl, inject, wristup, conf} */

static std::vector<injector_info> injector_infos;

injector_manager::injector_manager()
{
	if (!dbus_init()) {
		_E("Failed to init dbus");
		throw;
	}
}

injector_manager::~injector_manager()
{
	dbus_fini();
}

bool injector_manager::register_injector(injector_info info)
{
	injector_infos.push_back(info);
	return true;
}

injector_interface *injector_manager::get_injector(sensor_type_t type, const char *name)
{
	int injector_count;
	injector_count = injector_infos.size();

	for (int i = 0; i < injector_count; ++i) {
		if (type == injector_infos[i].type &&
		    !strcmp(injector_infos[i].name, name))
			return injector_infos[i].injector;
	}
	return NULL;
}

bool injector_manager::process(int argc, char *argv[])
{
	int option_count;
	char *options[8];
	bool result;
	sensor_type_t type;
	char *event_name;
	int i;

	if (argc < 4) {
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
	event_name = argv[3];

	injector_interface *injector = get_injector(type, event_name);
	if (injector == NULL) {
		_E("ERROR: cannot find matched injector\n");
		return false;
	}

	/* 3. init injector */
	result = injector->init();
	if (!result) {
		_E("ERROR: failed to init injector\n");
		return false;
	}

	/* 4. inject event with options */
	option_count = argc - ARGC_BASE;
	for (i = 0; i < option_count; ++i) {
		options[i] = new char[NAME_MAX_TEST];
		strncpy(options[i], argv[ARGC_BASE + i], strlen(argv[ARGC_BASE + i]));
	}

	result = injector->inject(option_count, options);
	if (!result) {
		_E("ERROR : failed to process injector\n");
		for (i = 0; i < option_count; ++i)
			delete [] options[i];

		return false;
	}

	for (i = 0; i < option_count; ++i)
		delete [] options[i];

	return true;
}

void injector_manager::usage(void)
{
	PRINT("usage: sensorctl inject <sensor_type> [<event_type>] [<options>]\n\n");

	usage_sensors();
}

