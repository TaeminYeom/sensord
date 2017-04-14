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

#ifndef _DBUS_LISTENER_H_
#define _DBUS_LISTENER_H_

#include <glib.h>
#include <gio/gio.h>

class dbus_listener {
public:
	static void init(void);

private:
	GDBusConnection *m_connection;

	dbus_listener();
	~dbus_listener();

	void connect(void);
	void disconnect(void);

	void subscribe(void);
	void get_current_state(void);

	static void on_connection_ready(GObject *source_object, GAsyncResult *res, gpointer user_data);
};

#endif /* _DBUS_LISTENER_H_ */
