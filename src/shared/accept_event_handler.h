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

#ifndef __ACCEPT_EVENT_HANDLER__
#define __ACCEPT_EVENT_HANDLER__

#include "event_handler.h"

namespace ipc {

class ipc_server;

class accept_event_handler : public event_handler
{
public:
	accept_event_handler(ipc_server *server);

	bool handle(int fd, event_condition condition);

private:
	ipc_server *m_server;
};

}

#endif /* __ACCEPT_EVENT_HANDLER__ */
