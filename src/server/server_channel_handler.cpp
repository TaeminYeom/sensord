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

#include "server_channel_handler.h"

#include <sensor_log.h>
#include <sensor_info.h>
#include <sensor_handler.h>
#include <sensor_utils.h>
#include <sensor_types_private.h>
#include <command_types.h>

#include "permission_checker.h"
#include "application_sensor_handler.h"

using namespace sensor;
using namespace ipc;

/* TODO */
std::unordered_map<uint32_t, sensor_listener_proxy *> server_channel_handler::m_listeners;
std::unordered_map<ipc::channel *, uint32_t> server_channel_handler::m_listener_ids;
std::unordered_map<ipc::channel *, application_sensor_handler *> server_channel_handler::m_app_sensors;

server_channel_handler::server_channel_handler(sensor_manager *manager)
: m_manager(manager)
{
}

server_channel_handler::~server_channel_handler()
{
}

void server_channel_handler::connected(channel *ch)
{
}

void server_channel_handler::disconnected(channel *ch)
{
	m_manager->deregister_channel(ch);

	auto it_asensor = m_app_sensors.find(ch);
	if (it_asensor != m_app_sensors.end()) {
		sensor_info info = it_asensor->second->get_sensor_info();

		_I("Disconnected provider[%s]", info.get_uri().c_str());

		m_manager->deregister_sensor(info.get_uri());
		m_app_sensors.erase(ch);
	}

	auto it_listener = m_listener_ids.find(ch);
	if (it_listener != m_listener_ids.end()) {
		_I("Disconnected listener[%u]", it_listener->second);

		delete m_listeners[it_listener->second];
		m_listeners.erase(it_listener->second);
		m_listener_ids.erase(ch);
	}
}

void server_channel_handler::read(channel *ch, message &msg)
{
	int err = -EINVAL;

	switch (msg.type()) {
	case CMD_MANAGER_CONNECT:
		err = manager_connect(ch, msg); break;
	case CMD_MANAGER_SENSOR_LIST:
		err = manager_get_sensor_list(ch, msg); break;
	case CMD_LISTENER_CONNECT:
		err = listener_connect(ch, msg); break;
	case CMD_LISTENER_START:
		err = listener_start(ch, msg); break;
	case CMD_LISTENER_STOP:
		err = listener_stop(ch, msg); break;
	case CMD_LISTENER_ATTR_INT:
		err = listener_attr_int(ch, msg); break;
	case CMD_LISTENER_ATTR_STR:
		err = listener_attr_str(ch, msg); break;
	case CMD_LISTENER_GET_DATA:
		err = listener_get_data(ch, msg); break;
	case CMD_PROVIDER_CONNECT:
		err = provider_connect(ch, msg); break;
	case CMD_PROVIDER_PUBLISH:
		err = provider_publish(ch, msg); break;
	case CMD_HAS_PRIVILEGE:
		err = has_privileges(ch, msg); break;
	default: break;
	}

	if (err != 0) {
		message reply(err);
		ch->send_sync(&reply);
	}
}

int server_channel_handler::manager_connect(channel *ch, message &msg)
{
	m_manager->register_channel(ch);
	return OP_SUCCESS;
}

int server_channel_handler::manager_get_sensor_list(channel *ch, message &msg)
{
	ipc::message reply;
	char *bytes;
	int size;

	size = m_manager->serialize(ch->get_fd(), &bytes);
	retv_if(size < 0, size);

	reply.enclose((const char *)bytes, size);
	reply.header()->err = OP_SUCCESS;
	ch->send_sync(&reply);

	delete [] bytes;

	return OP_SUCCESS;
}

int server_channel_handler::listener_connect(channel *ch, message &msg)
{
	static uint32_t listener_id = 1;
	cmd_listener_connect_t buf;

	msg.disclose((char *)&buf);

	sensor_listener_proxy *listener = NULL;
	listener = new(std::nothrow) sensor_listener_proxy(listener_id,
				buf.sensor, m_manager, ch);
	retvm_if(!listener, OP_ERROR, "Failed to allocate memory");

	if (!has_privileges(ch->get_fd(), listener->get_required_privileges())) {
		delete listener;
		listener = NULL;
		_E("Permission denied[%d, %s]", listener_id,
			m_listeners[listener_id]->get_required_privileges().c_str());
		return -EACCES;
	}

	buf.listener_id = listener_id;

	message reply;
	reply.enclose((const char *)&buf, sizeof(buf));
	reply.header()->err = OP_SUCCESS;

	if (!ch->send_sync(&reply)) {
		delete listener;
		return OP_ERROR;
	}

	_I("Connected sensor_listener[fd(%d) -> id(%u)]", ch->get_fd(), listener_id);
	m_listeners[listener_id] = listener;
	m_listener_ids[ch] = listener_id;
	listener_id++;

	return OP_SUCCESS;
}

int server_channel_handler::listener_start(channel *ch, message &msg)
{
	cmd_listener_start_t buf;
	msg.disclose((char *)&buf);
	uint32_t id = buf.listener_id;

	auto it = m_listeners.find(id);
	retv_if(it == m_listeners.end(), -EINVAL);
	retvm_if(!has_privileges(ch->get_fd(), m_listeners[id]->get_required_privileges()),
			-EACCES, "Permission denied[%d, %s]",
			id, m_listeners[id]->get_required_privileges().c_str());

	int ret = m_listeners[id]->start();
	retvm_if(ret < 0, ret, "Failed to start listener[%d]", id);

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::listener_stop(channel *ch, message &msg)
{
	cmd_listener_stop_t buf;
	msg.disclose((char *)&buf);
	uint32_t id = buf.listener_id;

	auto it = m_listeners.find(id);
	retv_if(it == m_listeners.end(), -EINVAL);
	retvm_if(!has_privileges(ch->get_fd(), m_listeners[id]->get_required_privileges()),
			-EACCES, "Permission denied[%d, %s]",
			id, m_listeners[id]->get_required_privileges().c_str());

	int ret = m_listeners[id]->stop();
	retvm_if(ret < 0, ret, "Failed to stop listener[%d]", id);

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::listener_attr_int(channel *ch, message &msg)
{
	cmd_listener_attr_int_t buf;
	msg.disclose((char *)&buf);
	uint32_t id = buf.listener_id;

	int ret = OP_SUCCESS;

	auto it = m_listeners.find(id);
	retv_if(it == m_listeners.end(), -EINVAL);
	retvm_if(!has_privileges(ch->get_fd(), m_listeners[id]->get_required_privileges()),
			-EACCES, "Permission denied[%d, %s]",
			id, m_listeners[id]->get_required_privileges().c_str());

	switch (buf.attribute) {
	case SENSORD_ATTRIBUTE_INTERVAL:
		ret = m_listeners[id]->set_interval(buf.value); break;
	case SENSORD_ATTRIBUTE_MAX_BATCH_LATENCY:
		ret = m_listeners[id]->set_max_batch_latency(buf.value); break;
	case SENSORD_ATTRIBUTE_PASSIVE_MODE:
		ret = m_listeners[id]->set_passive_mode(buf.value); break;
	case SENSORD_ATTRIBUTE_PAUSE_POLICY:
	case SENSORD_ATTRIBUTE_AXIS_ORIENTATION:
	default:
		ret = m_listeners[id]->set_attribute(buf.attribute, buf.value);
	}
	/* TODO : check return value */
	if (ret < 0)
		_W("Return : %d", ret);

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::listener_attr_str(channel *ch, message &msg)
{
	uint32_t id;
	cmd_listener_attr_str_t *buf;

	buf = (cmd_listener_attr_str_t *) new(std::nothrow) char[msg.size()];
	retvm_if(!buf, -ENOMEM, "Failed to allocate memory");

	msg.disclose((char *)buf);

	id = buf->listener_id;
	auto it = m_listeners.find(id);
	if (it == m_listeners.end()) {
		delete [] buf;
		return -EINVAL;
	}

	if (!has_privileges(ch->get_fd(), m_listeners[id]->get_required_privileges())) {
		_E("Permission denied[%d, %s]", id, m_listeners[id]->get_required_privileges().c_str());
		delete [] buf;
		return -EACCES;
	}

	int ret = m_listeners[id]->set_attribute(buf->attribute, buf->value, buf->len);
	if (ret < 0) {
		delete [] buf;
		return ret;
	}

	delete [] buf;
	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::listener_get_data(channel *ch, message &msg)
{
	ipc::message reply;
	cmd_listener_get_data_t buf;
	sensor_data_t *data;
	int len;
	uint32_t id;

	msg.disclose((char *)&buf);
	id = buf.listener_id;

	auto it = m_listeners.find(id);
	retv_if(it == m_listeners.end(), -EINVAL);
	retvm_if(!has_privileges(ch->get_fd(), m_listeners[id]->get_required_privileges()),
			-EACCES, "Permission denied[%d, %s]",
			id, m_listeners[id]->get_required_privileges().c_str());

	int ret = m_listeners[id]->get_data(&data, &len);
	retv_if(ret < 0, ret);

	memcpy(&buf.data, data, sizeof(sensor_data_t));
	buf.len = sizeof(sensor_data_t);

	reply.enclose((const char *)&buf, sizeof(cmd_listener_get_data_t));
	reply.header()->err = OP_SUCCESS;
	reply.header()->type = CMD_LISTENER_GET_DATA;

	ch->send_sync(&reply);

	free(data);

	return OP_SUCCESS;
}

int server_channel_handler::provider_connect(channel *ch, message &msg)
{
	sensor_info info;
	info.clear();
	info.deserialize(msg.body(), msg.size());

	info.show();

	application_sensor_handler *sensor;
	sensor = new(std::nothrow) application_sensor_handler(info, ch);
	retvm_if(!sensor, -ENOMEM, "Failed to allocate memory");

	if (!m_manager->register_sensor(sensor)) {
		delete sensor;
		return -EINVAL;
	}

	/* temporarily */
	m_app_sensors[ch] = sensor;

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::provider_publish(channel *ch, message &msg)
{
	auto it = m_app_sensors.find(ch);
	retv_if(it == m_app_sensors.end(), -EINVAL);

	sensor_data_t *data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	retvm_if(!data, -ENOMEM, "Failed to allocate memory");

	msg.disclose((char *)data);

	it->second->publish(data, sizeof(sensor_data_t));

	return OP_SUCCESS;
}

int server_channel_handler::has_privileges(channel *ch, message &msg)
{
	sensor_handler *sensor;
	cmd_has_privilege_t buf;
	msg.disclose((char *)&buf);

	sensor = m_manager->get_sensor(buf.sensor);
	retv_if(!sensor, OP_ERROR);

	sensor_info info = sensor->get_sensor_info();

	if (!has_privileges(ch->get_fd(), info.get_privilege()))
		return OP_ERROR;

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::send_reply(channel *ch, int error)
{
	message reply(error);
	retvm_if(!ch->send_sync(&reply), OP_ERROR, "Failed to send reply");
	return OP_SUCCESS;
}

bool server_channel_handler::has_privilege(int fd, std::string &priv)
{
	static permission_checker checker;
	return checker.has_permission(fd, priv);
}

bool server_channel_handler::has_privileges(int fd, std::string priv)
{
	std::vector<std::string> privileges;
	privileges = utils::tokenize(priv, PRIV_DELIMITER);

	for (auto it = privileges.begin(); it != privileges.end(); ++it) {
		if (!has_privilege(fd, *it))
			return false;
	}

	return true;
}
