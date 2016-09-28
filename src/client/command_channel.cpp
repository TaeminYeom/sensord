/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#include <stdlib.h>
#include <command_channel.h>
#include <client_common.h>
#include <command_common.h>
#include <sensor_info.h>
#include <sensor_info_manager.h>

command_channel::command_channel()
: m_client_id(CLIENT_ID_INVALID)
, m_sensor_id(UNKNOWN_SENSOR)
{
}

command_channel::~command_channel()
{
	if (m_command_socket.is_valid())
		m_command_socket.close();
}

bool command_channel::command_handler(cpacket *packet, void **return_payload)
{
	if (!m_command_socket.is_valid()) {
		_E("Command socket(%d) is not valid for client %s", m_command_socket.get_socket_fd(), get_client_name());
		return false;
	}

	if (packet->size() == 0) {
		_E("Packet is not valid for client %s", get_client_name());
		return false;
	}

	if (m_command_socket.send(packet->packet(), packet->size()) <= 0) {
		m_command_socket.close();
		_E("Failed to send command in client %s", get_client_name());
		return false;
	}

	packet_header header;

	if (m_command_socket.recv(&header, sizeof(header)) <= 0) {
		m_command_socket.close();
		_E("Failed to receive header for command reply packet in client %s", get_client_name());
		return false;
	}

	char *buffer = new(std::nothrow) char[header.size];
	retvm_if(!buffer, false, "Failed to allocate memory");

	if (m_command_socket.recv(buffer, header.size) <= 0) {
		m_command_socket.close();
		_E("Failed to receive command reply packet in client %s", get_client_name());
		delete[] buffer;
		return false;
	}

	*return_payload = buffer;

	return true;
}

bool command_channel::create_channel(void)
{
	const int cllient_type = CLIENT_TYPE_SENSOR_CLIENT;
	if (!m_command_socket.create(SOCK_STREAM))
		return false;

	if (!m_command_socket.connect(COMMAND_CHANNEL_PATH)) {
		_E("Failed to connect command channel for client %s, command socket fd[%d]", get_client_name(), m_command_socket.get_socket_fd());
		return false;
	}

	m_command_socket.set_connection_mode();

	if (m_command_socket.send(&cllient_type, sizeof(cllient_type)) <= 0) {
		_E("Failed to send client type in client %s, command socket fd[%d]", get_client_name(), m_command_socket.get_socket_fd());
		return false;
	}

	return true;
}

void command_channel::set_client_id(int client_id)
{
	m_client_id = client_id;
}

bool command_channel::cmd_get_id(int &client_id)
{
	cpacket *packet;
	cmd_get_id_t *cmd_get_id;
	cmd_get_id_done_t *cmd_get_id_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_get_id_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_GET_ID);

	cmd_get_id = (cmd_get_id_t *)packet->data();

	get_proc_name(getpid(), cmd_get_id->name);

	_I("%s send cmd_get_id()", get_client_name());

	if (!command_handler(packet, (void **)&cmd_get_id_done)) {
		_E("Client %s failed to send/receive command", get_client_name());
		delete packet;
		return false;
	}

	if (cmd_get_id_done->client_id < 0) {
		_E("Client %s failed to get client_id[%d] from server",
			get_client_name(), cmd_get_id_done->client_id);
		delete[] (char *)cmd_get_id_done;
		delete packet;
		return false;
	}

	client_id = cmd_get_id_done->client_id;

	delete[] (char *)cmd_get_id_done;
	delete packet;

	return true;
}

bool command_channel::cmd_get_sensor_list(void)
{
	cpacket packet;
	cmd_get_sensor_list_done_t *cmd_get_sensor_list_done;

	packet.set_payload_size(sizeof(cmd_get_sensor_list_t));
	packet.set_cmd(CMD_GET_SENSOR_LIST);

	_I("%s send cmd_get_sensor_list", get_client_name());

	if (!command_handler(&packet, (void **)&cmd_get_sensor_list_done)) {
		_E("Client %s failed to send/receive command", get_client_name());
		return false;
	}

	int sensor_cnt;
	const size_t *size_field;
	const char *raw_data_field;

	sensor_cnt = cmd_get_sensor_list_done->sensor_cnt;
	size_field = (const size_t *)cmd_get_sensor_list_done->data;
	raw_data_field = (const char *)size_field + (sizeof(size_t) * sensor_cnt);

	sensor_info *info;

	int idx = 0;
	for (int i = 0; i < sensor_cnt; ++i) {
		info = new(std::nothrow) sensor_info;

		if (!info) {
			_E("Failed to allocate memory");
			delete[] (char *)cmd_get_sensor_list_done;
			return false;
		}

		info->set_raw_data(raw_data_field + idx, size_field[i]);
		sensor_info_manager::get_instance().add_info(info);
		idx += size_field[i];
	}

	delete[] (char *)cmd_get_sensor_list_done;
	return true;
}

bool command_channel::cmd_hello(sensor_id_t sensor)
{
	cpacket *packet;
	cmd_hello_t *cmd_hello;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_hello_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_HELLO);

	cmd_hello = (cmd_hello_t*)packet->data();
	cmd_hello->client_id = m_client_id;
	cmd_hello->sensor = sensor;

	_I("%s send cmd_hello(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(sensor));

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("Client %s failed to send/receive command for sensor[%s]",
			get_client_name(), get_sensor_name(sensor));
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("client %s got error[%d] from server with sensor [%s]",
			get_client_name(), cmd_done->value, get_sensor_name(sensor));

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	m_sensor_id = sensor;

	return true;
}

bool command_channel::cmd_byebye(void)
{
	cpacket *packet;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_byebye_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_BYEBYE);

	_I("%s send cmd_byebye(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_id));

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("Client %s failed to send/receive command for sensor[%s] with client_id [%d]",
			get_client_name(), get_sensor_name(m_sensor_id), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("Client %s got error[%d] from server for sensor[%s] with client_id [%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_id), m_client_id);

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	if (m_command_socket.is_valid())
		m_command_socket.close();

	m_client_id = CLIENT_ID_INVALID;
	m_sensor_id = UNKNOWN_SENSOR;

	return true;
}

bool command_channel::cmd_start(void)
{
	cpacket *packet;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_start_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_START);

	_I("%s send cmd_start(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_id));

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("Client %s failed to send/receive command for sensor[%s] with client_id [%d]",
			get_client_name(), get_sensor_name(m_sensor_id), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("Client %s got error[%d] from server for sensor[%s] with client_id [%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_id), m_client_id);

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	return true;
}

bool command_channel::cmd_stop(void)
{
	cpacket *packet;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_stop_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_STOP);

	_I("%s send cmd_stop(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_id));

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("Client %s failed to send/receive command for sensor[%s] with client_id [%d]",
			get_client_name(), get_sensor_name(m_sensor_id), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("Client %s got error[%d] from server for sensor[%s] with client_id [%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_id), m_client_id);

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	return true;
}

bool command_channel::cmd_set_pause_policy(int pause_policy)
{
	cpacket *packet;
	cmd_set_pause_policy_t *cmd_set_pause_policy;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_set_pause_policy_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_SET_PAUSE_POLICY);

	cmd_set_pause_policy = (cmd_set_pause_policy_t*)packet->data();
	cmd_set_pause_policy->pause_policy = pause_policy;

	_I("%s send cmd_set_pause_policy(client_id=%d, %s, pause_policy=%d)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_id), pause_policy);

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("Client %s failed to send/receive command for sensor[%s] with client_id [%d], pause_policy[%d]",
			get_client_name(), get_sensor_name(m_sensor_id), m_client_id, pause_policy);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("Client %s got error[%d] from server for sensor[%s] with client_id [%d], pause_policy[%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_id), m_client_id, pause_policy);

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	return true;
}

bool command_channel::cmd_register_event(unsigned int event_type)
{
	cpacket *packet;
	cmd_reg_t *cmd_reg;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_reg_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_REG);

	cmd_reg = (cmd_reg_t*)packet->data();
	cmd_reg->event_type = event_type;

	_I("%s send cmd_register_event(client_id=%d, %s)",
		get_client_name(), m_client_id, get_event_name(event_type));

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("Client %s failed to send/receive command with client_id [%d], event_type[%s]",
			get_client_name(), m_client_id, get_event_name(event_type));
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("Client %s got error[%d] from server with client_id [%d], event_type[%s]",
			get_client_name(), cmd_done->value, m_client_id, get_event_name(event_type));

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	return true;
}

bool command_channel::cmd_register_events(event_type_vector &event_vec)
{
	auto it_event = event_vec.begin();

	while (it_event != event_vec.end()) {
		if (!cmd_register_event(*it_event))
			return false;

		++it_event;
	}

	return true;
}

bool command_channel::cmd_unregister_event(unsigned int event_type)
{
	cpacket *packet;
	cmd_unreg_t *cmd_unreg;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_unreg_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_UNREG);

	cmd_unreg = (cmd_unreg_t*)packet->data();
	cmd_unreg->event_type = event_type;

	_I("%s send cmd_unregister_event(client_id=%d, %s)",
		get_client_name(), m_client_id, get_event_name(event_type));

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("Client %s failed to send/receive command with client_id [%d], event_type[%s]",
			get_client_name(), m_client_id, get_event_name(event_type));
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("Client %s got error[%d] from server with client_id [%d], event_type[%s]",
			get_client_name(), cmd_done->value, m_client_id, get_event_name(event_type));

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	return true;
}

bool command_channel::cmd_unregister_events(event_type_vector &event_vec)
{
	auto it_event = event_vec.begin();

	while (it_event != event_vec.end()) {
		if (!cmd_unregister_event(*it_event))
			return false;

		++it_event;
	}

	return true;
}

bool command_channel::cmd_set_batch(unsigned int interval, unsigned int latency)
{
	cpacket *packet;
	cmd_set_batch_t *cmd_set_batch;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_set_batch_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_SET_BATCH);

	cmd_set_batch = (cmd_set_batch_t*)packet->data();
	cmd_set_batch->interval = interval;
	cmd_set_batch->latency = latency;

	_I("%s send cmd_set_batch(client_id=%d, %s, interval=%d, latency = %d)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_id), interval, latency);

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("%s failed to send/receive command for sensor[%s] with client_id [%d], interval[%d], latency[%d]",
			get_client_name(), get_sensor_name(m_sensor_id), m_client_id, interval, latency);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("%s got error[%d] from server for sensor[%s] with client_id [%d], interval[%d], latency[%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_id), m_client_id, interval, latency);

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	return true;
}

bool command_channel::cmd_unset_batch(void)
{
	cpacket *packet;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_unset_batch_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_UNSET_BATCH);

	_I("%s send cmd_unset_batch(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_id));

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("Client %s failed to send/receive command for sensor[%s] with client_id [%d]",
			get_client_name(), get_sensor_name(m_sensor_id), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("Client %s got error[%d] from server for sensor[%s] with client_id [%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_id), m_client_id);

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	return true;
}

bool command_channel::cmd_get_data(unsigned int type, sensor_data_t* sensor_data)
{
	cpacket *packet;
	cmd_get_data_t *cmd_get_data;
	cmd_get_data_done_t *cmd_get_data_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_get_data_done_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_GET_DATA);

	cmd_get_data = (cmd_get_data_t*)packet->data();
	cmd_get_data->type = type;

	if (!command_handler(packet, (void **)&cmd_get_data_done)) {
		_E("Client %s failed to send/receive command with client_id [%d]",
			get_client_name(), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_get_data_done->state < 0 ) {
		_E("Client %s got error[%d] from server with client_id [%d]",
			get_client_name(), cmd_get_data_done->state, m_client_id);
		sensor_data->accuracy = SENSOR_ACCURACY_UNDEFINED;
		sensor_data->timestamp = 0;
		sensor_data->value_count = 0;
		delete[] (char *)cmd_get_data_done;
		delete packet;
		return false;
	}

	memcpy(sensor_data, &cmd_get_data_done->base_data, sizeof(sensor_data_t));

	delete[] (char *)cmd_get_data_done;
	delete packet;

	return true;
}

bool command_channel::cmd_set_attribute_int(int attribute, int value)
{
	cpacket *packet;
	cmd_set_attribute_int_t *cmd_set_attribute_int;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_set_attribute_int_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_SET_ATTRIBUTE_INT);

	cmd_set_attribute_int = (cmd_set_attribute_int_t*)packet->data();
	cmd_set_attribute_int->attribute = attribute;
	cmd_set_attribute_int->value = value;

	_I("%s send cmd_set_attribute_int(client_id=%d, %s, %#x, %d)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_id), attribute, value);

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("Client %s failed to send/receive command for sensor[%s] with client_id [%d], attribute[%#x], value[%d]",
			get_client_name(), get_sensor_name(m_sensor_id), m_client_id, attribute, value);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("Client %s got error[%d] from server for sensor[%s] with attribute[%#x], value[%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_id), attribute, value);

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	return true;
}

bool command_channel::cmd_set_attribute_str(int attribute, const char* value, int len)
{
	cpacket *packet;
	cmd_set_attribute_str_t *cmd_set_attribute_str;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_set_attribute_str_t) + len);
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_SET_ATTRIBUTE_STR);

	cmd_set_attribute_str = (cmd_set_attribute_str_t*)packet->data();
	cmd_set_attribute_str->attribute = attribute;
	cmd_set_attribute_str->len = len;
	memcpy(cmd_set_attribute_str->value, value, len);

	_I("%s send cmd_set_attribute_str(client_id=%d, attribute = %#x, value_len = %d, value = %#x)",
		get_client_name(), m_client_id, attribute, len, value);

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("%s failed to send/receive command with client_id [%d]",
			get_client_name(), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("%s got error[%d] from server with client_id [%d]",
			get_client_name(), cmd_done->value, m_client_id);

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

bool command_channel::cmd_flush(void)
{
	cpacket *packet;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_flush_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_FLUSH);

	_I("%s send cmd_flush(client_id=%d)", get_client_name(), m_client_id);

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("%s failed to send flush with client_id [%d]",
			get_client_name(), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("%s got error[%d] from server with client_id [%d]",
			get_client_name(), cmd_done->value, m_client_id);

		delete [] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete [] (char *)cmd_done;
	delete packet;
	return true;
}
