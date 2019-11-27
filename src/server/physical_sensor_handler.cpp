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

#include "physical_sensor_handler.h"

#include <sensor_log.h>
#include <command_types.h>
#include <message.h>
#include <algorithm>

using namespace sensor;

physical_sensor_handler::physical_sensor_handler(const sensor_info &info,
		sensor_device *device, int hal_id,
		physical_sensor *sensor)
: sensor_handler(info)
, m_device(device)
, m_sensor(sensor)
, m_hal_id(hal_id)
, m_prev_interval(0)
, m_prev_latency(0)
{
}

physical_sensor_handler::~physical_sensor_handler()
{
}

const sensor_info &physical_sensor_handler::get_sensor_info(void)
{
	return m_info;
}

int physical_sensor_handler::get_hal_id(void)
{
	return m_hal_id;
}

int physical_sensor_handler::get_poll_fd(void)
{
	retv_if(!m_device, -EINVAL);

	return m_device->get_poll_fd();
}

int physical_sensor_handler::read_fd(std::vector<uint32_t> &ids)
{
	retv_if(!m_device, -EINVAL);

	int size;
	uint32_t *_ids;

	size = m_device->read_fd(&_ids);
	retv_if(size == 0, -ENODATA);

	for (int i = 0; i < size; ++i)
		ids.push_back(_ids[i]);

	return OP_SUCCESS;
}

int physical_sensor_handler::on_event(const sensor_data_t *data, int32_t len, int32_t remains)
{
	retv_if(!m_device, -EINVAL);

	if (m_sensor) {
		int ret = m_sensor->on_event(const_cast<sensor_data_t *>(data), len, remains);
		retv_if(ret <= OP_ERROR, ret);
	}

	return OP_SUCCESS;
}
int physical_sensor_handler::start(sensor_observer *ob)
{
	retv_if(!m_device, -EINVAL);

	bool ret;
	int policy = OP_DEFAULT;

	ret = add_observer(ob);
	retvm_if(!ret, OP_SUCCESS, "Listener is already added");

	if (m_sensor) {
		policy = m_sensor->start(ob);
		if (policy <= OP_ERROR) {
			remove_observer(ob);
			return policy;
		}
	}

	if (policy == OP_DEFAULT) {
		if (observer_count() > 1)
			return OP_SUCCESS; /* already started */
	}

	_I("Started[%s]", m_info.get_uri().c_str());

	return m_device->enable(m_hal_id);
}

int physical_sensor_handler::stop(sensor_observer *ob)
{
	retv_if(!m_device, -EINVAL);

	int policy = OP_DEFAULT;

	if (m_sensor) {
		policy = m_sensor->stop(ob);
		retv_if(policy <= OP_ERROR, policy);
	}

	remove_observer(ob);

	if (policy == OP_DEFAULT) {
		if (observer_count() >= 1)
			return OP_SUCCESS; /* already stopped */
	}

	_I("Stopped[%s]", m_info.get_uri().c_str());

	return m_device->disable(m_hal_id);
}

int physical_sensor_handler::get_min_interval(void)
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

int physical_sensor_handler::set_interval(sensor_observer *ob, int32_t interval)
{
	retv_if(!m_device, -EINVAL);

	bool ret = false;
	int32_t cur_interval = interval;
	int policy = OP_DEFAULT;

	if (m_sensor) {
		policy = m_sensor->set_interval(ob, cur_interval);
		retv_if(policy <= OP_ERROR, policy);
	}

	m_interval_map[ob] = cur_interval;

	if (policy == OP_DEFAULT)
		cur_interval = get_min_interval();

	retv_if(m_prev_interval == cur_interval, OP_SUCCESS);

	ret = m_device->set_interval(m_hal_id, cur_interval);

	m_prev_interval = cur_interval;

	_I("Set interval[%d] to sensor[%s]", cur_interval, m_info.get_uri().c_str());

	return (ret ? OP_SUCCESS : OP_ERROR);
}

int physical_sensor_handler::get_min_batch_latency(void)
{
	int batch_latency;
	std::vector<int> temp;

	for (auto it = m_batch_latency_map.begin(); it != m_batch_latency_map.end(); ++it)
		temp.push_back(it->second);

	if (temp.empty())
		return -1;

	batch_latency = *std::min_element(temp.begin(), temp.end());

	return batch_latency;
}

int physical_sensor_handler::set_batch_latency(sensor_observer *ob, int32_t latency)
{
	retv_if(!m_device, -EINVAL);

	bool ret = false;
	int32_t cur_latency = latency;
	int policy = OP_DEFAULT;

	if (m_sensor) {
		policy = m_sensor->set_batch_latency(ob, latency);
		retv_if(policy <= OP_ERROR, policy);
	}

	m_batch_latency_map[ob] = cur_latency;

	if (policy == OP_DEFAULT)
		cur_latency = get_min_batch_latency();

	retv_if(m_prev_latency == cur_latency, OP_SUCCESS);

	ret = m_device->set_batch_latency(m_hal_id, cur_latency);

	m_prev_latency = cur_latency;

	_I("Set batch latency[%d] to sensor[%s]", cur_latency, m_info.get_uri().c_str());

	return (ret ? OP_SUCCESS : OP_ERROR);
}

int physical_sensor_handler::delete_batch_latency(sensor_observer *ob)
{
	bool ret = false;
	int policy = OP_DEFAULT;
	int32_t latency;

	m_batch_latency_map.erase(ob);

	latency = get_min_batch_latency();
	retv_if(m_prev_latency == latency, OP_SUCCESS);

	ret = m_device->set_batch_latency(m_hal_id, latency);

	m_prev_latency = latency;

	return (ret ? OP_SUCCESS : OP_ERROR);
}

int physical_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, int32_t value)
{
	retv_if(!m_device, -EINVAL);

	bool ret = false;
	int policy = OP_DEFAULT;

	if (m_sensor) {
		policy = m_sensor->set_attribute(ob, attr, value);
		retv_if(policy <= OP_ERROR, policy);
	}

	/*
	 * TODO: default policy for attribute?
	if (policy == OP_DEFAULT) {
		if (observer_count() > 1)
			return OP_SUCCESS;
	}
	*/

	ret = m_device->set_attribute_int(m_hal_id, attr, value);

	if (ret) {
		update_attribute(attr, value);
	}
	return (ret ? OP_SUCCESS : OP_ERROR);
}

int physical_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, const char *value, int len)
{
	retv_if(!m_device, -EINVAL);

	bool ret = false;
	int policy = OP_DEFAULT;

	if (m_sensor) {
		policy = m_sensor->set_attribute(ob, attr, value, len);
		retv_if(policy <= OP_ERROR, policy);
	}

	/*
	 * TODO: default policy for attribute?
	if (policy == OP_DEFAULT) {
		if (observer_count() > 1)
			return OP_SUCCESS;
	}
	*/

	ret = m_device->set_attribute_str(m_hal_id, attr, const_cast<char *>(value), len);

	if (ret) {
		update_attribute(attr, value, len);
	}

	return (ret ? OP_SUCCESS : OP_ERROR);
}

int physical_sensor_handler::flush(sensor_observer *ob)
{
	retv_if(!m_device, -EINVAL);
	int ret = false;

	if (m_sensor) {
		ret = m_sensor->flush(ob);
		retv_if(ret <= OP_ERROR, ret);
	}

	ret = m_device->flush(m_hal_id);

	return (ret ? OP_SUCCESS : OP_ERROR);
}

int physical_sensor_handler::get_data(sensor_data_t **data, int *len)
{
	retv_if(!m_device, -EINVAL);

	int remains = m_device->get_data(m_hal_id, data, len);
	retvm_if(*len <= 0, OP_ERROR, "Failed to get sensor event");

	return remains;
}

