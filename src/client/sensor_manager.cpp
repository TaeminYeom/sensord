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

#define SIZE_STR_SENSOR_ALL 27

using namespace sensor;

class manager_handler : public ipc::channel_handler
{
public:
	manager_handler(sensor_manager *manager)
	: m_manager(manager)
	{}
	void connected(ipc::channel *ch) {}
	void disconnected(ipc::channel *ch)
	{
		/* If channel->disconnect() is not explicitly called, it will be restored */
		m_manager->restore();
	}

	void read(ipc::channel *ch, ipc::message &msg)
	{
		/* TODO: if dynamic sensor is loaded,
		 * it will be called with the sensor information */
	}

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
	return get_sensor(utils::get_uri(type), sensor);
}

int sensor_manager::get_sensors(sensor_type_t type, sensor_t **list, int *count)
{
	return get_sensors(utils::get_uri(type), list, count);
}

int sensor_manager::get_sensor(const char *uri, sensor_t *sensor)
{
	if (!is_supported(uri)) {
		*sensor = NULL;
		return -ENODATA;
	}

	sensor_info *info = get_info(uri);
	retv_if(!info, -EACCES);

	*sensor = (sensor_t)info;
	return OP_SUCCESS;
}

int sensor_manager::get_sensors(const char *uri, sensor_t **list, int *count)
{
	retv_if(!is_supported(uri), -ENODATA);

	std::vector<sensor_info *> infos;
	int size;

	infos = get_infos(uri);
	size = infos.size();
	retv_if(size == 0, -EACCES);

	*list = (sensor_t *)malloc(sizeof(sensor_info *) * size);
	retvm_if(!*list, -ENOMEM, "Failed to allocate memory");

	for (int i = 0; i < size; ++i)
		*(*list + i) = infos[i];

	*count = size;
	return OP_SUCCESS;
}

bool sensor_manager::is_supported(sensor_t sensor)
{
	retvm_if(!sensor, false, "Invalid parameter[%#x]", sensor);

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		if (&*it == sensor)
			return true;
	}

	return false;
}

bool sensor_manager::is_supported(const char *uri)
{
	if (strncmp(uri, utils::get_uri(ALL_SENSOR), SIZE_STR_SENSOR_ALL) == 0)
		return true;

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		std::size_t found = (*it).get_uri().rfind(uri);

		if (found != std::string::npos)
			return true;
	}

	return false;
}

bool sensor_manager::init(void)
{
	m_client = new(std::nothrow) ipc::ipc_client(SENSOR_CHANNEL_PATH);
	retvm_if(!m_client, false, "Failed to allocate memory");

	m_handler = new(std::nothrow) manager_handler(this);
	if (!m_handler) {
		delete m_client;
		m_client = NULL;
		return false;
	}

	return true;
}

void sensor_manager::deinit(void)
{
	disconnect();

	delete m_handler;
	m_handler = NULL;

	delete m_client;
	m_client = NULL;
}

bool sensor_manager::connect_channel(void)
{
	m_channel = m_client->connect(m_handler, &m_loop);
	retvm_if(!m_channel, false, "Failed to connect to server");

	m_connected.store(true);

	_D("Connected");
	return true;
}

bool sensor_manager::connect(void)
{
	retv_if(is_connected(), true);
	retv_if(!connect_channel(), false);

	return get_sensors_internal();
}

void sensor_manager::disconnect(void)
{
	ret_if(!is_connected());

	m_connected.store(false);
	m_channel->disconnect();

	delete m_channel;
	m_channel = NULL;

	_D("Disconnected");
}

bool sensor_manager::is_connected(void)
{
	return m_connected.load();
}

void sensor_manager::restore(void)
{
	ret_if(!is_connected());

	m_connected.store(false);
	retm_if(!connect_channel(), "Failed to restore manager");

	_D("Restored manager");
}

void sensor_manager::decode_sensors(const char *buf, std::vector<sensor_info> &infos)
{
	int count = 0;
	sensor_info info;
	const size_t *size;
	const char *data;
	cmd_manager_sensor_list_t *raw;

	raw = (cmd_manager_sensor_list_t *)buf;
	count = raw->sensor_cnt;
	size = (const size_t *)raw->data;
	data = (const char *)raw->data + sizeof(size_t);

	for (int i = 0; i < count; ++i) {
		info.clear();
		info.deserialize(data, size[0]);
		infos.push_back(info);

		size = (const size_t *)((const char *)data + size[0]);
		data = (const char *)size + sizeof(size_t);
	}

	_D("Sensor count : %d", count);
}

bool sensor_manager::get_sensors_internal(void)
{
	retvm_if(!is_connected(), false, "Failed to get sensors");

	bool ret;
	ipc::message msg;
	ipc::message reply;
	char buf[MAX_BUF_SIZE];

	msg.set_type(CMD_MANAGER_SENSOR_LIST);

	ret = m_channel->send_sync(&msg);
	retvm_if(!ret, false, "Failed to send message");

	ret = m_channel->read_sync(reply);
	retvm_if(!ret, false, "Failed to receive message");

	reply.disclose(buf);

	decode_sensors(buf, m_sensors);

	return true;
}

bool sensor_manager::has_privilege(std::string &uri)
{
	retvm_if(!is_connected(), false, "Failed to get sensors");

	bool ret;
	ipc::message msg;
	ipc::message reply;
	cmd_has_privilege_t buf = {0, };

	msg.set_type(CMD_HAS_PRIVILEGE);
	memcpy(buf.sensor, uri.c_str(), uri.size());
	msg.enclose((const char *)&buf, sizeof(buf));

	ret = m_channel->send_sync(&msg);
	retvm_if(!ret, false, "Failed to send message");

	ret = m_channel->read_sync(reply);
	retvm_if(!ret, false, "Failed to receive message");

	if (reply.header()->err == OP_SUCCESS)
		return true;

	return false;
}

sensor_info *sensor_manager::get_info(const char *uri)
{
	if (strncmp(uri, utils::get_uri(ALL_SENSOR), SIZE_STR_SENSOR_ALL) == 0)
		return &m_sensors[0];

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		std::size_t found = (*it).get_uri().rfind(uri);

		if (found == std::string::npos)
			continue;

		if ((*it).get_privilege().empty())
			return &*it;

		if (has_privilege((*it).get_uri()))
			return &*it;
	}

	return NULL;
}

std::vector<sensor_info *> sensor_manager::get_infos(const char *uri)
{
	std::vector<sensor_info *> infos;
	bool all = false;

	if (strncmp(uri, utils::get_uri(ALL_SENSOR), SIZE_STR_SENSOR_ALL) == 0)
		all = true;

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		std::size_t found = (*it).get_uri().rfind(uri);

		if (!all && found == std::string::npos)
			continue;

		if ((*it).get_privilege().empty()) {
			infos.push_back(&*it);
			continue;
		}

		if (has_privilege((*it).get_uri()))
			infos.push_back(&*it);
	}

	return infos;
}

