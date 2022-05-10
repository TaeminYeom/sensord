/*
 * sensord
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

#ifndef __SENSOR_MANAGER_CHANNEL_HANDLER__
#define __SENSOR_MANAGER_CHANNEL_HANDLER__

#include <sensor_internal.h>
#include <sensor_manager.h>
#include <channel_handler.h>
#include <map>

namespace sensor {

class sensor_manager::channel_handler : public ipc::channel_handler
{
public:
	channel_handler(sensor_manager *manager);

	void connected(ipc::channel *ch);
	void disconnected(ipc::channel *ch);
	void read(ipc::channel *ch, ipc::message &msg);
	void read_complete(ipc::channel *ch);
	void error_caught(ipc::channel *ch, int error);

	void on_sensor_added(ipc::channel *ch, ipc::message &msg);
	void on_sensor_removed(ipc::channel *ch, ipc::message &msg);

	void add_sensor_added_cb(sensord_added_cb cb, void *user_data);
	void remove_sensor_added_cb(sensord_added_cb cb);

	void add_sensor_removed_cb(sensord_removed_cb cb, void *user_data);
	void remove_sensor_removed_cb(sensord_removed_cb cb);

	void set_handler(int num, ipc::channel_handler* handler) {}
	void disconnect(void) {}

private:
	typedef std::map<sensord_added_cb, void *> sensor_added_cb_list_t;
	typedef std::map<sensord_removed_cb, void *> sensor_removed_cb_list_t;

	sensor_manager *m_manager;
	sensor_added_cb_list_t m_sensor_added_callbacks;
	sensor_removed_cb_list_t m_sensor_removed_callbacks;
};

}

#endif /* __SENSOR_MANAGER_CHANNEL_HANDLER__ */
