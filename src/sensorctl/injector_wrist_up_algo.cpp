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
#include <sensorctl_log.h>
#include "dbus_util.h"
#include "injector_manager.h"
#include "injector_wrist_up_algo.h"

#define WRIST_UP_ALGO_SIGNAL "algo"
#define OPTION_INDEX 0

typedef std::map<std::string, int> option_map_t;
static option_map_t option_map;

bool injector_wrist_up_algo::init(void)
{
	option_map.insert(option_map_t::value_type("auto", 0));
	option_map.insert(option_map_t::value_type("green", 1));
	option_map.insert(option_map_t::value_type("purple", 2));
	option_map.insert(option_map_t::value_type("red", 3));

	return true;
}

bool injector_wrist_up_algo::inject(int option_count, char *options[])
{
	int option;

	if (option_count == 0) {
		_E("ERROR: invalid argument\n");
		return false;
	}

	option_map_t::iterator it;
	it = option_map.find(options[OPTION_INDEX]);

	if (it == option_map.end()) {
		_E("ERROR: no matched-option: %s\n", options[OPTION_INDEX]);
		return false;
	}

	option = it->second;

	dbus_emit_signal(NULL,
			(gchar *)SENSORD_OBJ_PATH,
			(gchar *)SENSORD_INTERFACE_NAME,
			(gchar *)WRIST_UP_ALGO_SIGNAL,
			g_variant_new("(i)", option),
			NULL);

	_I("set [%s] mode to wristup (%d)", options[OPTION_INDEX], option);
	return true;
}

REGISTER_INJECTOR(MOTION_SENSOR, "algo", injector_wrist_up_algo)
