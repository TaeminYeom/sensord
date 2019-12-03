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

#include "ipc_server.h"

#include "channel.h"
#include "sensor_log.h"
#include "event_loop.h"
#include "channel_event_handler.h"
#include "accept_event_handler.h"

using namespace ipc;

#define MAX_CONNECTIONS 1000

ipc_server::ipc_server(const std::string &path)
: m_event_loop(NULL)
, m_handler(NULL)
, m_accept_handler(NULL)
{
	m_accept_sock.create(path);
}

ipc_server::~ipc_server()
{
}

bool ipc_server::set_option(int option, int value)
{
	/* TODO */
	return true;
}

bool ipc_server::set_option(const std::string &option, int value)
{
	/* TODO */
	return true;
}

void ipc_server::accept(ipc::socket &cli_sock)
{
	m_accept_sock.accept(cli_sock);

	_D("Accepted[%d]", cli_sock.get_fd());
}

bool ipc_server::bind(channel_handler *handler, event_loop *loop)
{
	m_handler = handler;
	m_event_loop = loop;

	m_accept_sock.bind();
	m_accept_sock.listen(MAX_CONNECTIONS);

	register_acceptor();

	_D("Bound[%d]", m_accept_sock.get_fd());
	return true;
}

void ipc_server::register_channel(int fd, channel *ch)
{
	channel_event_handler *ev_handler = new(std::nothrow) channel_event_handler(ch, m_handler);
	retm_if(!ev_handler, "Failed to allocate memory");

	uint64_t id = ch->bind(ev_handler, m_event_loop, true);

	if (id == 0)
		delete ev_handler;

	_D("Registered event[%llu]", id);
}

void ipc_server::register_acceptor(void)
{
	int fd = m_accept_sock.get_fd();

	m_accept_handler = new(std::nothrow) accept_event_handler(this);
	retm_if(!m_accept_handler, "Failed to allocate memory");

	uint64_t id = m_event_loop->add_event(fd,
			(event_condition)(EVENT_IN | EVENT_HUP | EVENT_NVAL), m_accept_handler);

	if (id == 0) {
		_D("Failed to add accept event handler");
		delete m_accept_handler;
		m_accept_handler = NULL;
	}
}

bool ipc_server::close(void)
{
	m_accept_sock.close();

	m_handler = NULL;

	return true;
}
