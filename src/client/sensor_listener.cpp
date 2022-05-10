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
	{
		evt_handler[0] = evt_handler[1] = evt_handler[2] = evt_handler[3] = NULL;
	}

	void connected(ipc::channel *ch) {}
	void disconnected(ipc::channel *ch)
	{
		/* If channel->disconnect() is not explicitly called,
		 * listener will be restored */
		if (m_listener)
			m_listener->restore();
	}

	void disconnect(void)
	{
		m_listener = NULL;
	}

	void read(ipc::channel *ch, ipc::message &msg)
	{
		ipc::channel_handler *handler = NULL;
		switch (msg.header()->type) {
		case CMD_LISTENER_EVENT:
			handler = evt_handler[0];
			if (handler)
				handler->read(ch, msg);
			break;
		case CMD_LISTENER_ACC_EVENT:
			handler = evt_handler[1];
			if (handler)
				handler->read(ch, msg);
			break;
		case CMD_LISTENER_SET_ATTR_INT:
			handler = evt_handler[2];
			if (handler)
				handler->read(ch, msg);
			break;
		case CMD_LISTENER_SET_ATTR_STR:
			handler = evt_handler[3];
			if (handler)
				handler->read(ch, msg);
			break;
		case CMD_LISTENER_CONNECTED:
			// Do nothing
			break;
		default:
			_W("Invalid command message");
		}
	}

	void set_handler(int num, ipc::channel_handler* handler) {
		evt_handler[num] = handler;
	}

	void read_complete(ipc::channel *ch) {}
	void error_caught(ipc::channel *ch, int error) {}

private:
	ipc::channel_handler *evt_handler[4];
	sensor_listener *m_listener;
};

sensor_listener::sensor_listener(sensor_t sensor)
: m_id(0)
, m_sensor(reinterpret_cast<sensor_info *>(sensor))
, m_client(NULL)
, m_cmd_channel(NULL)
, m_evt_channel(NULL)
, m_handler(NULL)
, m_evt_handler(NULL)
, m_acc_handler(NULL)
, m_attr_int_changed_handler(NULL)
, m_attr_str_changed_handler(NULL)
, m_connected(false)
, m_started(false)
{
	init();
}

sensor_listener::sensor_listener(sensor_t sensor, ipc::event_loop *loop)
: m_id(0)
, m_sensor(reinterpret_cast<sensor_info *>(sensor))
, m_client(NULL)
, m_cmd_channel(NULL)
, m_evt_channel(NULL)
, m_handler(NULL)
, m_evt_handler(NULL)
, m_acc_handler(NULL)
, m_attr_int_changed_handler(NULL)
, m_attr_str_changed_handler(NULL)
, m_loop(loop)
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
	m_client = new(std::nothrow) ipc::ipc_client(SENSOR_CHANNEL_PATH);
	retvm_if(!m_client, false, "Failed to allocate memory");

	m_handler = new(std::nothrow) listener_handler(this);
	if (!m_handler) {
		delete m_client;
		return false;
	}

	if (!connect()) {
		delete m_handler;
		delete m_client;
		m_handler = NULL;
		m_client = NULL;
		return false;
	}

	return true;
}

void sensor_listener::deinit(void)
{
	AUTOLOCK(lock);
	_D("Deinitializing..");
	stop();
	disconnect();

	m_handler->disconnect();
	m_loop->add_channel_handler_release_list(m_handler);
	m_handler = NULL;

	delete m_client;
	m_client = NULL;

	unset_event_handler();
	unset_accuracy_handler();
	unset_attribute_int_changed_handler();
	unset_attribute_str_changed_handler();

	m_attributes_int.clear();
	m_attributes_str.clear();
	_D("Deinitialized..");
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
	if (lock.try_lock())
		return;

	m_cmd_channel->disconnect();
	delete m_cmd_channel;
	m_cmd_channel = NULL;

	retm_if(!connect(), "Failed to restore listener");

	_D("Restoring sensor listener");

	/* Restore attributes/status */
	if (m_started.load())
		start();

	auto interval = m_attributes_int.find(SENSORD_ATTRIBUTE_INTERVAL);
	if (interval != m_attributes_int.end())
		set_interval(m_attributes_int[SENSORD_ATTRIBUTE_INTERVAL]);

	auto latency = m_attributes_int.find(SENSORD_ATTRIBUTE_MAX_BATCH_LATENCY);
	if (latency != m_attributes_int.end())
		set_max_batch_latency(m_attributes_int[SENSORD_ATTRIBUTE_MAX_BATCH_LATENCY]);

	auto power = m_attributes_int.find(SENSORD_ATTRIBUTE_PAUSE_POLICY);
	if (power != m_attributes_int.end())
		set_attribute(SENSORD_ATTRIBUTE_PAUSE_POLICY, m_attributes_int[SENSORD_ATTRIBUTE_PAUSE_POLICY]);

	_D("Restored listener[%d]", get_id());
	lock.unlock();
}

bool sensor_listener::connect(void)
{
	m_cmd_channel = m_client->connect(NULL);
	retvm_if(!m_cmd_channel, false, "Failed to connect to server");

	m_evt_channel = m_client->connect(m_handler, m_loop, false);
	retvm_if(!m_evt_channel, false, "Failed to connect to server");

	ipc::message msg;
	ipc::message reply;
	cmd_listener_connect_t buf = {0, };

	memcpy(buf.sensor, m_sensor->get_uri().c_str(), m_sensor->get_uri().size());
	msg.set_type(CMD_LISTENER_CONNECT);
	msg.enclose((const char *)&buf, sizeof(buf));
	m_evt_channel->send_sync(msg);

	m_evt_channel->read_sync(reply);
	reply.disclose((char *)&buf, sizeof(buf));

	m_id = buf.listener_id;
	m_connected.store(true);

	m_evt_channel->bind();

	_I("Connected listener[%d] with sensor[%s]", get_id(), m_sensor->get_uri().c_str());

	return true;
}

void sensor_listener::disconnect(void)
{
	ret_if(!is_connected());
	m_connected.store(false);

	_D("Disconnecting..");

	m_loop->add_channel_release_queue(m_evt_channel);
	m_evt_channel = NULL;

	m_cmd_channel->disconnect();
	delete m_cmd_channel;
	m_cmd_channel = NULL;

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
	m_handler->set_handler(0, handler);
	if (m_evt_handler)
		m_loop->add_channel_handler_release_list(m_evt_handler);
	m_evt_handler = handler;
}

void sensor_listener::unset_event_handler(void)
{
	if (m_evt_handler) {
		m_handler->set_handler(0, NULL);
		m_loop->add_channel_handler_release_list(m_evt_handler);
		m_evt_handler = NULL;
	}
}

ipc::channel_handler *sensor_listener::get_accuracy_handler(void)
{
	return m_acc_handler;
}

void sensor_listener::set_accuracy_handler(ipc::channel_handler *handler)
{
	m_handler->set_handler(1, handler);
	if (m_acc_handler)
		m_loop->add_channel_handler_release_list(m_acc_handler);
	m_acc_handler = handler;
}

void sensor_listener::unset_accuracy_handler(void)
{
	if (m_acc_handler) {
		m_handler->set_handler(1, NULL);
		m_loop->add_channel_handler_release_list(m_acc_handler);
		m_acc_handler = NULL;
	}
}

ipc::channel_handler *sensor_listener::get_attribute_int_changed_handler(void)
{
	return m_attr_int_changed_handler;
}

void sensor_listener::set_attribute_int_changed_handler(ipc::channel_handler *handler)
{
	m_handler->set_handler(2, handler);
	if (m_attr_int_changed_handler)
		m_loop->add_channel_handler_release_list(m_attr_int_changed_handler);
	m_attr_int_changed_handler = handler;
}

void sensor_listener::unset_attribute_int_changed_handler(void)
{
	if (m_attr_int_changed_handler) {
		m_handler->set_handler(2, NULL);
		m_loop->add_channel_handler_release_list(m_attr_int_changed_handler);
		m_attr_int_changed_handler = NULL;
	}
}

ipc::channel_handler *sensor_listener::get_attribute_str_changed_handler(void)
{
	return m_attr_str_changed_handler;
}

void sensor_listener::set_attribute_str_changed_handler(ipc::channel_handler *handler)
{
	m_handler->set_handler(3, handler);
	if (m_attr_str_changed_handler)
		m_loop->add_channel_handler_release_list(m_attr_str_changed_handler);
	m_attr_str_changed_handler = handler;
}

void sensor_listener::unset_attribute_str_changed_handler(void)
{
	if (m_attr_str_changed_handler) {
		m_handler->set_handler(3, NULL);
		m_loop->add_channel_handler_release_list(m_attr_str_changed_handler);
		m_attr_str_changed_handler = NULL;
	}
}

int sensor_listener::start(void)
{
	ipc::message msg;
	ipc::message reply;
	cmd_listener_start_t buf;

	retvm_if(!m_cmd_channel, -EINVAL, "Failed to connect to server");

	buf.listener_id = m_id;
	msg.set_type(CMD_LISTENER_START);
	msg.enclose((char *)&buf, sizeof(buf));

	m_cmd_channel->send_sync(msg);
	m_cmd_channel->read_sync(reply);

	if (reply.header()->err < 0) {
		_E("Failed to start listener[%d], sensor[%s]", get_id(), m_sensor->get_uri().c_str());
		return reply.header()->err;
	}

	m_started.store(true);

	_I("Listener[%d] started", get_id());

	return OP_SUCCESS;
}

int sensor_listener::stop(void)
{
	ipc::message msg;
	ipc::message reply;
	cmd_listener_stop_t buf;

	retvm_if(!m_cmd_channel, -EINVAL, "Failed to connect to server");
	retvm_if(!m_started.load(), -EAGAIN, "Already stopped");

	buf.listener_id = m_id;
	msg.set_type(CMD_LISTENER_STOP);
	msg.enclose((char *)&buf, sizeof(buf));

	m_cmd_channel->send_sync(msg);
	m_cmd_channel->read_sync(reply);

	if (reply.header()->err < 0) {
		_E("Failed to stop listener[%d]", get_id());
		return reply.header()->err;
	}

	m_started.store(false);

	_I("Listener[%d] stopped", get_id());

	return OP_SUCCESS;
}

int sensor_listener::get_interval(void)
{
	auto it = m_attributes_int.find(SENSORD_ATTRIBUTE_INTERVAL);
	retv_if(it == m_attributes_int.end(), -1);

	return m_attributes_int[SENSORD_ATTRIBUTE_INTERVAL];
}

int sensor_listener::get_max_batch_latency(void)
{
	auto it = m_attributes_int.find(SENSORD_ATTRIBUTE_MAX_BATCH_LATENCY);
	retv_if(it == m_attributes_int.end(), -1);

	return m_attributes_int[SENSORD_ATTRIBUTE_MAX_BATCH_LATENCY];
}

int sensor_listener::get_pause_policy(void)
{
	auto it = m_attributes_int.find(SENSORD_ATTRIBUTE_PAUSE_POLICY);
	retv_if(it == m_attributes_int.end(), -1);

	return m_attributes_int[SENSORD_ATTRIBUTE_PAUSE_POLICY];
}

int sensor_listener::get_passive_mode(void)
{
	auto it = m_attributes_int.find(SENSORD_ATTRIBUTE_PASSIVE_MODE);
	retv_if(it == m_attributes_int.end(), -1);

	return m_attributes_int[SENSORD_ATTRIBUTE_PASSIVE_MODE];
}

int sensor_listener::set_interval(unsigned int interval)
{
	int _interval;

	/* TODO: move this logic to server */
	if (interval == 0)
		_interval = DEFAULT_INTERVAL;
	else if (interval < (unsigned int)m_sensor->get_min_interval())
		_interval = m_sensor->get_min_interval();
	else
		_interval = interval;

	_I("Listener[%d] set interval[%u]", get_id(), _interval);

	/* If it is not started, store the value only */
	if (!m_started.load()) {
		m_attributes_int[SENSORD_ATTRIBUTE_INTERVAL] = _interval;
		return OP_SUCCESS;
	}

	return set_attribute(SENSORD_ATTRIBUTE_INTERVAL, _interval);
}

int sensor_listener::set_max_batch_latency(unsigned int max_batch_latency)
{
	_I("Listener[%d] set max batch latency[%u]", get_id(), max_batch_latency);

	/* If it is not started, store the value only */
	if (!m_started.load()) {
		m_attributes_int[SENSORD_ATTRIBUTE_MAX_BATCH_LATENCY] = max_batch_latency;
		return OP_SUCCESS;
	}

	return set_attribute(SENSORD_ATTRIBUTE_MAX_BATCH_LATENCY, max_batch_latency);
}

int sensor_listener::set_passive_mode(bool passive)
{
	_I("Listener[%d] set passive mode[%d]", get_id(), passive);

	return set_attribute(SENSORD_ATTRIBUTE_PASSIVE_MODE, passive);
}

int sensor_listener::flush(void)
{
	_I("Listener[%d] flushes", get_id());

	return set_attribute(SENSORD_ATTRIBUTE_FLUSH, 1);
}

int sensor_listener::set_attribute(int attribute, int value)
{
	ipc::message msg;
	ipc::message reply;
	cmd_listener_attr_int_t buf;

	retvm_if(!m_cmd_channel, -EIO, "Failed to connect to server");

	buf.listener_id = m_id;
	buf.attribute = attribute;
	buf.value = value;
	msg.set_type(CMD_LISTENER_SET_ATTR_INT);
	msg.enclose((char *)&buf, sizeof(buf));

	m_cmd_channel->send_sync(msg);
	m_cmd_channel->read_sync(reply);

	if (reply.header()->err < 0)
		return reply.header()->err;

	if (attribute != SENSORD_ATTRIBUTE_FLUSH) {
		update_attribute(attribute, value);
	}

	return OP_SUCCESS;
}

int sensor_listener::get_attribute(int attribute, int* value)
{
	ipc::message msg;
	ipc::message reply;
	cmd_listener_attr_int_t buf;

	retvm_if(!m_cmd_channel, -EIO, "Failed to connect to server");

	buf.listener_id = m_id;
	buf.attribute = attribute;
	msg.set_type(CMD_LISTENER_GET_ATTR_INT);
	msg.enclose((char *)&buf, sizeof(buf));

	m_cmd_channel->send_sync(msg);
	m_cmd_channel->read_sync(reply);

	if (reply.header()->err < 0) {
		return reply.header()->err;
	}

	if (reply.header()->length && reply.body()) {
		*value = ((cmd_listener_attr_int_t *)reply.body())->value;
		return OP_SUCCESS;
	}

	return OP_ERROR;
}

void sensor_listener::update_attribute(int attribute, int value)
{
	AUTOLOCK(lock);
	m_attributes_int[attribute] = value;
	_I("Update_attribute(int) listener[%d] attribute[%d] value[%d] attributes size[%d]", get_id(), attribute, value, m_attributes_int.size());
}

int sensor_listener::set_attribute(int attribute, const char *value, int len)
{
	ipc::message msg;
	ipc::message reply;
	cmd_listener_attr_str_t *buf;
	size_t size;

	retvm_if(!m_cmd_channel, -EIO, "Failed to connect to server");

	size = sizeof(cmd_listener_attr_str_t) + len;

	buf = (cmd_listener_attr_str_t *) new(std::nothrow) char[size];
	retvm_if(!buf, -ENOMEM, "Failed to allocate memory");

	msg.set_type(CMD_LISTENER_SET_ATTR_STR);
	buf->listener_id = m_id;
	buf->attribute = attribute;

	memcpy(buf->value, value, len);
	buf->len = len;

	msg.enclose((char *)buf, size);

	m_cmd_channel->send_sync(msg);
	m_cmd_channel->read_sync(reply);

	/* Message memory is released automatically after sending message,
	   so it doesn't need to free memory */

	delete [] buf;

	if (reply.header()->err < 0)
		return reply.header()->err;

	update_attribute(attribute, value, len);

	return OP_SUCCESS;
}

int sensor_listener::get_attribute(int attribute, char **value, int* len)
{
	ipc::message msg;
	ipc::message reply;
	cmd_listener_attr_str_t buf;

	buf.listener_id = m_id;
	buf.attribute = attribute;

	msg.set_type(CMD_LISTENER_GET_ATTR_STR);
	msg.enclose((char *)&buf, sizeof(buf));
	m_cmd_channel->send_sync(msg);

	m_cmd_channel->read_sync(reply);
	if (reply.header()->err < 0) {
		return reply.header()->err;
	}

	if (reply.header()->length && reply.body()) {
		cmd_listener_attr_str_t * recv_buf = (cmd_listener_attr_str_t *)reply.body();
		char* p = (char *)recv_buf->value;
		*len = recv_buf->len;
		*value = (char *) malloc(*len);
		std::copy(p, p + recv_buf->len, *value);
		return OP_SUCCESS;
	}

	return OP_ERROR;
}

void sensor_listener::update_attribute(int attribute, const char *value, int len)
{
	AUTOLOCK(lock);
	m_attributes_str[attribute].clear();
	m_attributes_str[attribute].insert(m_attributes_str[attribute].begin(), value, value + len);
	_I("Update_attribute(str) listener[%d] attribute[%d] value[%s] attributes size[%zu]", get_id(), attribute, value, m_attributes_int.size());
}

int sensor_listener::get_sensor_data(sensor_data_t *data)
{
	ipc::message msg;
	ipc::message reply;
	cmd_listener_get_data_t buf;

	retvm_if(!m_cmd_channel, -EIO, "Failed to connect to server");

	buf.listener_id = m_id;
	msg.set_type(CMD_LISTENER_GET_DATA);
	msg.enclose((char *)&buf, sizeof(buf));

	m_cmd_channel->send_sync(msg);
	m_cmd_channel->read_sync(reply);

	if (reply.header()->err < 0) {
		return OP_ERROR;
	}

	reply.disclose((char *)&buf, sizeof(buf));
	int size = sizeof(sensor_data_t);

	if (buf.len > size || buf.len <= 0) {
		data->accuracy = -1;
		data->value_count = 0;
		return OP_ERROR;
	}

	memcpy(data, &buf.data, buf.len);

	_D("Listener[%d] read sensor data", get_id());

	return OP_SUCCESS;
}

int sensor_listener::get_sensor_data_list(sensor_data_t **data, int *count)
{
	ipc::message msg;
	ipc::message reply;
	cmd_listener_get_data_list_t buf;

	retvm_if(!m_cmd_channel, -EIO, "Failed to connect to server");

	buf.listener_id = m_id;
	msg.set_type(CMD_LISTENER_GET_DATA_LIST);
	msg.enclose((char *)&buf, sizeof(buf));

	m_cmd_channel->send_sync(msg);
	m_cmd_channel->read_sync(reply);

	if (reply.header()->err < 0) {
		return reply.header()->err;
	}

	size_t size = reply.size();
	cmd_listener_get_data_list_t* reply_buf = (cmd_listener_get_data_list_t *) new(std::nothrow) char[size];

	retvm_if(!reply_buf, -ENOMEM, "Failed to allocate memory");

	reply.disclose((char *)reply_buf, size);

	if (reply_buf->len <= 0) {
		delete [] reply_buf;
		return OP_ERROR;
	}

	*count = reply_buf->data_count;
	*data = (sensor_data_t*) malloc(reply_buf->len);

	if (!(*data)) {
		_E("Memory allocation failed");
		delete [] reply_buf;
		return -ENOMEM;
	}

	memcpy(*data, reply_buf->data, reply_buf->len);

	_D("Listener[%d] read sensor data list", get_id());
	delete [] reply_buf;
	return OP_SUCCESS;
}
