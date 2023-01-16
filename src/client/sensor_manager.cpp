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

#include "sensor_manager_channel_handler.h"

using namespace sensor;

sensor_manager::sensor_manager()
: m_client(NULL)
, m_cmd_channel(NULL)
, m_mon_channel(NULL)
, m_connected(false)
, m_handler(NULL)
{
	init();
}

sensor_manager::~sensor_manager()
{
	deinit();
}

int sensor_manager::get_sensor(const char *uri, sensor_t *sensor)
{
	if (!is_supported(uri)) {
		*sensor = NULL;
		_D("Not supported URI [%s]\n", uri);
		return -ENODATA;
	}

	sensor_info *info = get_info(uri);
	retvm_if(!info, -EACCES, "There is no accessible sensor for uri[%s]", uri);

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
	retvm_if(size == 0, -EACCES, "There is no accessible sensors for uri[%s]", uri);

	*list = (sensor_t *)malloc(sizeof(sensor_info *) * size);
	retvm_if(!*list, -ENOMEM, "Failed to allocate memory");

	for (int i = 0; i < size; ++i)
		*(*list + i) = infos[i];

	*count = size;
	return OP_SUCCESS;
}

bool sensor_manager::is_supported(sensor_t sensor)
{
	retvm_if(!sensor, false, "Invalid sensor");

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		if (&*it == sensor)
			return true;
	}

	return false;
}

bool sensor_manager::is_supported(const char *uri)
{
	if (strncmp(uri, utils::get_uri(ALL_SENSOR), strlen(utils::get_uri(ALL_SENSOR))) == 0)
		return true;

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		if ((*it).get_uri() == uri)
			return true;

		std::size_t found = (*it).get_uri().find_last_of("/");
		if (found == std::string::npos)
			continue;

		if ((*it).get_uri().substr(0, found) == uri)
			return true;
	}

	return false;
}

int sensor_manager::set_attribute(sensor_t sensor, int attribute, int value)
{
	if (!sensor) {
		_E("Failed to validate the parameter");
		return -EINVAL;
	}

	if (!m_cmd_channel) {
		_E("Failed to connect to server");
		return -EIO;
	}

	ipc::message msg;
	ipc::message reply;
	cmd_manager_attr_int_t buf = {0, };

	buf.attribute = attribute;
	buf.value = value;

	sensor_info *info = (sensor_info*)sensor;
	memcpy(buf.sensor, info->get_uri().c_str(), info->get_uri().size());

	msg.set_type(CMD_MANAGER_SET_ATTR_INT);
	msg.enclose((char*)&buf, sizeof(buf));

	bool ret = m_cmd_channel->send_sync(msg);
	if (!ret) {
		_E("Failed to send command to set attribute");
		return -EIO;
	}

	ret = m_cmd_channel->read_sync(reply);
	if (!ret) {
		_E("Failed to read reply to set attribute");
		return -EIO;
	}

	if (reply.header()->err < 0) {
		_E("Failed to set attribute");
		return reply.header()->err;
	}

	return OP_SUCCESS;
}

int sensor_manager::get_attribute(sensor_t sensor, int attribute, int *value)
{
	if (!sensor || !value) {
		_E("Failed to validate the parameters");
		return -EINVAL;
	}

	if (!m_cmd_channel) {
		_E("Failed to connect to server");
		return -EIO;
	}

	ipc::message msg;
	ipc::message reply;
	cmd_manager_attr_int_t buf = {0, };

	buf.attribute = attribute;

	sensor_info *info = (sensor_info*)sensor;
	memcpy(buf.sensor, info->get_uri().c_str(), info->get_uri().size());

	msg.set_type(CMD_MANAGER_GET_ATTR_INT);
	msg.enclose((char*)&buf, sizeof(buf));

	bool ret = m_cmd_channel->send_sync(msg);
	if (!ret) {
		_E("Failed to send command to get attribute");
		return -EIO;
	}

	ret = m_cmd_channel->read_sync(reply);
	if (!ret) {
		_E("Failed to read reply to get attribute");
		return -EIO;
	}

	if (reply.header()->err < 0) {
		_E("Failed to get attribute");
		return reply.header()->err;
	}

	if (!reply.header()->length || !reply.body()) {
		_E("Failed to get attribute");
		return -EIO;
	}

	*value = ((cmd_manager_attr_int_t *)reply.body())->value;

	return OP_SUCCESS;
}

int sensor_manager::add_sensor(sensor_info &info)
{
	retv_if(is_supported(info.get_uri().c_str()), OP_ERROR);

	m_sensors.push_back(info);

	_I("Added sensor[%s]", info.get_uri().c_str());

	return OP_SUCCESS;
}

int sensor_manager::add_sensor(sensor_provider *provider)
{
	retvm_if(!provider, -EINVAL, "Invalid parameter");
	return add_sensor(*(provider->get_sensor_info()));
}

int sensor_manager::remove_sensor(const char *uri)
{
	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		if ((*it).get_uri() == uri) {
			m_sensors.erase(it);
			_I("Removed sensor[%s]", uri);

			return OP_SUCCESS;
		}
	}

	return OP_ERROR;
}

int sensor_manager::remove_sensor(sensor_provider *provider)
{
	retvm_if(!provider, -EINVAL, "Invalid parameter");
	return remove_sensor(provider->get_uri());
}

void sensor_manager::add_sensor_added_cb(sensord_added_cb cb, void *user_data)
{
	m_handler->add_sensor_added_cb(cb, user_data);
}

void sensor_manager::remove_sensor_added_cb(sensord_added_cb cb)
{
	m_handler->remove_sensor_added_cb(cb);
}

void sensor_manager::add_sensor_removed_cb(sensord_removed_cb cb, void *user_data)
{
	m_handler->add_sensor_removed_cb(cb, user_data);
}

void sensor_manager::remove_sensor_removed_cb(sensord_removed_cb cb)
{
	m_handler->remove_sensor_removed_cb(cb);
}

bool sensor_manager::init(void)
{
	m_client = new(std::nothrow) ipc::ipc_client(SENSOR_CHANNEL_PATH);
	retvm_if(!m_client, false, "Failed to allocate memory");

	m_handler = new(std::nothrow) channel_handler(this);
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
	ipc::message msg;

	m_cmd_channel = m_client->connect(NULL);
	retvm_if(!m_cmd_channel, false, "Failed to connect to server");

	m_mon_channel = m_client->connect(m_handler, &m_loop);
	retvm_if(!m_mon_channel, false, "Failed to connect to server");

	msg.set_type(CMD_MANAGER_CONNECT);
	m_mon_channel->send_sync(msg);

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

	m_mon_channel->disconnect();
	delete m_mon_channel;
	m_mon_channel = NULL;

	m_cmd_channel->disconnect();
	delete m_cmd_channel;
	m_cmd_channel = NULL;

	_D("Disconnected");
}

bool sensor_manager::is_connected(void)
{
	return m_connected.load();
}

void sensor_manager::restore(void)
{
	ret_if(!is_connected());

	m_cmd_channel->disconnect();
	delete m_cmd_channel;
	m_cmd_channel = NULL;

	m_connected.store(false);
	retm_if(!connect_channel(), "Failed to restore manager");

	_D("Restored manager");
}

void sensor_manager::decode_sensors(const char *buf, std::list<sensor_info> &infos)
{
	int count = 0;
	sensor_info info;
	const int32_t *size;
	const char *data;
	cmd_manager_sensor_list_t *raw;

	raw = (cmd_manager_sensor_list_t *)buf;
	count = raw->sensor_cnt;
	size = (const int32_t *)raw->data;
	data = (const char *)raw->data + sizeof(int32_t);

	for (int i = 0; i < count; ++i) {
		info.clear();
		info.deserialize(data, size[0]);
		infos.push_back(info);

		size = (const int32_t *)((const char *)data + size[0]);
		data = (const char *)size + sizeof(int32_t);
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

	ret = m_cmd_channel->send_sync(msg);
	retvm_if(!ret, false, "Failed to send message");

	ret = m_cmd_channel->read_sync(reply);
	retvm_if(!ret, false, "Failed to receive message");

	reply.disclose(buf, MAX_BUF_SIZE);

	if (!m_sensors.empty())
		m_sensors.clear();

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

	ret = m_cmd_channel->send_sync(msg);
	retvm_if(!ret, false, "Failed to send message");

	ret = m_cmd_channel->read_sync(reply);
	retvm_if(!ret, false, "Failed to receive message");

	if (reply.header()->err == OP_SUCCESS)
		return true;

	_W("This client doesn't have the privilege for sensor[%s]", uri.c_str());

	return false;
}

sensor_info *sensor_manager::get_info(const char *uri)
{
	if (strncmp(uri, utils::get_uri(ALL_SENSOR), strlen(utils::get_uri(ALL_SENSOR))) == 0)
		return &m_sensors.front();

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		if ((*it).get_uri() != uri)
			continue;

		if ((*it).get_privilege().empty() || has_privilege((*it).get_uri()))
			return &*it;

		return NULL;
	}

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		std::size_t found = (*it).get_uri().find_last_of("/");
		if (found == std::string::npos)
			continue;
		if ((*it).get_uri().substr(0, found) != uri)
			continue;

		if ((*it).get_privilege().empty() || has_privilege((*it).get_uri()))
			return &*it;
	}

	return NULL;
}

std::vector<sensor_info *> sensor_manager::get_infos(const char *uri)
{
	std::vector<sensor_info *> infos;
	bool all = false;

	if (strncmp(uri, utils::get_uri(ALL_SENSOR), strlen(utils::get_uri(ALL_SENSOR))) == 0)
		all = true;

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		if ((*it).get_uri() != uri)
			continue;

		if ((*it).get_privilege().empty() || has_privilege((*it).get_uri()))
			infos.push_back(&*it);

		return infos;
	}

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		std::size_t found = (*it).get_uri().find_last_of("/");
		if (!all && found == std::string::npos)
			continue;
		if (!all && (*it).get_uri().substr(0, found) != uri)
			continue;

		if ((*it).get_privilege().empty() || has_privilege((*it).get_uri()))
			infos.push_back(&*it);
	}

	return infos;
}

