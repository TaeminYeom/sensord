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

#include <command_worker.h>
#include <sensor_loader.h>
#include <sensor_info.h>
#include <sensor_types.h>
#include <thread>
#include <string>
#include <vector>
#include <utility>
#include <permission_checker.h>
#include <set>

using std::string;
using std::vector;
using std::make_pair;

command_worker::cmd_handler_t command_worker::m_cmd_handlers[];
sensor_raw_data_map command_worker::m_sensor_raw_data_map;
cpacket command_worker::m_sensor_list;

std::set<unsigned int> priority_list;

command_worker::command_worker(const csocket& socket)
: m_client_id(CLIENT_ID_INVALID)
, m_permission(SENSOR_PERMISSION_NONE)
, m_socket(socket)
, m_module(NULL)
, m_sensor_id(UNKNOWN_SENSOR)
{
	static bool init = false;

	if (!init) {
		init_cmd_handlers();
		make_sensor_raw_data_map();

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
	m_cmd_handlers[CMD_SET_OPTION]			= &command_worker::cmd_set_option;
	m_cmd_handlers[CMD_SET_WAKEUP]			= &command_worker::cmd_set_wakeup;
	m_cmd_handlers[CMD_SET_BATCH]			= &command_worker::cmd_set_batch;
	m_cmd_handlers[CMD_UNSET_BATCH]			= &command_worker::cmd_unset_batch;
	m_cmd_handlers[CMD_SET_COMMAND]			= &command_worker::cmd_set_command;
	m_cmd_handlers[CMD_GET_DATA]			= &command_worker::cmd_get_data;
	m_cmd_handlers[CMD_SEND_SENSORHUB_DATA]	= &command_worker::cmd_send_sensorhub_data;
}

void command_worker::get_sensor_list(int permissions, cpacket &sensor_list)
{
	const int PERMISSION_COUNT = sizeof(permissions) * 8;
	vector<raw_data_t *> sensor_raw_vec;
	size_t total_raw_data_size = 0;

	for (int i = 0; i < PERMISSION_COUNT; ++i) {
		int perm = (permissions & (1 << i));

		if (perm) {
			auto range = m_sensor_raw_data_map.equal_range(perm);

			sensor_raw_data_map::iterator it_raw_data;

			for (it_raw_data = range.first; it_raw_data != range.second; ++it_raw_data) {
				total_raw_data_size += it_raw_data->second.size();
				sensor_raw_vec.push_back(&(it_raw_data->second));
			}
		}
	}

	int sensor_cnt;

	sensor_cnt = sensor_raw_vec.size();

	sensor_list.set_payload_size(sizeof(cmd_get_sensor_list_done_t) + (sizeof(size_t) * sensor_cnt) + total_raw_data_size);
	sensor_list.set_cmd(CMD_GET_SENSOR_LIST);

	cmd_get_sensor_list_done_t *cmd_get_sensor_list_done;

	cmd_get_sensor_list_done = (cmd_get_sensor_list_done_t*)sensor_list.data();
	cmd_get_sensor_list_done->sensor_cnt = sensor_cnt;
	size_t* size_field = (size_t *) cmd_get_sensor_list_done->data;


	for (int i = 0; i < sensor_cnt; ++i)
		size_field[i] = sensor_raw_vec[i]->size();

	char* raw_data_field = cmd_get_sensor_list_done->data + (sizeof(size_t) * sensor_cnt);

	int idx = 0;
	for (int i = 0; i < sensor_cnt; ++i) {
		copy(sensor_raw_vec[i]->begin(), sensor_raw_vec[i]->end(), raw_data_field + idx);
		idx += sensor_raw_vec[i]->size();
	}

}

void command_worker::make_sensor_raw_data_map(void)
{
	vector<sensor_base *> sensors;
	vector<sensor_type_t> types;
	std::vector<sensor_type_t>::iterator it_type;
	std::vector<sensor_base *>::iterator it_sensor;
	sensor_info info;
	int permission;

	types = sensor_loader::get_instance().get_sensor_types();

	it_type = types.begin();
	while (it_type != types.end()) {
		sensor_type_t type;
		type = *it_type;

		sensors = sensor_loader::get_instance().get_sensors(type);
		it_sensor = sensors.begin();

		while (it_sensor != sensors.end()) {
			(*it_sensor)->get_sensor_info(info);
			permission = (*it_sensor)->get_permission();

			sensor_raw_data_map::iterator it_sensor_raw_data;
			it_sensor_raw_data = m_sensor_raw_data_map.insert(std::make_pair(permission, raw_data_t()));

			info.get_raw_data(it_sensor_raw_data->second);
			info.clear();
			++it_sensor;
		}
		++it_type;
	}
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
		DBG("%s failed to receive header", info.c_str());
		return false;
	}

	if (header.size > 0) {

		payload = new(std::nothrow) char[header.size];
		retvm_if(!payload, false, "Failed to allocate memory");

		if (inst->m_socket.recv(payload, header.size) <= 0) {
			string info;
			inst->get_info(info);
			DBG("%s failed to receive data of packet", info.c_str());
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
	INFO("%s is stopped", info.c_str());

	if ((inst->m_module) && (inst->m_client_id != CLIENT_ID_INVALID)) {

		if (get_client_info_manager().is_started(inst->m_client_id, inst->m_sensor_id)) {
			WARN("Does not receive cmd_stop before connection broken for [%s]!!", inst->m_module->get_name());
			inst->m_module->delete_interval(inst->m_client_id, false);
			inst->m_module->stop();
		}

		if (inst->m_sensor_id) {
			if (get_client_info_manager().has_sensor_record(inst->m_client_id, inst->m_sensor_id)) {
				INFO("Removing sensor[0x%llx] record for client_id[%d]", inst->m_sensor_id, inst->m_client_id);
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
		ERR("Unknown command: %d", cmd);
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
		ERR("Failed to send a cmd_done to client_id [%d] with value [%ld]", m_client_id, value);
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
		ERR("Failed to send a cmd_get_id_done with client_id [%d]", client_id);
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

	memcpy(&cmd_get_data_done->base_data , data, sizeof(sensor_data_t));

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		ERR("Failed to send a cmd_get_data_done");
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}


bool command_worker::send_cmd_get_sensor_list_done(void)
{
	cpacket sensor_list;

	int permission = get_permission();

	INFO("permission = 0x%x", permission);

	get_sensor_list(permission, sensor_list);

	if (m_socket.send(sensor_list.packet(), sensor_list.size()) <= 0) {
		ERR("Failed to send a cmd_get_sensor_list_done");
		return false;
	}

	return true;
}

bool command_worker::cmd_get_id(void *payload)
{
	cmd_get_id_t *cmd;
	int client_id;
	struct ucred cr;
	socklen_t opt_len = sizeof(cr);

	DBG("CMD_GET_ID Handler invoked\n");
	cmd = (cmd_get_id_t*)payload;

	if (getsockopt(m_socket.get_socket_fd(), SOL_SOCKET, SO_PEERCRED, &cr, &opt_len)) {
		ERR("Failed to get socket option with SO_PEERCRED");
		return false;
	}

	client_id = get_client_info_manager().create_client_record();

	get_client_info_manager().set_client_info(client_id, cr.pid, cmd->name);

	m_permission = get_permission();
	get_client_info_manager().set_permission(client_id, m_permission);

	INFO("New client id [%d] created", client_id);

	if (!send_cmd_get_id_done(client_id))
		ERR("Failed to send cmd_done to a client");

	return true;
}


bool command_worker::cmd_get_sensor_list(void *payload)
{
	DBG("CMD_GET_SENSOR_LIST Handler invoked\n");

	if (!send_cmd_get_sensor_list_done())
		ERR("Failed to send cmd_get_sensor_list_done to a client");

	return true;
}

bool command_worker::cmd_hello(void *payload)
{
	cmd_hello_t *cmd;
	long ret_value = OP_ERROR;

	DBG("CMD_HELLO Handler invoked\n");
	cmd = (cmd_hello_t*)payload;

	m_sensor_id = cmd->sensor;
	m_client_id = cmd->client_id;

	if (m_permission == SENSOR_PERMISSION_NONE)
		get_client_info_manager().get_permission(m_client_id, m_permission);

	m_module = (sensor_base *)sensor_loader::get_instance().get_sensor(cmd->sensor);

	if (!m_module) {
		ERR("Sensor type[%d] is not supported", cmd->sensor);
		if (!get_client_info_manager().has_sensor_record(m_client_id))
			get_client_info_manager().remove_client_record(m_client_id);

		ret_value = OP_ERROR;
		goto out;
	}

	if (!is_permission_allowed()) {
		ERR("Permission denied to connect sensor[0x%llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	DBG("Hello sensor [0x%llx], client id [%d]", m_sensor_id, m_client_id);
	get_client_info_manager().create_sensor_record(m_client_id, m_sensor_id);
	INFO("New sensor record created for sensor [0x%llx], sensor name [%s] on client id [%d]\n", m_sensor_id, m_module->get_name(), m_client_id);
	ret_value = OP_SUCCESS;
out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_byebye(void *payload)
{
	long ret_value = OP_ERROR;

	if (!is_permission_allowed()) {
		ERR("Permission denied to stop sensor[0x%llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	DBG("CMD_BYEBYE for client [%d], sensor [0x%llx]", m_client_id, m_sensor_id);

	if (!get_client_info_manager().remove_sensor_record(m_client_id, m_sensor_id)) {
		ERR("Error removing sensor_record for client [%d]", m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	m_client_id = CLIENT_ID_INVALID;
	ret_value = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	if (ret_value == OP_SUCCESS)
		return false;

	return true;
}

bool command_worker::cmd_start(void *payload)
{
	long ret_value = OP_ERROR;

	if (!is_permission_allowed()) {
		ERR("Permission denied to start sensor[0x%llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	DBG("START Sensor [0x%llx], called from client [%d]", m_sensor_id, m_client_id);

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
		ERR("Failed to start sensor [0x%llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
	}

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_stop(void *payload)
{
	long ret_value = OP_ERROR;

	if (!is_permission_allowed()) {
		ERR("Permission denied to stop sensor[0x%llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	DBG("STOP Sensor [0x%llx], called from client [%d]", m_sensor_id, m_client_id);

	if (m_module->stop()) {
		get_client_info_manager().set_start(m_client_id, m_sensor_id, false);
		ret_value = OP_SUCCESS;
	} else {
		ERR("Failed to stop sensor [0x%llx] for client [%d]", m_sensor_id, m_client_id);
		ret_value = OP_ERROR;
	}

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_register_event(void *payload)
{
	cmd_reg_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_reg_t*)payload;

	if (!is_permission_allowed()) {
		ERR("Permission denied to register event [0x%x] for client [%d] to client info manager",
			cmd->event_type, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().register_event(m_client_id, m_sensor_id, cmd->event_type)) {
		INFO("Failed to register event [0x%x] for client [%d] to client info manager",
			cmd->event_type, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	insert_priority_list(cmd->event_type);

	ret_value = OP_SUCCESS;
	DBG("Registering Event [0x%x] is done for client [%d]", cmd->event_type, m_client_id);

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_unregister_event(void *payload)
{
	cmd_unreg_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_unreg_t*)payload;

	if (!is_permission_allowed()) {
		ERR("Permission denied to unregister event [0x%x] for client [%d] to client info manager",
			cmd->event_type, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().unregister_event(m_client_id, m_sensor_id, cmd->event_type)) {
		ERR("Failed to unregister event [0x%x] for client [%d] from client info manager",
			cmd->event_type, m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;
	DBG("Unregistering Event [0x%x] is done for client [%d]",
		cmd->event_type, m_client_id);

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_batch(void *payload)
{
	cmd_set_batch_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_set_batch_t*)payload;

	if (!is_permission_allowed()) {
		ERR("Permission denied to set batch for client [%d], for sensor [0x%llx] with batch [%d, %d] to client info manager",
			m_client_id, m_sensor_id, cmd->interval, cmd->latency);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().set_batch(m_client_id, m_sensor_id, cmd->interval, cmd->latency)) {
		ERR("Failed to set batch for client [%d], for sensor [0x%llx] with batch [%d, %d] to client info manager",
			m_client_id, m_sensor_id, cmd->interval, cmd->latency);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!m_module->add_interval(m_client_id, cmd->interval, false)) {
		ERR("Failed to set interval for client [%d], for sensor [0x%llx] with interval [%d]",
			m_client_id, m_sensor_id, cmd->interval);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!m_module->add_batch(m_client_id, cmd->latency)) {
		ERR("Failed to set latency for client [%d], for sensor [0x%llx] with latency [%d]",
			m_client_id, m_sensor_id, cmd->latency);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_unset_batch(void *payload)
{
	long ret_value = OP_ERROR;

	if (!is_permission_allowed()) {
		ERR("Permission denied to unset batch for client [%d], for sensor [0x%llx] to client info manager",
			m_client_id, m_sensor_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().set_batch(m_client_id, m_sensor_id, 0, 0)) {
		ERR("Failed to unset batch for client [%d], for sensor [0x%llx] to client info manager",
			m_client_id, m_sensor_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!m_module->delete_interval(m_client_id, false)) {
		ERR("Failed to delete interval for client [%d]", m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!m_module->delete_batch(m_client_id)) {
		ERR("Failed to delete latency for client [%d]", m_client_id);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_option(void *payload)
{
	cmd_set_option_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_set_option_t*)payload;

	if (!is_permission_allowed()) {
		ERR("Permission denied to set interval for client [%d], for sensor [0x%llx] with option [%d] to client info manager",
			m_client_id, m_sensor_id, cmd->option);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().set_option(m_client_id, m_sensor_id, cmd->option)) {
		ERR("Failed to set option for client [%d], for sensor [0x%llx] with option [%d] to client info manager",
			m_client_id, m_sensor_id, cmd->option);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = OP_SUCCESS;
out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_wakeup(void *payload)
{
	cmd_set_wakeup_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_set_wakeup_t*)payload;

	if (!is_permission_allowed()) {
		ERR("Permission denied to set wakeup for client [%d], for sensor [0x%llx] with wakeup [%d] to client info manager",
			m_client_id, m_sensor_id, cmd->wakeup);
		ret_value = OP_ERROR;
		goto out;
	}

	if (!get_client_info_manager().set_wakeup(m_client_id, m_sensor_id, cmd->wakeup)) {
		ERR("Failed to set wakeup for client [%d], for sensor [0x%llx] with wakeup [%d] to client info manager",
			m_client_id, m_sensor_id, cmd->wakeup);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = m_module->add_wakeup(m_client_id, cmd->wakeup);

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_command(void *payload)
{
	cmd_set_command_t *cmd;
	long ret_value = OP_ERROR;

	DBG("CMD_SET_COMMAND  Handler invoked\n");

	cmd = (cmd_set_command_t*)payload;

	if (!is_permission_allowed()) {
		ERR("Permission denied to set command for client [%d], for sensor [0x%llx] with cmd [%d]",
			m_client_id, m_sensor_id, cmd->cmd);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = m_module->set_attribute(cmd->cmd, cmd->value);

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_get_data(void *payload)
{
	const unsigned int GET_DATA_MIN_INTERVAL = 10;
	int state = OP_ERROR;
	bool adjusted = false;
	int length;

	sensor_data_t *data;

	DBG("CMD_GET_VALUE Handler invoked\n");

	if (!is_permission_allowed()) {
		ERR("Permission denied to get data for client [%d], for sensor [0x%llx]",
			m_client_id, m_sensor_id);
		state = OP_ERROR;
		goto out;
	}

	state = m_module->get_data(&data, &length);

	// In case of not getting sensor data, wait short time and retry again
	// 1. changing interval to be less than 10ms
	// 2. In case of first time, wait for INIT_WAIT_TIME
	// 3. at another time, wait for WAIT_TIME
	// 4. retrying to get data
	// 5. repeat 2 ~ 4 operations RETRY_CNT times
	// 6. reverting back to original interval
	if ((state >= 0) && !data->timestamp) {
		const int RETRY_CNT	= 5;
		const unsigned long long INIT_WAIT_TIME = 20000; //20ms
		const unsigned long WAIT_TIME = 100000;	//100ms
		int retry = 0;

		unsigned int interval = m_module->get_interval(m_client_id, false);

		if (interval > GET_DATA_MIN_INTERVAL) {
			m_module->add_interval(m_client_id, GET_DATA_MIN_INTERVAL, false);
			adjusted = true;
		}

		while ((state >= 0) && !data->timestamp && (retry++ < RETRY_CNT)) {
			INFO("Wait sensor[0x%llx] data updated for client [%d] #%d", m_sensor_id, m_client_id, retry);
			usleep((retry == 1) ? INIT_WAIT_TIME : WAIT_TIME);
			state = m_module->get_data(&data, &length);
		}

		if (adjusted)
			m_module->add_interval(m_client_id, interval, false);
	}

	if (!data->timestamp)
		state = OP_ERROR;

	if (state <= 0) {
		ERR("Failed to get data for client [%d], for sensor [0x%llx]",
			m_client_id, m_sensor_id);
	}

out:
	send_cmd_get_data_done(state, data);

	return true;
}

bool command_worker::cmd_send_sensorhub_data(void *payload)
{
	cmd_send_sensorhub_data_t *cmd;
	long ret_value = OP_ERROR;

	DBG("CMD_SEND_SENSORHUB_DATA Handler invoked");

	cmd = (cmd_send_sensorhub_data_t*)payload;

	if (!is_permission_allowed()) {
		ERR("Permission denied to send sensorhub_data for client [%d], for sensor [0x%llx]",
			m_client_id, m_sensor_id);
		ret_value = OP_ERROR;
		goto out;
	}

	ret_value = m_module->set_attribute(0, cmd->data, cmd->data_len);

out:
	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

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

void insert_priority_list(unsigned int event_type)
{
	if (event_type == ORIENTATION_RAW_DATA_EVENT ||
			event_type == LINEAR_ACCEL_RAW_DATA_EVENT ||
			event_type == GRAVITY_RAW_DATA_EVENT ||
			event_type == ROTATION_VECTOR_RAW_DATA_EVENT) {
		priority_list.insert(ACCELEROMETER_RAW_DATA_EVENT);
		priority_list.insert(GYROSCOPE_RAW_DATA_EVENT);
		priority_list.insert(GEOMAGNETIC_RAW_DATA_EVENT);
	}

	if (event_type == GEOMAGNETIC_RV_RAW_DATA_EVENT) {
		priority_list.insert(ACCELEROMETER_RAW_DATA_EVENT);
		priority_list.insert(GEOMAGNETIC_RAW_DATA_EVENT);
	}

	if (event_type == GAMING_RV_RAW_DATA_EVENT) {
		priority_list.insert(ACCELEROMETER_RAW_DATA_EVENT);
		priority_list.insert(GYROSCOPE_RAW_DATA_EVENT);
	}
}
