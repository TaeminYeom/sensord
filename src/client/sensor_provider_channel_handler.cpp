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

#include "sensor_provider_channel_handler.h"

#include <command_types.h>
#include <sensor_log.h>
#include "sensor_provider.h"

using namespace sensor;

sensor_provider::channel_handler::channel_handler(sensor_provider *provider)
: m_provider(provider)
, m_start_cb(NULL)
, m_stop_cb(NULL)
, m_interval_changed_cb(NULL)
, m_start_user_data(NULL)
, m_stop_user_data(NULL)
, m_interval_changed_user_data(NULL)
{
}

void sensor_provider::channel_handler::connected(ipc::channel *ch)
{
	_I("Connected");
}

void sensor_provider::channel_handler::disconnected(ipc::channel *ch)
{
	/* TODO */
	/* m_provider->restore(); */
}

void sensor_provider::channel_handler::read(ipc::channel *ch, ipc::message &msg)
{
	switch (msg.type()) {
	case CMD_PROVIDER_START:
		if (m_start_cb)
			m_start_cb(m_provider, m_start_user_data);
		break;
	case CMD_PROVIDER_STOP:
		if (m_stop_cb)
			m_stop_cb(m_provider, m_stop_user_data);
		break;
	case CMD_PROVIDER_ATTR_INT:
		cmd_provider_attr_int_t buf;
		msg.disclose((char *)&buf);

		if (buf.attribute == SENSORD_ATTRIBUTE_INTERVAL && m_interval_changed_cb)
			m_interval_changed_cb(m_provider, buf.value, m_interval_changed_user_data);
		break;
	}
}

void sensor_provider::channel_handler::read_complete(ipc::channel *ch)
{
}

void sensor_provider::channel_handler::error_caught(ipc::channel *ch, int error)
{
}

void sensor_provider::channel_handler::set_start_cb(sensord_provider_start_cb cb, void *user_data)
{
	m_start_cb = cb;
	m_start_user_data = user_data;
}

void sensor_provider::channel_handler::set_stop_cb(sensord_provider_stop_cb cb, void *user_data)
{
	m_stop_cb = cb;
	m_stop_user_data = user_data;
}

void sensor_provider::channel_handler::set_interval_cb(sensord_provider_interval_changed_cb cb, void *user_data)
{
	m_interval_changed_cb = cb;
	m_interval_changed_user_data = user_data;
}
