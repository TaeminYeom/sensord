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
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <map>
#include <string>

#include "log.h"
#include "dbus_util.h"
#include "injector.h"

#define WRISTUP_ALGO_SIGNAL "algo"
#define OPTION_INDEX 0

typedef std::map<std::string, int> option_map_t;
static option_map_t option_map;

class injector_wristup_algo : public injector {
public:
	injector_wristup_algo(sensor_type_t sensor_type, const char *event_name);
	virtual ~injector_wristup_algo() {}

	bool setup(void);
	bool inject(int argc, char *argv[]);
};

injector_wristup_algo::injector_wristup_algo(sensor_type_t sensor_type, const char *event_name)
: injector(sensor_type, event_name)
{
}

bool injector_wristup_algo::setup(void)
{
	option_map.insert(option_map_t::value_type("auto", 0));
	option_map.insert(option_map_t::value_type("green", 1));
	option_map.insert(option_map_t::value_type("purple", 2));
	option_map.insert(option_map_t::value_type("red", 3));

	return true;
}

bool injector_wristup_algo::inject(int argc, char *argv[])
{
	int option;

	RETVM_IF(argc == 0, false, "Invalid argument\n");

	option_map_t::iterator it;
	it = option_map.find(argv[INJECTOR_ARGC]);
	RETVM_IF(it == option_map.end(), false, "No matched option: %s\n", argv[INJECTOR_ARGC]);

	option = it->second;

	dbus_emit_signal(NULL,
			(gchar *)SENSORD_OBJ_PATH,
			(gchar *)SENSORD_INTERFACE_NAME,
			(gchar *)WRISTUP_ALGO_SIGNAL,
			g_variant_new("(i)", option),
			NULL);

	_I("Set up [%s] mode to wristup (%d)\n", argv[INJECTOR_ARGC], option);
	return true;
}

REGISTER_INJECTOR(GESTURE_WRIST_UP_SENSOR, WRISTUP_ALGO_SIGNAL, injector_wristup_algo)
