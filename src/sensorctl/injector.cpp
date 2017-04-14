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

#include "injector.h"

#include <stdio.h>
#include <string.h>
#include <sensor_internal.h>

#include "log.h"
#include "dbus_util.h"

std::vector<injector *> injector_manager::injectors;

injector::injector(sensor_type_t sensor_type, const char *event_name)
: m_type(sensor_type)
, m_name(event_name)
{
	injector_manager::register_injector(this);
}

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

void injector_manager::register_injector(injector *inject)
{
	injectors.push_back(inject);
}

bool injector_manager::run(int argc, char *argv[])
{
	sensor_type_t type;
	bool result;

	if (argc < INJECTOR_ARGC) {
		usage();
		return false;
	}

	/* 1. get sensor type */
	type = get_sensor_type(argv[2]);
	RETVM_IF(type == UNKNOWN_SENSOR, false, "Invalid argument : %s\n", argv[2]);

	/* 2. set up injector */
	injector *_injector = get_injector(type, argv[3]);
	RETVM_IF(!_injector, false, "Cannot find matched injector\n");

	/* 3. set up injector */
	result = _injector->setup();
	RETVM_IF(!result, false, "Failed to init injector\n");

	/* 4. inject event */
	result = _injector->inject(argc, argv);
	RETVM_IF(!result, false, "Failed to run injector\n");

	/* 5. tear down injector */
	result = _injector->teardown();
	RETVM_IF(!result, false, "Failed to tear down injector\n");

	return true;
}

injector *injector_manager::get_injector(sensor_type_t type, const char *name)
{
	int count;

	count = injectors.size();

	for (int i = 0; i < count; ++i) {
		if (type == injectors[i]->type() &&
		    (injectors[i]->name() == name))
			return injectors[i];
	}
	return NULL;
}

void injector_manager::usage(void)
{
	_N("usage: sensorctl inject <sensor_type> [<event_type>] [<options>]\n\n");

	usage_sensors();
}
