/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <sensor_loader.h>
#include <sensor_info.h>
#include <sensor_log.h>
#include <thread>
#include <string>
#include <vector>
#include <utility>
#include <permission_checker.h>
#include <command_worker.h>

#define WAIT_TIME(X) ((1 << ((X) < 4 ? (X) : 4)) * 10000)	/* 20, 40, 80, 160, 160ms, ... */

using std::string;
using std::vector;
using std::make_pair;

command_worker::cmd_handler_t command_worker::m_cmd_handlers[];
sensor_raw_data_map command_worker::m_sensor_raw_data_map;
cpacket command_worker::m_sensor_list;
cmutex command_worker::m_shared_mutex;

command_worker::command_worker(const csocket& socket)
: m_client_id(CLIENT_ID_INVALID)
, m_permission(SENSOR_PERMISSION_NONE)
, m_socket(socket)
, m_module(NULL)
, m_sensor_id(UNKNOWN_SENSOR)
{
	static bool init = false;

	AUTOLOCK(m_shared_mutex);

	if (!init) {
		init_cmd_handlers();

		init = true;
	}

	m_worker.set_context(this);
	m_worker.set_working(working);
	m_worker.set_stopped(stopped);
}

command_worker::~command_worker()
{
	m_socket.close();
}

bool command_worker::start(void)
{
	return m_worker.start();
}

void command_worker::init_cmd_handlers(void)
{
	m_cmd_handlers[CMD_GET_ID]				= &command_worker::cmd_get_id;
	m_cmd_handlers[CMD_GET_SENSOR_LIST]		= &command_worker::cmd_get_sensor_list;
	m_cmd_handlers[CMD_HELLO]				= &command_worker::cmd_hello;
	m_cmd_handlers[CMD_BYEBYE]				= &command_worker::cmd_byebye;
	m_cmd_handlers[CMD_START]				= &command_worker::cmd_start;
	m_cmd_handlers[CMD_STOP]				= &command_worker::cmd_stop;
	m_cmd_handlers[CMD_REG]					= &command_worker::cmd_register_event;
	m_cmd_handlers[CMD_UNREG]				= &command_worker::cmd_unregister_event;
	m_cmd_handlers[CMD_SET_PAUSE_POLICY]	= &command_worker::cmd_set_pause_policy;
	m_cmd_handlers[CMD_SET_BATCH]			= &command_worker::cmd_set_batch;
	m_cmd_handlers[CMD_UNSET_BATCH]			= &command_worker::cmd_unset_batch;
	m_cmd_handlers[CMD_GET_DATA]			= &command_worker::cmd_get_data;
	m_cmd_handlers[CMD_SET_ATTRIBUTE_INT]	= &command_worker::cmd_set_attribute_int;
	m_cmd_handlers[CMD_SET_ATTRIBUTE_STR]	= &command_worker::cmd_set_attribute_str;
	m_cmd_handlers[CMD_FLUSH]				= &command_worker::cmd_flush;
}

int command_worker::create_sensor_raw_list(int client_perms, std::vector<raw_data_t *> &raw_list)
{
	size_t total_raw_data_size = 0;
	vector<sensor_base *> sensors;
	vector<sensor_type_t> types;
	sensor_info info;
	int permission;

	types = sensor_loader::get_instance().get_sensor_types();

	for (auto it_type = types.begin(); it_type != types.end(); ++it_type) {
		sensor_type_t type = *it_type;
		sensors = sensor_loader::get_instance().get_sensors(type);

		for (auto it_sensor = sensors.begin(); it_sensor != sensors.end(); ++it_sensor) {
			(*it_sensor)->get_sensor_info(info);
			permission = (*it_sensor)->get_permission();
			permission = (client_perms & permission);

			if (!permission) {
				info.clear();
				info.set_id((sensor_id_t)-EACCES);
				info.set_type(type);
			}

			raw_data_t *raw_data = new(std::nothrow) raw_data_t();
			retvm_if(!raw_data, -1, "Failed to allocated memory");
			info.get_raw_data(*raw_data);

			total_raw_data_size += raw_data->size();
			raw_list.push_back(raw_data);

			info.clear();
		}
	}

	return total_raw_data_size;
}

void command_worker::get_sensor_list(int client_perms, cpacket &sensor_list)
{
	size_t total_raw_data_size = 0;
	vector<raw_data_t *> raw_list;
	int sensor_cnt;
	int idx = 0;

	total_raw_data_size = create_sensor_raw_list(client_perms, raw_list);

	sensor_cnt = raw_list.size();

	sensor_list.set_payload_size(sizeof(cmd_get_sensor_list_done_t) + (sizeof(size_t) * sensor_cnt) + total_raw_data_size);
	sensor_list.set_cmd(CMD_GET_SENSOR_LIST);

	cmd_get_sensor_list_done_t *cmd_get_sensor_list_done;

	cmd_get_sensor_list_done = (cmd_get_sensor_list_done_t*)sensor_list.data();
	cmd_get_sensor_list_done->sensor_cnt = sensor_cnt;
	size_t* size_field = (size_t *) cmd_get_sensor_list_done->data;

	for (int i = 0; i < sensor_cnt; ++i)
		size_field[i] = raw_list[i]->size();

	char* raw_data_field = cmd_get_sensor_list_done->data + (sizeof(size_t) * sensor_cnt);

	for (int i = 0; i < sensor_cnt; ++i) {
		copy(raw_list[i]->begin(), raw_list[i]->end(), raw_data_field + idx);
		idx += raw_list[i]->size();
	}

	for (auto it = raw_list.begin(); it != raw_list.end(); ++it)
		delete *it;
}

bool command_worker::working(void *ctx)
{
	bool ret;
	command_worker *inst = (command_worker *)ctx;

	packet_header header;
	char *payload;

	if (inst->m_socket.recv(&header, sizeof(header)) <= 0) {
		string info;
		inst->get_info(info);
		_D("%s failed to receive header", info.c_str());
		return false;
	}

	if (header.size > 0) {
		payload = new(std::nothrow) char[header.size];
		retvm_if(!payload, false, "Failed to allocate memory");

		if (inst->m_socket.recv(payload, header.size) <= 0) {
			string info;
			inst->get_info(info);
			_D("%s failed to receive data of packet", info.c_str());
			delete[] payload;
			return false;
		}
	} else {
		payload = NULL;
	}

	ret = inst->dispatch_command(header.cmd, payload);

	if (payload)
		delete[] payload;

	return ret;
}

bool command_worker::stopped(void *ctx)
{
	string info;
	command_worker *inst = (command_worker *)ctx;

	inst->get_info(info);
	_I("%s is stopped", info.c_str());

	if ((inst->m_module) && (inst->m_client_id != CLIENT_ID_INVALID)) {
		if (get_client_info_manager().is_started(inst->m_client_id, inst->m_sensor_id)) {
			_W("Does not receive cmd_stop before connection broken for [%s]!!", inst->m_module->get_name());
			inst->m_module->delete_interval(inst->m_client_id, false);
			inst->m_module->delete_batch(inst->m_client_id);
			inst->m_module->delete_attribute(inst->m_client_id);
			inst->m_module->stop();
		}

		if (inst->m_sensor_id > UNKNOWN_SENSOR) {
			if (get_client_info_manager().has_sensor_record(inst->m_client_id, inst->m_sensor_id)) {
				_I("Removing sensor[%#llx] record for client_id[%d]", inst->m_sensor_id, inst->m_client_id);
				get_client_info_manager().remove_sensor_record(inst->m_client_id, inst->m_sensor_id);
			}
		}
	}

	delete inst;
	return true;
}

bool command_worker::dispatch_command(int cmd, void* payload)
{
	int ret = false;

	if (!(cmd > 0 && cmd < CMD_CNT)) {
		_E("Unknown command: %d", cmd);
	} else {
		cmd_handler_t cmd_handler;
		cmd_handler = command_worker::m_cmd_handlers[cmd];
		if (cmd_handler)
			ret = (this->*cmd_handler)(payload);
	}

	return ret;
}

bool command_worker::send_cmd_done(long value)
{
	cpacket* ret_packet;
	cmd_done_t *cmd_done;

	ret_packet = new(std::nothrow) cpacket(sizeof(cmd_done_t));
	retvm_if(!ret_packet, false, "Failed to allocate memory");

	ret_packet->set_cmd(CMD_DONE);

	cmd_done = (cmd_done_t*)ret_packet->data();
	cmd_done->value = value;

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		_E("Failed to send a cmd_done to client_id [%d] with value [%ld]", m_client_id, value);
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}

bool command_worker::send_cmd_get_id_done(int client_id)
{
	cpacket* ret_packet;
	cmd_get_id_done_t *cmd_get_id_done;

	ret_packet = new(std::nothrow) cpacket(sizeof(cmd_get_id_done_t));
	retvm_if(!ret_packet, false, "Failed to allocate memory");

	ret_packet->set_cmd(CMD_GET_ID);

	cmd_get_id_done = (cmd_get_id_done_t*)ret_packet->data();
	cmd_get_id_done->client_id = client_id;

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		_E("Failed to send a cmd_get_id_done with client_id [%d]", client_id);
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}

bool command_worker::send_cmd_get_data_done(int state, sensor_data_t *data)
{
	cpacket* ret_packet;
	cmd_get_data_done_t *cmd_get_data_done;

	ret_packet = new(std::nothrow) cpacket(sizeof(cmd_get_data_done_t));
	retvm_if(!ret_packet, false, "Failed to allocate memory");

	ret_packet->set_cmd(CMD_GET_DATA);

	cmd_get_data_done = (cmd_get_data_done_t*)ret_packet->data();
	cmd_get_data_done->state = state;

	if (data)
		memcpy(&cmd_get_data_done->base_data, data, sizeof(sensor_data_t));

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		_E("Failed to send a cmd_get_data_done");
		free(data);
		delete ret_packet;
		return false;
	}

	free(data);
	delete ret_packet;
	return true;
}

bool command_worker::send_cmd_get_sensor_list_done(void)
{
	cpacket sensor_list;

	int permission = get_permission();

	_I("permission = %#x", permission);

	get_sensor_list(permission, sensor_list);

	if (m_socket.send(sensor_list.packet(), sensor_list.size()) <= 0) {
		_E("Failed to send a cmd_get_sensor_list_done");
		return false;
	}

	/*
	 * TODO: get_sensor_list() command is processed on one-time connection in the current architecture.
	 *       but it doesn't need to use one-time connection.
	 */
	m_socket.close();

	return true;
}

bool command_worker::cmd_get_id(void *payload)
{
	cmd_get_id_t *cmd;
	int client_id;
	struct ucred cr;
	socklen_t opt_len = sizeof(cr);

	_D("CMD_GET_ID Handler invoked\n");
	cmd = (cmd_get_id_t*)payload;

	if (getsockopt(m_socket.get_socket_fd(), SOL_SOCKET, SO_PEERCRED, &cr, &opt_len)) {
		_E("Failed to get socket option with SO_PEERCRED");
		return false;
	}

	client_id = get_client_info_manager().create_client_record();

	get_client_info_manager().set_client_info(client_id, cr.pid, cmd->name);

	m_permission = get_permission();

	get_client_info_manager().set_permission(client_id, m_permission);

	_I("New client id [%d] created", client_id);

	if (!send_cmd_get_id_done(client_id))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_get_sensor_list(void *payload)
{
	_D("CMD_GET_SENSOR_LIST Handler invoked\n");

	if (!send_cmd_get_sensor_list_done())
		_E("Failed to send cmd_get_sensor_list_done to a client");

	return true;
}

bool command_worker::cmd_hello(void *payload)
{
	cmd_hello_t *cmd;
	long ret_value = OP_ERROR;

	_D("CMD_HELLO Handler invoked\n");
	cmd = (cmd_hello_t*)payload;

	m_sensor_id = cmd->sensor;
	m_client_id = cmd->client_id;

	if (m_permission == SENSOR_PERMISSION_NONE)
		get_client_info_manager().get_permission(m_client_id, m_permission);

	m_module = (sensor_base *)sensor_loader::get_instance().get_sensor(cmd->sensor);

	if (!m_module) {
		_W("Sensor type[%d] is not supported", cmd->sensor);
		if (!get_client_info_manager().has_sensor_record(m_client_id))
			get_client_info_manager().remove_client_record(m_client_id);

		ret_value = OP_ERROR;
		goto out;
	}

	if (!is_permission_allowed()) {
		_E("Permission denied to connect sensor[%#llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	_D("Hello sensor [%#llx], client id [%d]", m_sensor_id, m_client_id);
	get_client_info_manager().create_sensor_record(m_client_id, m_sensor_id);
	_I("New sensor record created for sensor [%#llx], sensor name [%s] on client id [%d]\n", m_sensor_id, m_module->get_name(), m_client_id);
	ret_value = OP_SUCCESS;
out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_byebye(void *payload)
{
	long ret_value = OP_ERROR;

	if (!is_permission_allowed()) {
		_E("Permission denied to stop sensor[%#llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	_D("CMD_BYEBYE for client [%d], sensor [%#llx]", m_client_id, m_sensor_id);

	if (!get_client_info_manager().remove_sensor_record(m_client_id, m_sensor_id)) {
		_E("Error removing sensor_record for client [%d]", m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	m_client_id = CLIENT_ID_INVALID;
	ret_value = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	if (ret_value == OP_SUCCESS)
		return false;

	return true;
}

bool command_worker::cmd_start(void *payload)
{
	long ret_value = OP_ERROR;

	if (!is_permission_allowed()) {
		_E("Permission denied to start sensor[%#llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	_D("START Sensor [%#llx], called from client [%d]", m_sensor_id, m_client_id);

	if (m_module->start()) {
		get_client_info_manager().set_start(m_client_id, m_sensor_id, true);
/*
 *	Rotation could be changed even LCD is off by pop sync rotation
 *	and a client listening rotation event with always-on option.
 *	To reflect the last rotation state, request it to event dispatcher.
 */
		get_event_dispathcher().request_last_event(m_client_id, m_sensor_id);
		ret_value = OP_SUCCESS;
	} else {
		_E("Failed to start sensor [%#llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
	}

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_stop(void *payload)
{
	long ret_value = OP_ERROR;

	if (!is_permission_allowed()) {
		_E("Permission denied to stop sensor[%#llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	_D("STOP Sensor [%#llx], called from client [%d]", m_sensor_id, m_client_id);

	if (m_module->stop()) {
		get_client_info_manager().set_start(m_client_id, m_sensor_id, false);
		m_module->delete_attribute(m_client_id);
		ret_value = OP_SUCCESS;
	} else {
		_E("Failed to stop sensor [%#llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
	}

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_register_event(void *payload)
{
	cmd_reg_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_reg_t*)payload;

	if (!is_permission_allowed()) {
		_E("Permission denied to register event [%#x] for client [%d] to client info manager",
			cmd->event_type, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().register_event(m_client_id, m_sensor_id, cmd->event_type)) {
		_I("Failed to register event [%#x] for client [%d] to client info manager",
			cmd->event_type, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;
	_D("Registering Event [%#x] is done for client [%d]", cmd->event_type, m_client_id);

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_unregister_event(void *payload)
{
	cmd_unreg_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_unreg_t*)payload;

	if (!is_permission_allowed()) {
		_E("Permission denied to unregister event [%#x] for client [%d] to client info manager",
			cmd->event_type, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().unregister_event(m_client_id, m_sensor_id, cmd->event_type)) {
		_E("Failed to unregister event [%#x] for client [%d] from client info manager",
			cmd->event_type, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;
	_D("Unregistering Event [%#x] is done for client [%d]",
		cmd->event_type, m_client_id);

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_batch(void *payload)
{
	cmd_set_batch_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_set_batch_t*)payload;

	if (!is_permission_allowed()) {
		_E("Permission denied to set batch for client [%d], for sensor [%#llx] with batch [%d, %d] to client info manager",
			m_client_id, m_sensor_id, cmd->interval, cmd->latency);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().set_batch(m_client_id, m_sensor_id, cmd->interval, cmd->latency)) {
		_E("Failed to set batch for client [%d], for sensor [%#llx] with batch [%d, %d] to client info manager",
			m_client_id, m_sensor_id, cmd->interval, cmd->latency);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!m_module->add_interval(m_client_id, cmd->interval, false)) {
		_E("Failed to set interval for client [%d], for sensor [%#llx] with interval [%d]",
			m_client_id, m_sensor_id, cmd->interval);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!m_module->add_batch(m_client_id, cmd->latency)) {
		_E("Failed to set latency for client [%d], for sensor [%#llx] with latency [%d]",
			m_client_id, m_sensor_id, cmd->latency);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_unset_batch(void *payload)
{
	long ret_value = OP_ERROR;

	if (!is_permission_allowed()) {
		_E("Permission denied to unset batch for client [%d], for sensor [%#llx] to client info manager",
			m_client_id, m_sensor_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().set_batch(m_client_id, m_sensor_id, 0, 0)) {
		_E("Failed to unset batch for client [%d], for sensor [%#llx] to client info manager",
			m_client_id, m_sensor_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!m_module->delete_interval(m_client_id, false)) {
		_E("Failed to delete interval for client [%d]", m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!m_module->delete_batch(m_client_id)) {
		_E("Failed to delete latency for client [%d]", m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_pause_policy(void *payload)
{
	cmd_set_pause_policy_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_set_pause_policy_t*)payload;

	if (!is_permission_allowed()) {
		_E("Permission denied to set interval for client [%d], for sensor [%#llx] with pause_policy [%d] to client info manager",
			m_client_id, m_sensor_id, cmd->pause_policy);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().set_pause_policy(m_client_id, m_sensor_id, cmd->pause_policy)) {
		_E("Failed to set pause_policy for client [%d], for sensor [%#llx] with pause_policy [%d] to client info manager",
			m_client_id, m_sensor_id, cmd->pause_policy);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;
out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_get_data(void *payload)
{
	const unsigned int GET_DATA_MIN_INTERVAL = 10;
	int state;
	bool adjusted = false;

	sensor_data_t *data = NULL;

	_D("CMD_GET_VALUE Handler invoked\n");

	if (!is_permission_allowed()) {
		_E("Permission denied to get data for client [%d], for sensor [%#llx]",
			m_client_id, m_sensor_id);
		state = -EACCES;
		goto out;
	}

	state = m_module->get_cache(&data);

	/* if there is no cached data, wait short time and retry to get data again */
	if (state == -ENODATA) {
		const int RETRY_CNT = 10;
		int retry = 0;

		unsigned int interval = m_module->get_interval(m_client_id, false);

		 /* 1. change interval to 10ms. */
		if (interval > GET_DATA_MIN_INTERVAL) {
			m_module->add_interval(m_client_id, GET_DATA_MIN_INTERVAL, false);
			adjusted = true;
		}

		/* 2. try to get sensor data increasing the waited time(20ms, 40ms, 80ms, 160ms, 160ms...) */
		/* 3. if data cannot be found in 10 times, stop it. */
		while ((state == -ENODATA) && (retry++ < RETRY_CNT)) {
			_I("Wait sensor[%#llx] data updated for client [%d] #%d", m_sensor_id, m_client_id, retry);
			usleep(WAIT_TIME(retry));
			state = m_module->get_cache(&data);
		}

		/* 4. revert to original interval */
		if (adjusted)
			m_module->add_interval(m_client_id, interval, false);
	}

	if (state < 0) {
		_E("Failed to get data for client [%d], for sensor [%#llx]",
			m_client_id, m_sensor_id);
	}

out:
	send_cmd_get_data_done(state < 0 ? OP_ERROR : OP_SUCCESS, data);

	return true;
}

bool command_worker::cmd_set_attribute_int(void *payload)
{
	cmd_set_attribute_int_t *cmd;
	long ret_value = OP_ERROR;

	_D("CMD_SET_COMMAND Handler invoked\n");

	cmd = (cmd_set_attribute_int_t*)payload;

	if (!is_permission_allowed()) {
		_E("Permission denied to set attribute for client [%d], for sensor [%#llx] with attribute [%d]",
			m_client_id, m_sensor_id, cmd->attribute);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = m_module->add_attribute(m_client_id, cmd->attribute, cmd->value);

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_attribute_str(void *payload)
{
	cmd_set_attribute_str_t *cmd;
	long ret_value = OP_ERROR;

	_D("CMD_SEND_SENSORHUB_DATA Handler invoked");

	cmd = (cmd_set_attribute_str_t*)payload;

	if (!is_permission_allowed()) {
		_E("Permission denied to set attribute for client [%d], for sensor [%#llx]",
			m_client_id, m_sensor_id);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = m_module->add_attribute(m_client_id, cmd->attribute, cmd->value, cmd->len);

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_flush(void *payload)
{
	long ret_value = OP_ERROR;

	_D("CMD_FLUSH Handler invoked");

	if (!is_permission_allowed()) {
		_E("Permission denied to flush sensor data for client [%d], for sensor [%#llx]",
			m_client_id, m_sensor_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!m_module->flush()) {
		_E("Failed to flush sensor_data [%d]", m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_value))
		_E("Failed to send cmd_done to a client");

	return true;
}

void command_worker::get_info(string &info)
{
	const char *client_info = NULL;
	const char *sensor_info = NULL;

	if (m_client_id != CLIENT_ID_INVALID)
		client_info = get_client_info_manager().get_client_info(m_client_id);

	if (m_module)
		sensor_info = m_module->get_name();

	info = string("Command worker for ") + (client_info ? client_info : "Unknown") + "'s "
		+ (sensor_info ? sensor_info : "Unknown");
}

int command_worker::get_permission(void)
{
	return permission_checker::get_instance().get_permission(m_socket.get_socket_fd());
}

bool command_worker::is_permission_allowed(void)
{
	if (!m_module)
		return false;

	if (m_module->get_permission() & m_permission)
		return true;

	return false;
}

client_info_manager& command_worker::get_client_info_manager(void)
{
	return client_info_manager::get_instance();
}

sensor_event_dispatcher& command_worker::get_event_dispathcher(void)
{
	return sensor_event_dispatcher::get_instance();
}

