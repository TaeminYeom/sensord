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

#include "fusion_sensor_handler.h"

#include <message.h>
#include <sensor_log.h>
#include <algorithm>

using namespace sensor;

fusion_sensor_handler::fusion_sensor_handler(const sensor_info &info,
		fusion_sensor *sensor)
: m_info(info)
, m_sensor(sensor)
{
}

fusion_sensor_handler::~fusion_sensor_handler()
{
}

void fusion_sensor_handler::add_required_sensor(sensor_handler *sensor)
{
	m_required_sensors.push_back(sensor);
}

int fusion_sensor_handler::update(const char *uri, ipc::message *msg)
{
	retv_if(!m_sensor, -EINVAL);

	if (m_sensor->update(uri, (sensor_data_t *)msg->body(), msg->size()) < 0)
		return OP_SUCCESS;

	sensor_data_t *data;
	int len;

	if (m_sensor->get_data(&data, &len) < 0)
		return OP_ERROR;

	std::string fsensor_uri = m_info.get_type_uri();

	return notify(fsensor_uri.c_str(), data, len);
}

const sensor_info &fusion_sensor_handler::get_sensor_info(void)
{
	return m_info;
}

int fusion_sensor_handler::start(sensor_observer *ob)
{
	retv_if(!m_sensor, -EINVAL);

	int policy = OP_DEFAULT;

	policy = m_sensor->start(ob);
	retv_if(policy <= OP_ERROR, policy);

	add_observer(ob);

	if (policy == OP_DEFAULT) {
		if (observer_count() > 1)
			return OP_SUCCESS;
	}

	return start_internal();
}

int fusion_sensor_handler::stop(sensor_observer *ob)
{
	retv_if(!m_sensor, -EINVAL);

	int policy = OP_DEFAULT;

	policy = m_sensor->stop(ob);
	retv_if(policy <= OP_ERROR, policy);

	remove_observer(ob);

	if (policy == OP_DEFAULT) {
		if (observer_count() >= 1)
			return OP_SUCCESS; /* already started */
	}

	return stop_internal();
}

int fusion_sensor_handler::get_min_interval(void)
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

int fusion_sensor_handler::set_interval(sensor_observer *ob, int32_t interval)
{
	retv_if(!m_sensor, -EINVAL);

	int _interval = interval;
	int policy = OP_DEFAULT;

	policy = m_sensor->set_interval(ob, _interval);
	retv_if(policy <= OP_ERROR, policy);

	m_interval_map[ob] = interval;

	if (policy == OP_DEFAULT)
		_interval = get_min_interval();

	return set_interval_internal(_interval);
}

int fusion_sensor_handler::get_min_batch_latency(void)
{
	int batch_latency;
	std::vector<int> temp;

	for (auto it = m_batch_latency_map.begin(); it != m_batch_latency_map.end(); ++it)
		if (it->second > 0)
		    temp.push_back(it->second);

	if (temp.empty())
		return 0;

	batch_latency = *std::min_element(temp.begin(), temp.end());

	return batch_latency;
}

int fusion_sensor_handler::set_batch_latency(sensor_observer *ob, int32_t latency)
{
	retv_if(!m_sensor, -EINVAL);

	int _latency = latency;
	int policy = OP_DEFAULT;

	if (m_sensor) {
		policy = m_sensor->set_batch_latency(ob, _latency);
		retv_if(policy <= OP_ERROR, policy);
	}

	m_batch_latency_map[ob] = _latency;

	if (policy == OP_DEFAULT)
		_latency = get_min_batch_latency();

	return set_batch_latency_internal(_latency);
}

int fusion_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, int32_t value)
{
	retv_if(!m_sensor, -EINVAL);

	int policy = OP_DEFAULT;

	policy = m_sensor->set_attribute(ob, attr, value);
	retv_if(policy <= OP_ERROR, policy);

	if (policy == OP_DEFAULT) {
		/* default logic */
	}

	return set_attribute_internal(attr, value);
}

int fusion_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, const char *value, int len)
{
	retv_if(!m_sensor, -EINVAL);

	int policy = OP_DEFAULT;

	policy = m_sensor->set_attribute(ob, attr, value, len);
	retv_if(policy <= OP_ERROR, policy);

	if (policy == OP_DEFAULT) {
		/* default logic */
	}

	return set_attribute_internal(attr, value, len);
}

int fusion_sensor_handler::get_data(sensor_data_t **data, int *len)
{
	/* TODO */
	return OP_SUCCESS;
}

int fusion_sensor_handler::flush(sensor_observer *ob)
{
	retv_if(!m_sensor, -EINVAL);

	m_sensor->flush(this);

	return OP_SUCCESS;
}

int fusion_sensor_handler::start_internal(void)
{
	int size = m_required_sensors.size();
	for (int i = 0; i < size; ++i) {
		if (m_required_sensors[i]->start(this) < 0)
			return OP_ERROR;
	}

	return OP_SUCCESS;
}

int fusion_sensor_handler::stop_internal(void)
{
	int size = m_required_sensors.size();
	for (int i = 0; i < size; ++i) {
		if (m_required_sensors[i]->stop(this) < 0)
			return OP_ERROR;
	}

	return OP_SUCCESS;
}

int fusion_sensor_handler::set_interval_internal(int32_t interval)
{
	int size = m_required_sensors.size();
	for (int i = 0; i < size; ++i) {
		if (m_required_sensors[i]->set_interval(this, interval) < 0)
			return OP_ERROR;
	}

	return OP_SUCCESS;
}

int fusion_sensor_handler::set_batch_latency_internal(int32_t latency)
{
	int size = m_required_sensors.size();
	for (int i = 0; i < size; ++i) {
		if (m_required_sensors[i]->set_batch_latency(this, latency) < 0)
			return OP_ERROR;
	}

	return OP_SUCCESS;
}

int fusion_sensor_handler::set_attribute_internal(int32_t attr, int32_t value)
{
	int size = m_required_sensors.size();
	for (int i = 0; i < size; ++i) {
		if (m_required_sensors[i]->set_attribute(this, attr, value) < 0)
			return OP_ERROR;
	}

	return OP_SUCCESS;
}

int fusion_sensor_handler::set_attribute_internal(int32_t attr, const char *value, int len)
{
	int size = m_required_sensors.size();
	for (int i = 0; i < size; ++i) {
		if (m_required_sensors[i]->set_attribute(this, attr, value, len) < 0)
			return OP_ERROR;
	}

	return OP_SUCCESS;
}
