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

#include "sensor_manager_channel_handler.h"

#include <sensor_log.h>
#include <command_types.h>
#include "sensor_manager.h"

using namespace sensor;

sensor_manager::channel_handler::channel_handler(sensor_manager *manager)
: m_manager(manager)
{
}

void sensor_manager::channel_handler::connected(ipc::channel *ch)
{
}

void sensor_manager::channel_handler::disconnected(ipc::channel *ch)
{
	/* If channel->disconnect() is not explicitly called, it will be restored */
	m_manager->restore();
}

void sensor_manager::channel_handler::read(ipc::channel *ch, ipc::message &msg)
{
	switch (msg.header()->type) {
	case CMD_MANAGER_SENSOR_ADDED:
		on_sensor_added(ch, msg);
		break;
	case CMD_MANAGER_SENSOR_REMOVED:
		on_sensor_removed(ch, msg);
		break;
	}
}

void sensor_manager::channel_handler::read_complete(ipc::channel *ch)
{
}

void sensor_manager::channel_handler::error_caught(ipc::channel *ch, int error)
{
}

void sensor_manager::channel_handler::on_sensor_added(ipc::channel *ch, ipc::message &msg)
{
	ret_if(msg.header()->err < OP_SUCCESS);

	sensor_info info;
	info.clear();
	info.deserialize(msg.body(), msg.size());

	m_manager->add_sensor(info);

	auto it = m_sensor_added_callbacks.begin();
	while (it != m_sensor_added_callbacks.end()) {
		it->first(info.get_uri().c_str(), it->second);
		++it;
	}
}

void sensor_manager::channel_handler::on_sensor_removed(ipc::channel *ch, ipc::message &msg)
{
	ret_if(msg.header()->err < 0);
	char uri[NAME_MAX] = {0, };

	msg.disclose(uri);
	m_manager->remove_sensor(uri);

	auto it = m_sensor_removed_callbacks.begin();
	while (it != m_sensor_removed_callbacks.end()) {
		it->first(uri, it->second);
		++it;
	}
}

void sensor_manager::channel_handler::add_sensor_added_cb(sensord_added_cb cb, void *user_data)
{
	m_sensor_added_callbacks.emplace(cb, user_data);
}

void sensor_manager::channel_handler::remove_sensor_added_cb(sensord_added_cb cb)
{
	m_sensor_added_callbacks.erase(cb);
}

void sensor_manager::channel_handler::add_sensor_removed_cb(sensord_removed_cb cb, void *user_data)
{
	m_sensor_removed_callbacks.emplace(cb, user_data);
}

void sensor_manager::channel_handler::remove_sensor_removed_cb(sensord_removed_cb cb)
{
	m_sensor_removed_callbacks.erase(cb);
}
