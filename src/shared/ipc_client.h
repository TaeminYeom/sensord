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

#ifndef __IPC_CLIENT_H__
#define __IPC_CLIENT_H__

#include <string>

#include "channel.h"
#include "channel_handler.h"
#include "event_loop.h"

namespace ipc {

class ipc_client {
public:
	ipc_client(const std::string &path);
	~ipc_client();

	bool set_option(int option, int value);
	bool set_option(const std::string &option, int value);

	/* call channel->disconnect() after you have used it */
	channel *connect(channel_handler *handler);
	channel *connect(channel_handler *handler, event_loop *loop);

private:
	std::string m_path;
};

}

#endif /* __IPC_CLIENT_H__ */
