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

#include <client_common.h>
#include <sensor_handle_info.h>
#include <limits>

using std::pair;

unsigned long long sensor_handle_info::m_event_id = 0;

attribute_info::attribute_info()
: m_attr(NULL)
, m_len(0)
{
}

attribute_info::~attribute_info()
{
	if (m_attr) {
		delete m_attr;
		m_attr = NULL;
	}
	m_len = 0;
}

bool attribute_info::set(const char *value, unsigned int len)
{
	if (m_attr)
		delete m_attr;

	m_attr = new(std::nothrow) char[len];
	retvm_if(!m_attr, false, "Failed to allocate memory");

	memcpy(m_attr, value, len);
	m_len = len;

	return true;
}

char *attribute_info::get(void)
{
	return m_attr;
}

unsigned int attribute_info::size(void)
{
	return m_len;
}

sensor_handle_info::sensor_handle_info()
: m_handle(0)
, m_sensor_id(UNKNOWN_SENSOR)
, m_sensor_state(SENSOR_STATE_UNKNOWN)
, m_pause_policy(SENSORD_PAUSE_ALL)
, m_bad_accuracy(false)
, m_accuracy(-1)
, m_accuracy_cb(NULL)
, m_accuracy_user_data(NULL)
, m_passive(false)
{
}

sensor_handle_info::~sensor_handle_info()
{
	clear_all_events();
}

reg_event_info* sensor_handle_info::get_reg_event_info(unsigned int event_type)
{
	auto it_event = m_reg_event_infos.find(event_type);

	if (it_event == m_reg_event_infos.end())
		return NULL;

	return &(it_event->second);
}

void sensor_handle_info::get_reg_event_types(event_type_vector &event_types)
{
	auto it_event = m_reg_event_infos.begin();

	while (it_event != m_reg_event_infos.end()) {
		event_types.push_back(it_event->first);
		++it_event;
	}
}

bool sensor_handle_info::add_reg_event_info(unsigned int event_type, unsigned int interval, unsigned int latency, void *cb, void *user_data)
{
	reg_event_info event_info;

	auto it_event = m_reg_event_infos.find(event_type);

	if (it_event != m_reg_event_infos.end()) {
		_E("Event %s[%#x] is already registered for client %s", get_event_name(event_type), event_type, get_client_name());
		return false;
	}

	event_info.m_id = renew_event_id();
	event_info.m_handle = m_handle;
	event_info.type = event_type;
	event_info.m_interval = interval;
	event_info.m_latency = latency;
	event_info.m_cb = cb;
	event_info.m_user_data = user_data;

	m_reg_event_infos.insert(pair<unsigned int, reg_event_info>(event_type, event_info));

	return true;
}

bool sensor_handle_info::delete_reg_event_info(unsigned int event_type)
{
	auto it_event = m_reg_event_infos.find(event_type);

	if (it_event == m_reg_event_infos.end()) {
		_E("Event %s[%#x] is not registered for client %s", get_event_name(event_type), event_type, get_client_name());
		return false;
	}

	m_reg_event_infos.erase(it_event);

	return true;
}

void sensor_handle_info::clear(void)
{
	sensor_attribute_str_map::iterator it_attribute;

	for (it_attribute = attributes_str.begin(); it_attribute != attributes_str.end(); ++it_attribute)
		delete it_attribute->second;

	attributes_int.clear();
	attributes_str.clear();
}

void sensor_handle_info::clear_all_events(void)
{
	m_reg_event_infos.clear();
}

unsigned long long sensor_handle_info::renew_event_id(void)
{
	return m_event_id++;
}

bool sensor_handle_info::change_reg_event_batch(unsigned int event_type, unsigned int interval, unsigned int latency)
{
	auto it_event = m_reg_event_infos.find(event_type);

	if (it_event == m_reg_event_infos.end()) {
		_E("Event %s[%#x] is not registered for client %s", get_event_name(event_type), event_type, get_client_name());
		return false;
	}

	it_event->second.m_id = renew_event_id();
	it_event->second.m_interval = interval;
	it_event->second.m_latency = latency;

	return true;
}

void sensor_handle_info::get_batch(unsigned int &interval, unsigned int &latency)
{
	if (m_reg_event_infos.empty()) {
		_D("No events are registered for client %s", get_client_name());
		interval = POLL_10HZ_MS;
		latency = 0;
		return;
	}

	unsigned int min_interval = POLL_MAX_HZ_MS;
	unsigned int min_latency = std::numeric_limits<unsigned int>::max();

	unsigned int _interval;
	unsigned int _latency;

	auto it_event = m_reg_event_infos.begin();

	while (it_event != m_reg_event_infos.end()) {
		_interval = it_event->second.m_interval;
		_latency = it_event->second.m_latency;

		min_interval = (_interval < min_interval) ? _interval : min_interval;
		min_latency = (_latency < min_latency) ? _latency : min_latency;
		++it_event;
	}

	interval = min_interval;
	latency = min_latency;
}

unsigned int sensor_handle_info::get_reg_event_count(void)
{
	return m_reg_event_infos.size();
}

bool sensor_handle_info::get_passive_mode(void)
{
	return m_passive;
}

void sensor_handle_info::set_passive_mode(bool passive)
{
	m_passive = passive;
}

bool sensor_handle_info::is_started(void)
{
	return (m_sensor_state == SENSOR_STATE_STARTED) || m_passive;
}
