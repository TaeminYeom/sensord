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

#pragma once // _DBUS_UTIL_H_

#include <glib.h>
#include <gio/gio.h>

bool dbus_init(void);
bool dbus_fini(void);
bool dbus_emit_signal(gchar *dest_bus_name, gchar *object_path,
		gchar *interface_name, gchar *signal_name,
		GVariant *variant, GError **error);
GVariant *make_variant_int(int count, char *options[]);
