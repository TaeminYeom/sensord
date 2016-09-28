/*
 * sensord
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include <sensor_common.h>
#include <command_common.h>
#include <external_sensor_worker.h>
#include <external_client_manager.h>
#include <external_sensor.h>
#include <external_sensor_service.h>

using std::string;

external_sensor_worker::cmd_handler_t external_sensor_worker::m_cmd_handlers[];

external_sensor_worker::external_sensor_worker(const csocket& socket)
: m_client_id(CLIENT_ID_INVALID)
, m_socket(socket)
, m_sensor(NULL)
, m_sensor_id(UNKNOWN_SENSOR)
{
	static bool init = false;

	if (!init) {
		init_cmd_handlers();
		init = true;
	}

	m_worker.set_context(this);
	m_worker.set_working(working);
	m_worker.set_stopped(stopped);
}

external_sensor_worker::~external_sensor_worker()
{
	m_socket.close();
}

bool external_sensor_worker::start(void)
{
	return m_worker.start();
}

void external_sensor_worker::init_cmd_handlers(void)
{
	m_cmd_handlers[CMD_EXT_GET_ID]		= &external_sensor_worker::cmd_get_id;
	m_cmd_handlers[CMD_EXT_CONNECT]		= &external_sensor_worker::cmd_connect;
	m_cmd_handlers[CMD_EXT_DISCONNECT]	= &external_sensor_worker::cmd_disconnect;
	m_cmd_handlers[CMD_EXT_POST]		= &external_sensor_worker::cmd_post;
}

bool external_sensor_worker::working(void *ctx)
{
	bool ret;
	external_sensor_worker *inst = (external_sensor_worker *)ctx;

	packet_header header;
	char *payload;

	if (inst->m_socket.recv(&header, sizeof(header)) <= 0) {
		string info;
		inst->get_info(info);
		_D("%s failed to receive header", info.c_str());
		return false;
	}

	retvm_if(header.size == 0, false, "Invalid header size");

	payload = new(std::nothrow) char[header.size];
	retvm_if(!payload, false, "Failed to allocate memory");

	if (inst->m_socket.recv(payload, header.size) <= 0) {
		string info;
		inst->get_info(info);
		_D("%s failed to receive data of packet", info.c_str());
		delete[] payload;
		return false;
	}

	ret = inst->dispatch_command(header.cmd, payload);

	delete[] payload;

	return ret;
}

bool external_sensor_worker::stopped(void *ctx)
{
	string info;
	external_sensor_worker *inst = (external_sensor_worker *)ctx;

	inst->get_info(info);
	_I("%s is stopped", info.c_str());

	if (inst->m_client_id != CLIENT_ID_INVALID) {
		_I("Client:%d leaves without disconnecting", inst->m_client_id);
		if (get_client_manager().has_sensor_record(inst->m_client_id, inst->m_sensor_id)) {
			_I("Removing sensor[%#x] record for client_id[%d]", inst->m_sensor_id, inst->m_client_id);
			get_client_manager().remove_sensor_record(inst->m_client_id, inst->m_sensor_id);

			if (inst->m_sensor)
				inst->m_sensor->set_source_connected(false);
		}
	}

	delete inst;
	return true;
}

bool external_sensor_worker::dispatch_command(int cmd, void* payload)
{
	int ret = false;

	if (!(cmd > 0 && cmd < CMD_EXT_CNT)) {
		_E("Unknown command: %d", cmd);
	} else {
		cmd_handler_t cmd_handler;
		cmd_handler = external_sensor_worker::m_cmd_handlers[cmd];
		if (cmd_handler)
			ret = (this->*cmd_handler)(payload);
	}

	return ret;
}

bool external_sensor_worker::send_cmd_done(long value)
{
	cpacket* ret_packet;
	cmd_ext_done_t *cmd_ext_done;

	ret_packet = new(std::nothrow) cpacket(sizeof(cmd_ext_done_t));
	retvm_if(!ret_packet, false, "Failed to allocate memory");

	ret_packet->set_cmd(CMD_EXT_DONE);

	cmd_ext_done = (cmd_ext_done_t*)ret_packet->data();
	cmd_ext_done->value = value;

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		_E("Failed to send a cmd_done to client_id [%d] with value [%ld]", m_client_id, value);
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}

bool external_sensor_worker::send_cmd_get_id_done(int client_id)
{
	cpacket* ret_packet;
	cmd_ext_get_id_done_t *cmd_ext_get_id_done;

	ret_packet = new(std::nothrow) cpacket(sizeof(cmd_ext_get_id_done_t));
	retvm_if(!ret_packet, false, "Failed to allocate memory");

	ret_packet->set_cmd(CMD_EXT_GET_ID);

	cmd_ext_get_id_done = (cmd_ext_get_id_done_t*)ret_packet->data();
	cmd_ext_get_id_done->client_id = client_id;

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		_E("Failed to send a cmd_get_id_done with client_id [%d]", client_id);
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}

bool external_sensor_worker::send_cmd_connect_done(sensor_id_t sensor_id)
{
	cpacket* ret_packet;
	cmd_ext_connect_done_t *cmd_ext_connect_done;

	ret_packet = new(std::nothrow) cpacket(sizeof(cmd_ext_connect_done_t));
	retvm_if(!ret_packet, false, "Failed to allocate memory");

	ret_packet->set_cmd(CMD_EXT_CONNECT);

	cmd_ext_connect_done = (cmd_ext_connect_done_t*)ret_packet->data();
	cmd_ext_connect_done->sensor_id = sensor_id;

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		_E("Failed to send a cmd_connect done");
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}

bool external_sensor_worker::cmd_get_id(void *payload)
{
	cmd_ext_get_id_t *cmd = static_cast<cmd_ext_get_id_t *>(payload);
	int client_id;
	struct ucred cr;
	socklen_t opt_len = sizeof(cr);

	if (getsockopt(m_socket.get_socket_fd(), SOL_SOCKET, SO_PEERCRED, &cr, &opt_len)) {
		_E("Failed to get socket option with SO_PEERCRED");
		return false;
	}

	client_id = get_client_manager().create_client_record();

	if (client_id != MAX_HANDLE_REACHED) {
		get_client_manager().set_client_info(client_id, cr.pid, cmd->name);
		_I("New client id [%d] created", client_id);
	}

	if (!send_cmd_get_id_done(client_id))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool external_sensor_worker::cmd_connect(void *payload)
{
	cmd_ext_connect_t *cmd = static_cast<cmd_ext_connect_t *>(payload);
	m_client_id = cmd->client_id;

	external_sensor *sensor;
	sensor = external_sensor_service::get_instance().get_sensor(string(cmd->key));
	if (!sensor) {
		_E("No matched external sensor with key: %s", cmd->key);
		goto out;
	}

	if (!sensor->set_source_connected(true)) {
		_E("External sensor(%s) is already connected", cmd->key);
		goto out;
	}

	m_sensor = sensor;
	m_sensor_id = sensor->get_id();

	if (!get_client_manager().create_sensor_record(m_client_id, m_sensor_id)) {
		_E("Failed to create sensor record for client: %d, sensor_id: %d", m_client_id, m_sensor_id);
		m_sensor_id = UNKNOWN_SENSOR;
		goto out;
	}

out:
	if (!send_cmd_connect_done(m_sensor_id))
		_E("Failed to send cmd_connect_done to a client : %d", m_client_id);

	return true;
}

bool external_sensor_worker::cmd_disconnect(void *payload)
{
	long ret_value = OP_ERROR;

	if (!m_sensor) {
		_E("External sensor is not connected");
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_manager().remove_sensor_record(m_client_id, m_sensor_id)) {
		_E("Failed to remove sensor record for client [%d]", m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	m_sensor->set_source_connected(false);

	m_sensor = NULL;
	m_client_id = CLIENT_ID_INVALID;
	m_sensor_id = UNKNOWN_SENSOR;
	ret_value = OP_SUCCESS;
out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	if (ret_value == OP_SUCCESS)
		return false;

	return true;
}

bool external_sensor_worker::cmd_post(void *payload)
{
	long ret_value = OP_SUCCESS;
	cmd_ext_post_t *cmd = static_cast<cmd_ext_post_t *>(payload);

	m_sensor->on_receive(cmd->timestamp, cmd->data, cmd->data_cnt);

	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

external_client_manager& external_sensor_worker::get_client_manager(void)
{
	return external_client_manager::get_instance();
}

void external_sensor_worker::get_info(string &info)
{
	const char *client_info = NULL;

	if (m_client_id != CLIENT_ID_INVALID)
		client_info = get_client_manager().get_client_info(m_client_id);

	info = string("Command worker for ") + (client_info ? string(client_info) : string("Unknown"));
}
