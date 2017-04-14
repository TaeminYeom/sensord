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

#ifndef __CHANNEL_EVENT_HANDLER_H__
#define __CHANNEL_EVENT_HANDLER_H__

#include <unordered_map>

#include "event_handler.h"
#include "channel_handler.h"

namespace ipc {

class channel;

class channel_event_handler : public event_handler, public channel_handler {
public:
	channel_event_handler(channel *ch, channel_handler *handler);
	virtual ~channel_event_handler();

	bool handle(int fd, event_condition condition);

	void connected(channel *ch);
	void disconnected(channel *ch);
	void read(channel *ch, message &msg);
	void read_complete(channel *ch);
	void error_caught(channel *ch, int error);

private:
	channel *m_ch;
	channel_handler *m_handler;
};

}

#endif /* __CHANNEL_EVENT_HANDLER_H__ */
