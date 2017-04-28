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

#include "sensor_provider.h"

#include <message.h>
#include <channel.h>
#include <sensor_log.h>
#include <sensor_types.h>
#include <ipc_client.h>
#include <command_types.h>

#include "sensor_provider_channel_handler.h"

using namespace sensor;

sensor_provider::sensor_provider(const char *uri)
: m_client(NULL)
, m_channel(NULL)
, m_handler(NULL)
, m_connected(false)
{
	init(uri);
}

sensor_provider::~sensor_provider()
{
	deinit();
}

bool sensor_provider::init(const char *uri)
{
	m_client = new(std::nothrow) ipc::ipc_client(SENSOR_CHANNEL_PATH);
	retvm_if(!m_client, false, "Failed to allocate memory");

	m_handler = new(std::nothrow) channel_handler(this);
	if (!m_handler) {
		delete m_client;
		return false;
	}

	m_sensor.set_uri(uri);
	m_sensor.set_min_range(0);
	m_sensor.set_max_range(1);
	m_sensor.set_resolution(1);
	m_sensor.set_privilege(PRIV_APPLICATION_SENSOR);

	return true;
}

void sensor_provider::deinit(void)
{
	disconnect();

	delete m_handler;
	m_handler = NULL;

	delete m_client;
	m_client = NULL;
}

const char *sensor_provider::get_uri(void)
{
	return m_sensor.get_uri().c_str();
}

sensor_info *sensor_provider::get_sensor_info(void)
{
	return &m_sensor;
}

int sensor_provider::serialize(sensor_info *info, char **bytes)
{
	int size;
	raw_data_t *raw = new(std::nothrow) raw_data_t;
	retvm_if(!raw, -ENOMEM, "Failed to allocated memory");

	info->serialize(*raw);

	*bytes = new(std::nothrow) char[raw->size()];
	retvm_if(!*bytes, -ENOMEM, "Failed to allocate memory");

	std::copy(raw->begin(), raw->end(), *bytes);

	size = raw->size();
	delete raw;

	return size;
}

int sensor_provider::send_sensor_info(sensor_info *info)
{
	char *bytes;
	int size;

	size = serialize(info, &bytes);

	ipc::message msg((const char *)bytes, size);
	msg.set_type(CMD_PROVIDER_CONNECT);

	m_channel->send_sync(&msg);

	return OP_SUCCESS;
}

int sensor_provider::connect(void)
{
	m_channel = m_client->connect(m_handler, &m_loop);
	retvm_if(!m_channel, -EIO, "Failed to connect to server");

	/* serialize and send sensor info */
	send_sensor_info(get_sensor_info());

	/* check error */
	ipc::message reply;
	m_channel->read_sync(reply);
	retv_if(reply.header()->err < 0, reply.header()->err);

	m_connected.store(true);

	_I("Provider URI[%s]", get_uri());

	return OP_SUCCESS;
}

bool sensor_provider::disconnect(void)
{
	retv_if(!is_connected(), false);
	m_connected.store(false);

	ipc::message msg(OP_SUCCESS);
	ipc::message reply;

	msg.set_type(CMD_PROVIDER_DISCONNECT);

	m_channel->send_sync(&msg);
	m_channel->read_sync(reply);

	m_channel->disconnect();

	delete m_channel;
	m_channel = NULL;

	_I("Disconnected[%s]", get_uri());

	return true;
}

void sensor_provider::restore(void)
{
	ret_if(!is_connected());
	retm_if(!connect(), "Failed to restore provider");

	_D("Restored provider[%s]", get_uri());
}

int sensor_provider::publish(sensor_data_t *data, int len)
{
	ipc::message msg;
	msg.set_type(CMD_PROVIDER_PUBLISH);
	msg.enclose((const char *)data, len);

	m_channel->send_sync(&msg);

	return OP_SUCCESS;
}

bool sensor_provider::is_connected(void)
{
	return m_connected.load();
}

void sensor_provider::set_start_cb(sensord_provider_start_cb cb, void *user_data)
{
	m_handler->set_start_cb(cb, user_data);
}

void sensor_provider::set_stop_cb(sensord_provider_stop_cb cb, void *user_data)
{
	m_handler->set_stop_cb(cb, user_data);
}

void sensor_provider::set_interval_cb(sensord_provider_set_interval_cb cb, void *user_data)
{
	m_handler->set_interval_cb(cb, user_data);
}
