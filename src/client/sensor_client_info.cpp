/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include <sensor_event_listener.h>
#include <client_common.h>
#include <sensor_info_manager.h>

#include <thread>
#include <chrono>

using std::thread;
using std::pair;

sensor_client_info::sensor_client_info()
: m_client_id(CLIENT_ID_INVALID)
{
}

sensor_client_info::~sensor_client_info()
{
}

sensor_client_info& sensor_client_info::get_instance(void)
{
	static sensor_client_info inst;
	return inst;
}

int sensor_client_info::create_handle(sensor_id_t sensor)
{
	sensor_handle_info handle_info;
	int handle = 0;

	AUTOLOCK(m_handle_info_lock);

	while (m_sensor_handle_infos.count(handle) > 0)
		handle++;

	if (handle == MAX_HANDLE) {
		_E("Handles of client %s are full", get_client_name());
		return MAX_HANDLE_REACHED;
	}

	handle_info.m_sensor_id = sensor;
	handle_info.m_sensor_state = SENSOR_STATE_STOPPED;
	handle_info.m_pause_policy = SENSORD_PAUSE_ALL;
	handle_info.m_handle = handle;
	handle_info.m_accuracy = -1;
	handle_info.m_accuracy_cb = NULL;
	handle_info.m_accuracy_user_data = NULL;

	m_sensor_handle_infos.insert(pair<int, sensor_handle_info>(handle, handle_info));

	return handle;
}

bool sensor_client_info::delete_handle(int handle)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	m_sensor_handle_infos.erase(it_handle);
	return true;
}

bool sensor_client_info::is_active(void)
{
	AUTOLOCK(m_handle_info_lock);

	return !m_sensor_handle_infos.empty();
}

bool sensor_client_info::register_event(int handle, unsigned int event_type,
		unsigned int interval, unsigned int latency, void *cb, void* user_data)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	if (!it_handle->second.add_reg_event_info(event_type, interval, latency, cb, user_data))
		return false;

	return true;
}

bool sensor_client_info::unregister_event(int handle, unsigned int event_type)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	if (!it_handle->second.delete_reg_event_info(event_type))
		return false;

	return true;
}

bool sensor_client_info::register_accuracy_cb(int handle, sensor_accuracy_changed_cb_t cb, void* user_data)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_accuracy = -1;
	it_handle->second.m_accuracy_cb = cb;
	it_handle->second.m_accuracy_user_data = user_data;

	return true;
}

bool sensor_client_info::unregister_accuracy_cb(int handle)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_accuracy = -1;
	it_handle->second.m_accuracy_cb = NULL;
	it_handle->second.m_accuracy_user_data = NULL;

	return true;
}

bool sensor_client_info::set_sensor_params(int handle, int sensor_state, int pause_policy)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_sensor_state = sensor_state;
	it_handle->second.m_pause_policy = pause_policy;

	return true;
}

bool sensor_client_info::get_sensor_params(int handle, int &sensor_state, int &pause_policy)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	sensor_state = it_handle->second.m_sensor_state;
	pause_policy = it_handle->second.m_pause_policy;

	return true;
}

bool sensor_client_info::set_sensor_state(int handle, int sensor_state)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_sensor_state = sensor_state;

	return true;
}

bool sensor_client_info::get_passive_mode(int handle)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	return it_handle->second.get_passive_mode();
}

bool sensor_client_info::set_passive_mode(int handle, bool passive)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.set_passive_mode(passive);

	return true;
}

bool sensor_client_info::set_sensor_pause_policy(int handle, int pause_policy)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_pause_policy = pause_policy;

	return true;
}

bool sensor_client_info::set_event_batch(int handle, unsigned int event_type, unsigned int interval, unsigned int latency)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	if (!it_handle->second.change_reg_event_batch(event_type, interval, latency))
		return false;

	return true;
}

bool sensor_client_info::set_accuracy(int handle, int accuracy)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_accuracy = accuracy;

	return true;
}

bool sensor_client_info::set_bad_accuracy(int handle, int bad_accuracy)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_bad_accuracy = bad_accuracy;

	return true;
}

bool sensor_client_info::get_event_info(int handle, unsigned int event_type, unsigned int &interval, unsigned int &latency, void* &cb, void* &user_data)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	const reg_event_info *event_info;

	event_info = it_handle->second.get_reg_event_info(event_type);

	if (!event_info)
		return NULL;

	interval = event_info->m_interval;
	cb = event_info->m_cb;
	user_data = event_info->m_user_data;
	latency = event_info->m_latency;

	return true;
}

void sensor_client_info::get_listening_sensors(sensor_id_vector &sensors)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		sensors.push_back(it_handle->second.m_sensor_id);
		++it_handle;
	}

	sort(sensors.begin(), sensors.end());
	unique(sensors.begin(), sensors.end());
}

void sensor_client_info::get_sensor_rep(sensor_id_t sensor, sensor_rep& rep) {
	const unsigned int INVALID_BATCH_VALUE = std::numeric_limits<unsigned int>::max();

	rep.active = is_sensor_active(sensor);
	rep.pause_policy = get_active_pause_policy(sensor);
	if (!get_active_batch(sensor, rep.interval, rep.latency)) {
		rep.interval = INVALID_BATCH_VALUE;
		rep.latency = INVALID_BATCH_VALUE;
	}

	get_active_event_types(sensor, rep.event_types);
}

bool sensor_client_info::add_command_channel(sensor_id_t sensor, command_channel *cmd_channel)
{
	auto it_channel = m_command_channels.find(sensor);

	if (it_channel != m_command_channels.end()) {
		_E("%s alreay has command_channel for %s", get_client_name(), get_sensor_name(sensor));
		return false;
	}

	m_command_channels.insert(pair<sensor_id_t, command_channel *>(sensor, cmd_channel));

	return true;
}

bool sensor_client_info::get_command_channel(sensor_id_t sensor, command_channel **cmd_channel)
{
	auto it_channel = m_command_channels.find(sensor);

	if (it_channel == m_command_channels.end()) {
		_E("%s doesn't have command_channel for %s", get_client_name(), get_sensor_name(sensor));
		return false;
	}

	*cmd_channel = it_channel->second;

	return true;
}

bool sensor_client_info::close_command_channel(void)
{
	auto it_channel = m_command_channels.begin();

	if (it_channel != m_command_channels.end()) {
		delete it_channel->second;
		++it_channel;
	}

	m_command_channels.clear();

	return true;
}

bool sensor_client_info::close_command_channel(sensor_id_t sensor_id)
{
	auto it_channel = m_command_channels.find(sensor_id);

	if (it_channel == m_command_channels.end()) {
		_E("%s doesn't have command_channel for %s", get_client_name(), get_sensor_name(sensor_id));
		return false;
	}

	delete it_channel->second;

	m_command_channels.erase(it_channel);

	return true;
}

bool sensor_client_info::has_client_id(void)
{
	return (m_client_id != CLIENT_ID_INVALID);
}

int sensor_client_info::get_client_id(void)
{
	return m_client_id;
}

void sensor_client_info::set_client_id(int client_id)
{
	m_client_id = client_id;
}

bool sensor_client_info::get_active_batch(sensor_id_t sensor, unsigned int &interval, unsigned int &latency)
{
	unsigned int min_interval = POLL_MAX_HZ_MS;
	unsigned int min_latency = std::numeric_limits<unsigned int>::max();

	unsigned int _interval;
	unsigned int _latency;

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if ((it_handle->second.m_sensor_id == sensor) &&
		    it_handle->second.is_started()) {
			it_handle->second.get_batch(_interval, _latency);
			min_interval = (_interval < min_interval) ? _interval : min_interval;
			min_latency = (_latency < min_latency) ? _latency : min_latency;
		}

		++it_handle;
	}

	if (!is_sensor_active(sensor)) {
		_D("Active sensor[%#llx] is not found for client %s", sensor, get_client_name());
		return false;
	}

	interval = min_interval;
	latency = min_latency;

	return true;
}

unsigned int sensor_client_info::get_active_pause_policy(sensor_id_t sensor)
{
	int active_pause = SENSORD_PAUSE_ALL;
	int pause;

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if ((it_handle->second.m_sensor_id == sensor) &&
		    it_handle->second.is_started()) {
			pause = it_handle->second.m_pause_policy;
			active_pause = (pause < active_pause) ? pause: active_pause;
		}

		++it_handle;
	}

	if (!is_sensor_active(sensor))
		_D("Active sensor[%#llx] is not found for client %s", sensor, get_client_name());

	return active_pause;
}

bool sensor_client_info::get_sensor_id(int handle, sensor_id_t &sensor)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	sensor = it_handle->second.m_sensor_id;

	return true;
}

bool sensor_client_info::get_sensor_state(int handle, int &sensor_state)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	sensor_state = it_handle->second.m_sensor_state;

	return true;
}

void sensor_client_info::get_active_event_types(sensor_id_t sensor, event_type_vector &active_event_types)
{
	event_type_vector event_types;

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if ((it_handle->second.m_sensor_id == sensor) &&
		    it_handle->second.is_started())
			it_handle->second.get_reg_event_types(event_types);

		++it_handle;
	}

	if (event_types.empty())
		return;

	sort(event_types.begin(), event_types.end());

	unique_copy(event_types.begin(), event_types.end(), back_inserter(active_event_types));
}

void sensor_client_info::get_all_handles(handle_vector &handles)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		handles.push_back(it_handle->first);
		++it_handle;
	}
}

void sensor_client_info::get_sensor_handle_info(sensor_id_t sensor, sensor_handle_info_map &handles_info)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if (it_handle->second.m_sensor_id == sensor)
			handles_info.insert(pair<int, sensor_handle_info>(it_handle->first, it_handle->second));

		++it_handle;
	}
}

void sensor_client_info::get_all_handle_info(sensor_handle_info_map &handles_info)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		handles_info.insert(pair<int, sensor_handle_info>(it_handle->first, it_handle->second));
		++it_handle;
	}
}

bool sensor_client_info::is_sensor_registered(sensor_id_t sensor)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if (it_handle->second.m_sensor_id == sensor)
			return true;

		++it_handle;
	}

	return false;
}

bool sensor_client_info::is_sensor_active(sensor_id_t sensor)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if ((it_handle->second.m_sensor_id == sensor) &&
		    it_handle->second.is_started())
			return true;

		++it_handle;
	}

	return false;
}

bool sensor_client_info::is_event_active(int handle, unsigned int event_type, unsigned long long event_id)
{
	reg_event_info *event_info;

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end())
		return false;

	event_info = it_handle->second.get_reg_event_info(event_type);
	if (!event_info)
		return false;

	if (event_info->m_id != event_id)
		return false;

	return true;
}

void sensor_client_info::set_pause_policy(sensor_id_t sensor, int pause_policy)
{
	sensor_handle_info_map handles_info;

	get_sensor_handle_info(sensor, handles_info);

	for (auto it_handle = handles_info.begin(); it_handle != handles_info.end(); ++it_handle) {
		if (it_handle->second.m_sensor_id != sensor)
			continue;

		if (pause_policy && (it_handle->second.m_pause_policy & pause_policy)) {
			if (it_handle->second.m_sensor_state == SENSOR_STATE_STARTED) {
				set_sensor_state(it_handle->first, SENSOR_STATE_PAUSED);
				_I("%s's %s[%d] is paused", get_client_name(), get_sensor_name(sensor), it_handle->first);
			}
		} else {
			if (it_handle->second.m_sensor_state == SENSOR_STATE_PAUSED) {
				set_sensor_state(it_handle->first, SENSOR_STATE_STARTED);
				_I("%s's %s[%d] is resumed", get_client_name(), get_sensor_name(sensor), it_handle->first);
			}
		}
	}
}

bool sensor_client_info::set_attribute(int handle, int attribute, int value)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.attributes_int[attribute] = value;

	return true;
}

bool sensor_client_info::set_attribute(int handle, int attribute, const char *value, int value_len)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	auto it_attribute = it_handle->second.attributes_str.find(attribute);

	if (it_attribute != it_handle->second.attributes_str.end()) {
		it_attribute->second->set(value, value_len);
		return true;
	}

	attribute_info *info = new(std::nothrow) attribute_info();
	retvm_if(!info, false, "Failed to allocate memory");

	info->set(value, value_len);
	it_handle->second.attributes_str[attribute] = info;

	return true;
}

void sensor_client_info::clear(void)
{
	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end())
		it_handle->second.clear();

	close_command_channel();
	m_sensor_handle_infos.clear();
	m_command_channels.clear();
	set_client_id(CLIENT_ID_INVALID);
}
