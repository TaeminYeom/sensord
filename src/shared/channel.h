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

#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <unistd.h>
#include <atomic>
#include <vector>

#include "socket.h"
#include "message.h"
#include "event_loop.h"
#include "channel_handler.h"
#include "cmutex.h"

namespace ipc {

class channel_handler;

class channel {
public:
	/* move owernership of the socket to the channel */
	channel(socket *sock);
	~channel();

	uint64_t bind(void);
	uint64_t bind(channel_handler *handler, event_loop *loop, bool loop_bind);

	uint64_t connect(channel_handler *handler, event_loop *loop, bool loop_bind);
	void disconnect(void);

	bool is_connected(void);

	bool send(std::shared_ptr<message> msg);
	bool send_sync(message &msg);

	bool read(void);
	bool read_sync(message &msg, bool select = true);

	bool get_option(int type, int &value) const;
	bool set_option(int type, int value);

	int get_fd(void) const;
	void remove_pending_event_id(uint64_t id);

private:
	int m_fd;
	uint64_t m_event_id;
	socket *m_socket;
	channel_handler *m_handler;
	event_loop *m_loop;
	std::vector<uint64_t> m_pending_event_id;

	std::atomic<bool> m_connected;
	cmutex m_cmutex;
};

}

#endif /* __CHANNEL_H__ */
