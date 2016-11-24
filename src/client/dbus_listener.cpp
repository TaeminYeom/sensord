/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#include <sensor_log.h>
#include "sensor_event_listener.h"
#include "dbus_listener.h"

#define HANDLE_GERROR(Err) \
	do { \
		if ((Err)) { \
			_E("GError: %s", Err->message); \
			g_error_free(Err); \
			Err = NULL; \
		} \
	} while (0)

#define ROTATION_DBUS_DEST		"org.tizen.system.coord"
#define ROTATION_DBUS_OBJ_PATH	"/Org/Tizen/System/Coord/Rotation"
#define ROTATION_DBUS_IFACE		"org.tizen.system.coord.rotation"
#define ROTATION_DBUS_SIGNAL	"Changed"
#define ROTATION_DBUS_METHOD	"Degree"

static void rotation_signal_cb(GDBusConnection *conn, const gchar *sender,
		const gchar *obj_path, const gchar *iface, const gchar *signal_name,
		GVariant *param, gpointer user_data)
{
	gint state;
	g_variant_get(param, "(i)", &state);
	sensor_event_listener::get_instance().set_display_rotation(state);
}

static void rotation_read_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	GError *error = NULL;
	GDBusConnection *conn = G_DBUS_CONNECTION(source_object);
	GVariant *result = g_dbus_connection_call_finish(conn, res, &error);
	HANDLE_GERROR(error);
	ret_if(result == NULL);

	gint state;
	g_variant_get(result, "(i)", &state);
	g_variant_unref(result);
	sensor_event_listener::get_instance().set_display_rotation(state);
}

dbus_listener::dbus_listener()
: m_connection(NULL)
{
#ifndef GLIB_VERSION_2_36
	g_type_init();
#endif
}

dbus_listener::~dbus_listener()
{
	disconnect();
}

void dbus_listener::init(void)
{
	static dbus_listener listener;
	static bool done = false;
	ret_if(done);
	listener.connect();
	done = true;
}

void dbus_listener::connect(void)
{
	GError *gerr = NULL;

	gchar *addr = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, &gerr);
	HANDLE_GERROR(gerr);
	retm_if(addr == NULL, "Getting address failed");

	g_dbus_connection_new_for_address(addr,
			(GDBusConnectionFlags)(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
			NULL, NULL, on_connection_ready, this);
	g_free(addr);
}

void dbus_listener::on_connection_ready(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	GError *gerr = NULL;
	dbus_listener *listener = static_cast<dbus_listener*>(user_data);

	GDBusConnection *conn = g_dbus_connection_new_finish(res, &gerr);
	HANDLE_GERROR(gerr);

	retm_if(conn == NULL, "Connection failed");
	_D("Dbus connection established: %s", g_dbus_connection_get_unique_name(conn));

	listener->m_connection = conn;
	listener->get_current_state();
	listener->subscribe();
}

void dbus_listener::disconnect(void)
{
	ret_if(!m_connection);
	g_dbus_connection_close_sync(m_connection, NULL, NULL);
	g_object_unref(m_connection);
}

void dbus_listener::subscribe(void)
{
	/* Diplay rotation */
	g_dbus_connection_signal_subscribe(m_connection,
			ROTATION_DBUS_DEST, ROTATION_DBUS_IFACE, ROTATION_DBUS_SIGNAL, ROTATION_DBUS_OBJ_PATH,
			NULL, G_DBUS_SIGNAL_FLAGS_NONE, (GDBusSignalCallback)rotation_signal_cb, NULL, NULL);
}

void dbus_listener::get_current_state(void)
{
	/* Display rotation */
	g_dbus_connection_call(m_connection,
			ROTATION_DBUS_DEST, ROTATION_DBUS_OBJ_PATH, ROTATION_DBUS_IFACE, ROTATION_DBUS_METHOD,
			NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback)rotation_read_cb, NULL);
}
