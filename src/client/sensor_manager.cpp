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

#include "sensor_manager.h"

#include <sensor_log.h>
#include <sensor_info.h>
#include <sensor_utils.h>
#include <command_types.h>
#include <ipc_client.h>
#include <message.h>
#include <channel.h>

using namespace sensor;

class manager_handler : public ipc::channel_handler
{
public:
	manager_handler(sensor_manager *manager)
	: m_manager(manager)
	{}
	void connected(ipc::channel *ch) {}
	void disconnected(ipc::channel *ch) {}
	void read(ipc::channel *ch, ipc::message &msg) {}
	void read_complete(ipc::channel *ch) {}
	void error_caught(ipc::channel *ch, int error) {}

private:
	sensor_manager *m_manager;
};

sensor_manager::sensor_manager()
: m_client(NULL)
, m_handler(NULL)
, m_channel(NULL)
, m_connected(false)
{
	init();
}

sensor_manager::~sensor_manager()
{
	deinit();
}

int sensor_manager::get_sensor(sensor_type_t type, sensor_t *sensor)
{
	return OP_ERROR;
}

int sensor_manager::get_sensors(sensor_type_t type, sensor_t **list, int *count)
{
	return OP_ERROR;
}

int sensor_manager::get_sensor(const char *uri, sensor_t *sensor)
{
	return OP_ERROR;
}

int sensor_manager::get_sensors(const char *uri, sensor_t **list, int *count)
{
	return OP_ERROR;
}

bool sensor_manager::is_supported(sensor_t sensor)
{
	return false;
}

bool sensor_manager::is_supported(const char *uri)
{
	return false;
}

bool sensor_manager::init(void)
{
	return true;
}

void sensor_manager::deinit(void)
{
}

bool sensor_manager::connect_channel(void)
{
	_D("Connected");
	return true;
}

bool sensor_manager::connect(void)
{
	return false;
}

void sensor_manager::disconnect(void)
{
	_D("Disconnected");
}

bool sensor_manager::is_connected(void)
{
	return m_connected.load();
}

void sensor_manager::restore(void)
{
	_D("Restored manager");
}

void sensor_manager::decode_sensors(const char *buf, std::vector<sensor_info> &infos)
{
	int count = 0;
	_D("Sensor count : %d", count);
}

bool sensor_manager::get_sensors_internal(void)
{
	return true;
}

sensor_info *sensor_manager::get_info(const char *uri)
{
	return NULL;
}

std::vector<sensor_info *> sensor_manager::get_infos(const char *uri)
{
	std::vector<sensor_info *> infos;
	return infos;
}

