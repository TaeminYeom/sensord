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

#include "sensor_handler.h"

#include <message.h>
#include <sensor_log.h>
#include <sensor_utils.h>
#include <sensor_types_private.h>
#include <command_types.h>
#include <sensor_listener_proxy.h>

using namespace sensor;

sensor_handler::sensor_handler(const sensor_info &info)
: m_info(info)
, m_prev_interval(0)
, m_prev_latency(0)
, m_need_to_notify_attribute_changed(false)
{
	const char *priv = sensor::utils::get_privilege(m_info.get_uri());
	m_info.set_privilege(priv);

	sensor_type_t type = sensor::utils::get_type(m_info.get_uri());
	m_info.set_type(type);

	/* TODO: temporary walkaround for sensors that require multiple privileges */
	switch (m_info.get_type()) {
	case EXTERNAL_EXERCISE_SENSOR:
	case EXERCISE_STANDALONE_SENSOR:
		m_info.add_privilege(PRIVILEGE_LOCATION_URI);
		break;
	case GPS_CTRL_SENSOR:
		m_info.add_privilege(PRIVILEGE_PLATFORM_URI);
		break;
	default:
		break;
	}
}

bool sensor_handler::has_observer(sensor_observer *ob)
{
	for (auto it = m_observers.begin(); it != m_observers.end(); ++it) {
		if ((*it) == ob)
			return true;
	}

	return false;
}

bool sensor_handler::add_observer(sensor_observer *ob)
{
	retv_if(has_observer(ob), false);

	m_observers.push_back(ob);
	return true;
}

void sensor_handler::remove_observer(sensor_observer *ob)
{
	m_observers.remove(ob);
}

int sensor_handler::notify(const char *uri, sensor_data_t *data, int len)
{
	if (observer_count() == 0)
		return OP_ERROR;

	auto msg = ipc::message::create((char *)data, len);

	retvm_if(!msg, OP_ERROR, "Failed to allocate memory");

	for (auto it = m_observers.begin(); it != m_observers.end(); ++it)
		(*it)->update(uri, msg);

	set_cache(data, len);

	return OP_SUCCESS;
}

uint32_t sensor_handler::observer_count(void)
{
	return m_observers.size();
}

void sensor_handler::set_cache(sensor_data_t *data, int size)
{
	char* p = (char*) data;

	try {
		m_sensor_data_cache.reserve(size);
	} catch (...) {
		_E("Memory allocation failed");
		return;
	}
	m_sensor_data_cache.clear();
	m_sensor_data_cache.insert(m_sensor_data_cache.begin(), p, p + size);
}

int sensor_handler::get_cache(sensor_data_t **data, int *len)
{
	auto size = m_sensor_data_cache.size();
	retv_if(size == 0, -ENODATA);

	char* temp = (char *)malloc(size);
	retvm_if(temp == NULL, -ENOMEM, "Memory allocation failed");
	std::copy(m_sensor_data_cache.begin(), m_sensor_data_cache.end(), temp);

	*len = size;
	*data = (sensor_data_t *)temp;

	return 0;
}

bool sensor_handler::notify_attribute_changed(uint32_t id, int32_t attribute, int32_t value)
{
	if (observer_count() == 0)
		return OP_ERROR;

	cmd_listener_attr_int_t buf;
	buf.listener_id = id;
	buf.attribute = attribute;
	buf.value = value;

	auto msg = ipc::message::create();

	retvm_if(!msg, OP_ERROR, "Failed to allocate memory");

	msg->set_type(CMD_LISTENER_SET_ATTR_INT);
	msg->enclose((char *)&buf, sizeof(buf));

	sensor_listener_proxy *proxy = NULL;
	for (auto it = m_observers.begin(); it != m_observers.end(); ++it) {
		proxy = dynamic_cast<sensor_listener_proxy *>(*it);
		if (proxy && proxy->get_id() != id) {
			proxy->on_attribute_changed(msg);
		}
	}

	return OP_SUCCESS;
}

bool sensor_handler::notify_attribute_changed(uint32_t id, int32_t attribute, const char *value, int len)
{
	if (observer_count() == 0)
		return OP_ERROR;

	cmd_listener_attr_str_t *buf;
	size_t size;
	size = sizeof(cmd_listener_attr_str_t) + len;
	buf = (cmd_listener_attr_str_t *) new(std::nothrow) char[size];
	retvm_if(!buf, -ENOMEM, "Failed to allocate memory");

	auto msg = ipc::message::create();
	retvm_if(!msg, OP_ERROR, "Failed to allocate memory");

	buf->listener_id = id;
	buf->attribute = attribute;
	memcpy(buf->value, value, len);
	buf->len = len;

	msg->set_type(CMD_LISTENER_SET_ATTR_STR);
	msg->enclose((char *)buf, size);

	_I("notify attribute changed by listener[%zu]\n", id);
	sensor_listener_proxy *proxy = NULL;
	for (auto it = m_observers.begin(); it != m_observers.end(); ++it) {
		proxy = dynamic_cast<sensor_listener_proxy *>(*it);
		if (proxy) {
			proxy->on_attribute_changed(msg);
		}
	}

	delete[] buf;

	return OP_SUCCESS;
}

int sensor_handler::delete_batch_latency(sensor_observer *ob)
{
	return 0;
}

int sensor_handler::get_attribute(int32_t attr, int32_t* value)
{
	auto it = m_attributes_int.find(attr);
	retv_if(it == m_attributes_int.end(), OP_ERROR);

	*value = it->second;
	return OP_SUCCESS;
}

void sensor_handler::update_attribute(int32_t attr, int32_t value)
{
	auto it = m_attributes_int.find(attr);
	if(it != m_attributes_int.end()) {
		if (it->second != value) {
			set_need_to_notify_attribute_changed(true);
		}
	} else {
		set_need_to_notify_attribute_changed(true);
	}
	if (need_to_notify_attribute_changed()) {
		m_attributes_int[attr] = value;
		_I("[%s] attributes(int) attr[%d] value[%d] attributes size[%zu]", m_info.get_uri().c_str(), attr, value, m_attributes_int.size());
	}
}

int sensor_handler::get_attribute(int32_t attr, char **value, int *len)
{
	auto it = m_attributes_str.find(attr);
	retv_if(it == m_attributes_str.end(), OP_ERROR);

	*len = it->second.size();
	*value = new(std::nothrow) char[*len];
	std::copy(it->second.begin(), it->second.end(), *value);

	return OP_SUCCESS;
}

void sensor_handler::update_attribute(int32_t attr, const char *value, int len)
{
	auto it = m_attributes_str.find(attr);
	if(it != m_attributes_str.end()) {
		if (it->second.size() != (size_t)len) {
			set_need_to_notify_attribute_changed(true);
		} else {
			for(int i = 0 ; i < len; i++) {
				if (value[i] != it->second[i]) {
					set_need_to_notify_attribute_changed(true);
					break;
				}
			}
		}
	} else {
		set_need_to_notify_attribute_changed(true);
	}
	if (need_to_notify_attribute_changed()) {
		m_attributes_str[attr].clear();
		m_attributes_str[attr].insert(m_attributes_str[attr].begin(), value, value + len);
		_I("[%s] attributes(int) attr[%d] value[%s] attributes size[%zu]", m_info.get_uri().c_str(), attr, value, m_attributes_str.size());
	}
}

bool sensor_handler::need_to_notify_attribute_changed()
{
	return m_need_to_notify_attribute_changed;
}

void sensor_handler::set_need_to_notify_attribute_changed(bool value)
{
	m_need_to_notify_attribute_changed = value;
}

void sensor_handler::update_prev_interval(int32_t interval)
{
	if (m_prev_interval != interval) {
		m_prev_interval = interval;
		set_need_to_notify_attribute_changed(true);
		_I("Set interval[%d] to sensor[%s]", m_prev_interval, m_info.get_uri().c_str());
	}
}

void sensor_handler::update_prev_latency(int32_t latency)
{
	if (m_prev_latency != latency) {
		m_prev_latency = latency;
		set_need_to_notify_attribute_changed(true);
		_I("Set interval[%d] to sensor[%s]", m_prev_latency, m_info.get_uri().c_str());
	}
}
