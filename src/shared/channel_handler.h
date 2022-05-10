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

#ifndef __CHANNEL_HANDLER_H__
#define __CHANNEL_HANDLER_H__

namespace ipc {

class channel;
class message;
class channel_handler;

class channel_handler {
public:
	virtual ~channel_handler() {}

	virtual void connected(channel *ch) = 0;
	virtual void disconnected(channel *ch) = 0;
	virtual void disconnect(void) = 0;
	virtual void read(channel *ch, message &msg) = 0;
	virtual void read_complete(channel *ch) = 0;
	virtual void error_caught(channel *ch, int error) = 0;
	virtual void set_handler(int num, channel_handler* handler) = 0;
};

}

#endif /* __CHANNEL_HANDLER_H__ */
