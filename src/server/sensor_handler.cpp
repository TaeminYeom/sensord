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

using namespace sensor;

sensor_handler::sensor_handler(const sensor_info &info)
: m_info(info)
, m_last_data(NULL)
, m_last_data_size(0)
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

	ipc::message *msg;

	msg = new(std::nothrow) ipc::message((char *)data, len);
	retvm_if(!msg, OP_ERROR, "Failed to allocate memory");

	for (auto it = m_observers.begin(); it != m_observers.end(); ++it)
		(*it)->update(uri, msg);

	set_cache(data, len);

	if (msg->ref_count() == 0)
		msg->unref();

	return OP_SUCCESS;
}

uint32_t sensor_handler::observer_count(void)
{
	return m_observers.size();
}

void sensor_handler::set_cache(sensor_data_t *data, int size)
{
	if (m_last_data_size != size) {
		m_last_data_size = size;
		if (m_last_data) {
			free(m_last_data);
		}
		m_last_data = (sensor_data_t*)malloc(m_last_data_size);
		retm_if(m_last_data == NULL, "Memory allocation failed");
	}

	m_last_data_size = size;
	memcpy(m_last_data, data, size);
}

int sensor_handler::get_cache(sensor_data_t **data, int *len)
{
	retv_if(m_last_data == NULL, -ENODATA);

	*data = (sensor_data_t *)malloc(m_last_data_size);
	retvm_if(*data == NULL, -ENOMEM, "Memory allocation failed");

	memcpy(*data, m_last_data, m_last_data_size);
	*len = m_last_data_size;

	return 0;
}

bool sensor_handler::notify_attribute_changed(int attribute, int value)
{
	if (observer_count() == 0)
		return OP_ERROR;

	cmd_listener_attr_int_t buf;
	buf.attribute = attribute;
	buf.value = value;

	ipc::message *msg;
	msg = new(std::nothrow) ipc::message();
	retvm_if(!msg, OP_ERROR, "Failed to allocate memory");

	msg->set_type(CMD_LISTENER_ATTR_INT);
	msg->enclose((char *)&buf, sizeof(buf));

	for (auto it = m_observers.begin(); it != m_observers.end(); ++it)
		(*it)->on_attribute_changed(msg);

	if (msg->ref_count() == 0)
		msg->unref();

	return OP_SUCCESS;
}

bool sensor_handler::notify_attribute_changed(int attribute, const char *value, int len)
{
	if (observer_count() == 0)
		return OP_ERROR;

	cmd_listener_attr_str_t *buf;
	size_t size;
	size = sizeof(cmd_listener_attr_str_t) + len;
	buf = (cmd_listener_attr_str_t *) new(std::nothrow) char[size];
	retvm_if(!buf, -ENOMEM, "Failed to allocate memory");

	ipc::message *msg;
	msg = new(std::nothrow) ipc::message();
	retvm_if(!msg, OP_ERROR, "Failed to allocate memory");

	buf->attribute = attribute;
	memcpy(buf->value, value, len);
	buf->len = len;

	msg->set_type(CMD_LISTENER_ATTR_STR);
	msg->enclose((char *)buf, size);

	for (auto it = m_observers.begin(); it != m_observers.end(); ++it)
		(*it)->on_attribute_changed(msg);

	if (msg->ref_count() == 0)
		msg->unref();

	delete[] buf;

	return OP_SUCCESS;
}

int sensor_handler::delete_batch_latency(sensor_observer *ob)
{
	return 0;
}
