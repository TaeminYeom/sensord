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

#ifndef __EVENT_HANDLER_H__
#define __EVENT_HANDLER_H__

#include <stdint.h>
namespace ipc {

typedef unsigned int event_condition;

class event_handler {
public:
	event_handler()
		: m_event_id(0)
	{
	}

	virtual ~event_handler() {}

	virtual bool handle(int fd, event_condition condition) = 0;
	void set_event_id(int64_t event_id)
	{
		m_event_id = 0;
	}

protected:
	int64_t m_event_id;
};

}

#endif /* __EVENT_HANDLER_H__ */
