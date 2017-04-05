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

#include "server_channel_handler.h"

#include <sensor_log.h>
#include <sensor_info.h>
#include <command_types.h>

using namespace sensor;
using namespace ipc;

server_channel_handler::server_channel_handler(sensor_manager *manager)
: m_manager(manager)
{
}

server_channel_handler::~server_channel_handler()
{
}

void server_channel_handler::connected(channel *ch)
{
}

void server_channel_handler::disconnected(channel *ch)
{
}

void server_channel_handler::read(channel *ch, message &msg)
{
	int err = -EINVAL;

	switch (msg.type()) {
	case CMD_MANAGER_SENSOR_LIST:
		err = manager_get_sensor_list(ch, msg); break;
	case CMD_LISTENER_CONNECT:
		err = listener_connect(ch, msg); break;
	case CMD_LISTENER_DISCONNECT:
		err = listener_disconnect(ch, msg); break;
	case CMD_LISTENER_START:
		err = listener_start(ch, msg); break;
	case CMD_LISTENER_STOP:
		err = listener_stop(ch, msg); break;
	case CMD_LISTENER_ATTR_INT:
		err = listener_attr_int(ch, msg); break;
	case CMD_LISTENER_ATTR_STR:
		err = listener_attr_str(ch, msg); break;
	case CMD_LISTENER_GET_DATA:
		err = listener_get_data(ch, msg); break;
	case CMD_PROVIDER_CONNECT:
		err = provider_connect(ch, msg); break;
	case CMD_PROVIDER_DISCONNECT:
		err = provider_disconnect(ch, msg); break;
	case CMD_PROVIDER_POST:
		err = provider_post(ch, msg); break;
	default: break;
	}

	if (err != 0) {
		message reply(err);
		ch->send_sync(&reply);
	}
}

int server_channel_handler::manager_get_sensor_list(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::listener_connect(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::listener_disconnect(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::listener_start(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::listener_stop(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::listener_attr_int(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::listener_attr_str(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::listener_get_data(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::provider_connect(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::provider_disconnect(channel *ch, message &msg)
{
	return OP_ERROR;
}

int server_channel_handler::provider_post(channel *ch, message &msg)
{
	return OP_ERROR;
}
