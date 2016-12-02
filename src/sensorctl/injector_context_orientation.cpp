/*
 * sensorctl
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <glib.h>
#include <gio/gio.h>
#include <sensorctl_log.h>
#include "dbus_util.h"
#include "injector_manager.h"
#include "injector_context_orientation.h"

#define CONTEXT_ORIENTATION_SIGNAL	"orientation"

bool injector_context_orientation::inject(int option_count, char *options[])
{
	GVariant *variant;

	if (option_count == 0) {
		_E("ERROR: invalid argument\n");
		return false;
	}

	variant = make_variant_int(option_count, options);

	if (variant == NULL)
		return false;

	dbus_emit_signal(NULL,
			(gchar *)SENSORD_OBJ_PATH,
			(gchar *)SENSORD_INTERFACE_NAME,
			(gchar *)CONTEXT_ORIENTATION_SIGNAL,
			variant,
			NULL);

	PRINT("set options to context: \n");
	for (int i = 0; i < option_count; ++i)
		PRINT("option %d: %s\n", i, options[i]);

	return true;
}

REGISTER_INJECTOR(CONTEXT_SENSOR, CONTEXT_ORIENTATION_SIGNAL, injector_context_orientation)
