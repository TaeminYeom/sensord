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

#include "accept_event_handler.h"

#include "sensor_log.h"
#include "ipc_server.h"

using namespace ipc;

accept_event_handler::accept_event_handler(ipc_server *server)
: m_server(server)
{
}

bool accept_event_handler::handle(int fd, event_condition condition)
{
	retv_if((condition & (EVENT_HUP)), false);

	stream_socket *cli_sock = new(std::nothrow) stream_socket();
	retvm_if(!cli_sock, false, "Failed to allocate memory");

	m_server->accept(*cli_sock);

	channel *_ch = new(std::nothrow) channel(cli_sock);
	retvm_if(!_ch, false, "Failed to allocate memory");

	m_server->register_channel(cli_sock->get_fd(), _ch);

	return true;
}
