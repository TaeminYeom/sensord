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
#include <command_types.h>

using namespace sensor;
using namespace ipc;

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
	auto it = m_listeners.find(ch);
	ret_if(it == m_listeners.end());

	_I("Disconnected listener[%u]", it->second->get_id());

	delete it->second;
	m_listeners.erase(ch);
}

void server_channel_handler::read(channel *ch, message &msg)
{
	int err = -EINVAL;

	switch (msg.type()) {
	case CMD_MANAGER_SENSOR_LIST:
		err = manager_get_sensor_list(ch, msg); break;
	case CMD_LISTENER_CONNECT:
		err = listener_connect(ch, msg); break;
	case CMD_LISTENER_DISCONNECT:
		err = listener_disconnect(ch, msg); break;
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
	case CMD_PROVIDER_DISCONNECT:
		err = provider_disconnect(ch, msg); break;
	case CMD_PROVIDER_POST:
		err = provider_post(ch, msg); break;
	default: break;
	}

	if (err != 0) {
		message reply(err);
		ch->send_sync(&reply);
	}
}

int server_channel_handler::manager_get_sensor_list(channel *ch, message &msg)
{
	ipc::message reply;
	char *bytes;
	int size;

	size = m_manager->serialize(ch->get_fd(), &bytes);
	retv_if(size < 0, size);

	reply.enclose((const char *)bytes, size);
	ch->send_sync(&reply);

	delete [] bytes;

	return OP_SUCCESS;
}

int server_channel_handler::listener_connect(channel *ch, message &msg)
{
	static uint32_t listener_id = 1;
	sensor_handler *sensor;
	cmd_listener_connect_t buf;

	msg.disclose((char *)&buf);

	sensor = m_manager->get_sensor(buf.sensor);
	retv_if(!sensor, OP_ERROR);

	sensor_listener_proxy *listener =
		new(std::nothrow) sensor_listener_proxy(listener_id, sensor, ch);
	retvm_if(!listener, OP_ERROR, "Failed to allocate memory");

	buf.listener_id = listener_id;

	message reply;
	reply.enclose((const char *)&buf, sizeof(buf));
	reply.header()->err = OP_SUCCESS;

	if (!ch->send_sync(&reply))
		return OP_ERROR;

	_I("Connected sensor_listener[fd(%d) -> id(%u)]", ch->get_fd(), listener_id);
	listener_id++;
	m_listeners[ch] = listener;

	return OP_SUCCESS;
}

int server_channel_handler::listener_disconnect(channel *ch, message &msg)
{
	auto it = m_listeners.find(ch);
	retv_if(it == m_listeners.end(), -EINVAL);

	uint32_t id = m_listeners[ch]->get_id();

	delete m_listeners[ch];
	m_listeners.erase(ch);

	_D("Disconnected sensor_listener[%u]", id);

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::listener_start(channel *ch, message &msg)
{
	auto it = m_listeners.find(ch);
	retv_if(it == m_listeners.end(), -EINVAL);

	int ret = m_listeners[ch]->start();
	retv_if(ret < 0, ret);

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::listener_stop(channel *ch, message &msg)
{
	auto it = m_listeners.find(ch);
	retv_if(it == m_listeners.end(), -EINVAL);

	int ret = m_listeners[ch]->stop();
	retv_if(ret < 0, ret);

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::listener_attr_int(channel *ch, message &msg)
{
	int ret = OP_SUCCESS;

	auto it = m_listeners.find(ch);
	retv_if(it == m_listeners.end(), -EINVAL);

	cmd_listener_attr_int_t buf;
	msg.disclose((char *)&buf);

	switch (buf.attribute) {
	case SENSORD_ATTRIBUTE_INTERVAL:
		ret = m_listeners[ch]->set_interval(buf.value); break;
	case SENSORD_ATTRIBUTE_MAX_BATCH_LATENCY:
		ret = m_listeners[ch]->set_max_batch_latency(buf.value); break;
	case SENSORD_ATTRIBUTE_PASSIVE_MODE:
		ret = m_listeners[ch]->set_passive_mode(buf.value); break;
	case SENSORD_ATTRIBUTE_PAUSE_POLICY:
	case SENSORD_ATTRIBUTE_AXIS_ORIENTATION:
	default:
		ret = m_listeners[ch]->set_attribute(buf.attribute, buf.value);
	}
	retv_if(ret < 0, ret);

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::listener_attr_str(channel *ch, message &msg)
{
	auto it = m_listeners.find(ch);
	retv_if(it == m_listeners.end(), -EINVAL);

	cmd_listener_attr_str_t buf;
	msg.disclose((char *)&buf);

	int ret = m_listeners[ch]->set_attribute(buf.attribute, buf.value, buf.len);
	retv_if(ret < 0, ret);

	return send_reply(ch, OP_SUCCESS);
}

int server_channel_handler::listener_get_data(channel *ch, message &msg)
{
	return send_reply(ch, OP_ERROR);
}

int server_channel_handler::provider_connect(channel *ch, message &msg)
{
	return send_reply(ch, OP_ERROR);
}

int server_channel_handler::provider_disconnect(channel *ch, message &msg)
{
	return send_reply(ch, OP_ERROR);
}

int server_channel_handler::provider_post(channel *ch, message &msg)
{
	return send_reply(ch, OP_ERROR);
}

int server_channel_handler::send_reply(channel *ch, int error)
{
	message reply(error);
	retvm_if(!ch->send_sync(&reply), OP_ERROR, "Failed to send reply");
	return OP_SUCCESS;
}
