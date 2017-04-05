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

#ifndef __IPC_SERVER_H__
#define __IPC_SERVER_H__

#include <string>

#include "stream_socket.h"
#include "channel.h"
#include "channel_handler.h"
#include "accept_event_handler.h"
#include "event_loop.h"

namespace ipc {

class ipc_server {
public:
	ipc_server(const std::string &path);
	~ipc_server();

	bool set_option(int option, int value);
	bool set_option(const std::string &option, int value);

	bool bind(channel_handler *handler, event_loop *loop);
	bool close(void);

	/* TODO: only accept_handler should use these functions */
	void accept(ipc::socket &cli_sock);
	void register_channel(int fd, channel *ch);
	void register_acceptor(void);

private:
	stream_socket m_accept_sock;

	event_loop *m_event_loop;
	channel_handler *m_handler;
	accept_event_handler *m_accept_handler;
};

}

#endif /* __IPC_SERVER_H__ */
