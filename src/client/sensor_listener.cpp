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

#include "sensor_listener.h"

#include <channel_handler.h>
#include <sensor_log.h>
#include <sensor_types.h>
#include <command_types.h>
#include <ipc_client.h>

using namespace sensor;

class listener_handler : public ipc::channel_handler
{
public:
	listener_handler(sensor_listener *listener)
	: m_listener(listener)
	{}
	void connected(ipc::channel *ch) {}
	void disconnected(ipc::channel *ch) {}
	void read(ipc::channel *ch, ipc::message &msg) {}
	void read_complete(ipc::channel *ch) {}
	void error_caught(ipc::channel *ch, int error) {}

private:
	sensor_listener *m_listener;
};

sensor_listener::sensor_listener(sensor_t sensor)
: m_id(0)
, m_sensor(reinterpret_cast<sensor_info *>(sensor))
, m_client(NULL)
, m_channel(NULL)
, m_handler(NULL)
, m_evt_handler(NULL)
, m_acc_handler(NULL)
, m_connected(false)
, m_started(false)
{
	init();
}

sensor_listener::~sensor_listener()
{
	deinit();
}

bool sensor_listener::init(void)
{
	return true;
}

void sensor_listener::deinit(void)
{
}

int sensor_listener::get_id(void)
{
	return m_id;
}

sensor_t sensor_listener::get_sensor(void)
{
	return static_cast<sensor_t>(m_sensor);
}

void sensor_listener::restore(void)
{
	_D("Restored listener[%d]", get_id());
}

bool sensor_listener::connect(void)
{
	_I("Listener ID[%d]", get_id());

	return true;
}

void sensor_listener::disconnect(void)
{
	_I("Disconnected[%d]", get_id());
}

bool sensor_listener::is_connected(void)
{
	return m_connected.load();
}

ipc::channel_handler *sensor_listener::get_event_handler(void)
{
	return m_evt_handler;
}

void sensor_listener::set_event_handler(ipc::channel_handler *handler)
{
	m_evt_handler = handler;
}

void sensor_listener::unset_event_handler(void)
{
	delete m_evt_handler;
	m_evt_handler = NULL;
}

ipc::channel_handler *sensor_listener::get_accuracy_handler(void)
{
	return m_acc_handler;
}

void sensor_listener::set_accuracy_handler(ipc::channel_handler *handler)
{
	m_acc_handler = handler;
}

void sensor_listener::unset_accuracy_handler(void)
{
	m_acc_handler = NULL;
}

int sensor_listener::start(void)
{
	return OP_ERROR;
}

int sensor_listener::stop(void)
{
	return OP_ERROR;
}

int sensor_listener::get_interval(void)
{
	return OP_ERROR;
}

int sensor_listener::get_max_batch_latency(void)
{
	return OP_ERROR;
}

int sensor_listener::get_pause_policy(void)
{
	return OP_ERROR;
}

int sensor_listener::get_passive_mode(void)
{
	return OP_ERROR;
}

int sensor_listener::set_interval(unsigned int interval)
{
	return OP_ERROR;
}

int sensor_listener::set_max_batch_latency(unsigned int max_batch_latency)
{
	return OP_ERROR;
}

int sensor_listener::set_passive_mode(bool passive)
{
	return OP_ERROR;
}

int sensor_listener::flush(void)
{
	return OP_ERROR;
}

int sensor_listener::set_attribute(int attribute, int value)
{
	return OP_ERROR;
}

int sensor_listener::set_attribute(int attribute, const char *value, int len)
{
	return OP_ERROR;
}

int sensor_listener::get_sensor_data(sensor_data_t *data)
{
	return OP_ERROR;
}

