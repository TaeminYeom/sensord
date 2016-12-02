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

#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <sensorctl_log.h>

static GDBusConnection *connection;

bool dbus_init(void)
{
	GError *error = NULL;
	gchar *gaddr;

	if (connection)
		return true;

#ifndef GLIB_VERSION_2_36
	g_type_init();
#endif

	gaddr = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

	if (!gaddr) {
		PRINT("ERROR: Failed to get dbus address : %s", error->message);
		g_error_free(error);
		error = NULL;
		return false;
	}

	connection = g_dbus_connection_new_for_address_sync(gaddr,
			(GDBusConnectionFlags)(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT
			| G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
			NULL, NULL, &error);
	g_free(gaddr);

	if (!connection) {
		PRINT("ERROR: Failed to get dbus connection : %s", error->message);
		g_error_free(error);
		error = NULL;
		return false;
	}

	PRINT("G-DBUS connected[%s]\n",
			g_dbus_connection_get_unique_name(connection));
	return true;
}

bool dbus_fini(void)
{
	if (!connection)
		return true;

	g_dbus_connection_close_sync(connection, NULL, NULL);
	g_object_unref(connection);
	connection = NULL;

	return true;
}

bool dbus_emit_signal(gchar *dest_bus_name, gchar *object_path,
		gchar *interface_name, gchar *signal_name,
		GVariant *variant, GError **error)
{
	g_dbus_connection_emit_signal(connection,
				dest_bus_name,
				object_path,
				interface_name,
				signal_name,
				variant,
				error);
	return true;
}

GVariant *make_variant_int(int count, char *options[])
{
	switch (count) {
	case 1:
		return g_variant_new("(i)", atoi(options[0]));
	case 2:
		return g_variant_new("(ii)", atoi(options[0]), atoi(options[1]));
	case 3:
		return g_variant_new("(iii)", atoi(options[0]), atoi(options[1]), atoi(options[2]));
	default:
		break;
	}

	return NULL;
}

