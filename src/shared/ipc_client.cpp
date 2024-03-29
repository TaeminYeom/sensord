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

#include "ipc_client.h"

#include "sensor_log.h"
#include "stream_socket.h"
#include "event_handler.h"
#include "channel_event_handler.h"

using namespace ipc;

ipc_client::ipc_client(const std::string &path)
{
	m_path = path;
}

ipc_client::~ipc_client()
{
}

bool ipc_client::set_option(int option, int value)
{
	return true;
}

bool ipc_client::set_option(const std::string &option, int value)
{
	return true;
}

channel *ipc_client::connect(channel_handler *handler)
{
	return connect(handler, NULL);
}

channel *ipc_client::connect(channel_handler *handler, event_loop *loop, bool bind)
{
	socket *sock = NULL;
	channel *ch = NULL;
	channel_event_handler *ev_handler = NULL;

	sock = new(std::nothrow) stream_socket();
	retvm_if(!sock, NULL, "Failed to allocate memory");

	if (!sock->create(m_path)) {
		delete sock;
		return NULL;
	}

	ch = new(std::nothrow) channel(sock);
	if (!ch) {
		delete sock;
		_E("Failed to allocate memory");
		return NULL;
	}

	ev_handler = new(std::nothrow) channel_event_handler(ch, handler);
	if (!ev_handler) {
		delete ch;
		_E("Failed to allocate memory");
		return NULL;
	}

	uint64_t id = ch->connect(ev_handler, loop, bind);

	_D("Connected[%llu]", id);

	return ch;
}
