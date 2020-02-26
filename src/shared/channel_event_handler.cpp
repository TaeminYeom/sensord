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

#include "channel_event_handler.h"

#include "channel.h"
#include "channel_handler.h"
#include "sensor_log.h"

using namespace ipc;

channel_event_handler::channel_event_handler(channel *ch, channel_handler *handler)
: m_ch(ch)
, m_handler(handler)
{
	_D("Create[%p]", this);
}

channel_event_handler::~channel_event_handler()
{
	_D("Destroy[%p]", this);
	m_ch = NULL;
	m_handler = NULL;
}

bool channel_event_handler::handle(int fd, event_condition condition)
{
	message msg;

	if (!m_ch || !m_ch->is_connected())
		return false;

	if (condition & (EVENT_HUP)) {
		_D("Disconnect[%p] : The other proccess is dead", this);
		m_ch->disconnect();
		m_ch = NULL;
		return false;
	}

	if (!m_ch->read_sync(msg, false)) {
		m_ch = NULL;
		return false;
	}

	return true;
}

void channel_event_handler::connected(channel *ch)
{
	if (m_handler)
		m_handler->connected(ch);
}

void channel_event_handler::disconnected(channel *ch)
{
	if (m_handler)
		m_handler->disconnected(ch);
}

void channel_event_handler::read(channel *ch, message &msg)
{
	if (m_handler)
		m_handler->read(ch, msg);
}

void channel_event_handler::read_complete(channel *ch)
{
	if (m_handler)
		m_handler->read_complete(ch);
}

void channel_event_handler::error_caught(channel *ch, int error)
{
	if (m_handler)
		m_handler->error_caught(ch, error);
}
