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

#ifndef __SERVER_CHANNEL_HANDLER_H__
#define __SERVER_CHANNEL_HANDLER_H__

#include <channel.h>
#include <channel_handler.h>
#include <unordered_map>
#include <functional>

#include "sensor_manager.h"
#include "sensor_listener_proxy.h"
#include "application_sensor_handler.h"

namespace sensor {

class server_channel_handler : public ipc::channel_handler
{
public:
	server_channel_handler(sensor_manager *manager);
	~server_channel_handler();

	void connected(ipc::channel *ch);
	void disconnected(ipc::channel *ch);
	void read(ipc::channel *ch, ipc::message &msg);
	void read_complete(ipc::channel *ch) {}
	void error_caught(ipc::channel *ch, int error) {}

private:
	int manager_connect(ipc::channel *ch, ipc::message &msg);
	int manager_disconnect(ipc::channel *ch, ipc::message &msg);
	int manager_get_sensor_list(ipc::channel *ch, ipc::message &msg);


	int listener_connect(ipc::channel *ch, ipc::message &msg);
	int listener_disconnect(ipc::channel *ch, ipc::message &msg);
	int listener_start(ipc::channel *ch, ipc::message &msg);
	int listener_stop(ipc::channel *ch, ipc::message &msg);
	int listener_attr_int(ipc::channel *ch, ipc::message &msg);
	int listener_attr_str(ipc::channel *ch, ipc::message &msg);
	int listener_get_data(ipc::channel *ch, ipc::message &msg);

	int provider_connect(ipc::channel *ch, ipc::message &msg);
	int provider_disconnect(ipc::channel *ch, ipc::message &msg);
	int provider_publish(ipc::channel *ch, ipc::message &msg);

	int has_privileges(ipc::channel *ch, ipc::message &msg);

	bool has_privilege(int fd, std::string &priv);
	bool has_privileges(int fd, std::string priv);

	int send_reply(ipc::channel *ch, int error);

	sensor_manager *m_manager;

	/* {id, listener} */
	static std::unordered_map<uint32_t, sensor_listener_proxy *> m_listeners;

	/* {channel, id} */
	static std::unordered_map<ipc::channel *, uint32_t> m_listener_ids;

	/* {channel, application_sensor_handler} */
	/* it should move to sensor_manager */
	static std::unordered_map<ipc::channel *, application_sensor_handler *> m_app_sensors;
};

}

#endif /* __SERVER_CHANNEL_HANDLER_H__ */
