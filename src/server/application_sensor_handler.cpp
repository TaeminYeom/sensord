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

#include "application_sensor_handler.h"

#include <message.h>
#include <command_types.h>
#include <sensor_log.h>
#include <algorithm>

using namespace sensor;

application_sensor_handler::application_sensor_handler(const sensor_info &info, ipc::channel *ch)
: sensor_handler(info)
, m_ch(ch)
, m_started(false)
, m_prev_interval(0)
{
}

application_sensor_handler::~application_sensor_handler()
{
}

int application_sensor_handler::publish(sensor_data_t *data, int len)
{
	std::string uri = m_info.get_uri();
	return notify(uri.c_str(), data, len);
}

const sensor_info &application_sensor_handler::get_sensor_info(void)
{
	return m_info;
}

int application_sensor_handler::start(sensor_observer *ob)
{
	add_observer(ob);

	if (observer_count() > 1 || m_started.load())
		return OP_SUCCESS; /* already started */

	ipc::message msg;
	msg.set_type(CMD_PROVIDER_START);
	m_ch->send_sync(msg);
	m_started.store(true);

	_I("Started[%s]", m_info.get_uri().c_str());

	return OP_SUCCESS;
}

int application_sensor_handler::stop(sensor_observer *ob)
{
	remove_observer(ob);

	if (observer_count() > 0 || !m_started.load())
		return OP_SUCCESS; /* already started */

	ipc::message msg;
	msg.set_type(CMD_PROVIDER_STOP);
	m_ch->send_sync(msg);
	m_started.store(false);

	_I("Stopped[%s]", m_info.get_uri().c_str());

	return OP_SUCCESS;
}

int application_sensor_handler::get_min_interval(void)
{
	int interval;
	std::vector<int> temp;

	for (auto it = m_interval_map.begin(); it != m_interval_map.end(); ++it)
		if (it->second > 0)
		    temp.push_back(it->second);

	if (temp.empty())
		return m_info.get_min_interval();

	interval = *std::min_element(temp.begin(), temp.end());

	if (interval < m_info.get_min_interval())
		return m_info.get_min_interval();

	return interval;
}

int application_sensor_handler::set_interval(sensor_observer *ob, int32_t interval)
{
	retv_if(interval == m_prev_interval, OP_SUCCESS);

	int32_t cur_interval = interval;

	m_interval_map[ob] = cur_interval;
	cur_interval = get_min_interval();

	ipc::message msg;
	cmd_provider_attr_int_t buf;
	buf.attribute = SENSORD_ATTRIBUTE_INTERVAL;
	buf.value = cur_interval;

	msg.set_type(CMD_PROVIDER_ATTR_INT);
	msg.enclose((const char *)&buf, sizeof(cmd_provider_attr_int_t));
	m_ch->send_sync(msg);

	m_prev_interval = cur_interval;

	_I("Set interval[%d] to sensor[%s]", cur_interval, m_info.get_uri().c_str());

	return OP_SUCCESS;
}

int application_sensor_handler::set_batch_latency(sensor_observer *ob, int32_t latency)
{
	return OP_SUCCESS;
}

int application_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, int32_t value)
{
	update_attribute(attr, value);
	return OP_SUCCESS;
}

int application_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, const char *value, int len)
{
	ipc::message msg;
	cmd_provider_attr_str_t *buf;
	size_t size;

	size = sizeof(cmd_provider_attr_str_t) + len;

	buf = (cmd_provider_attr_str_t *) malloc(sizeof(char) * size);
	retvm_if(!buf, -ENOMEM, "Failed to allocate memory");

	msg.set_type(CMD_PROVIDER_ATTR_STR);
	memcpy(buf->value, value, len);
	buf->attribute = attr;
	buf->len = len;

	msg.enclose((char *)buf, size);

	m_ch->send_sync(msg);

	_I("Set attribute[%d] to sensor[%s]", attr, m_info.get_uri().c_str());
	update_attribute(attr, value, len);
	return OP_SUCCESS;
}

int application_sensor_handler::flush(sensor_observer *ob)
{
	return OP_SUCCESS;
}

int application_sensor_handler::get_data(sensor_data_t **data, int *len)
{
	return OP_SUCCESS;
}
