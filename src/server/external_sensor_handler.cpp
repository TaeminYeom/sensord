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

#include "external_sensor_handler.h"

#include <message.h>
#include <sensor_log.h>
#include <external_sensor.h>
#include <algorithm>

using namespace sensor;

class external_sensor_notifier : public sensor_notifier {
public:
	external_sensor_notifier(external_sensor_handler *sensor);

	int notify(void);

private:
	external_sensor_handler *m_sensor;
};

external_sensor_notifier::external_sensor_notifier(external_sensor_handler *sensor)
: m_sensor(sensor)
{
}

int external_sensor_notifier::notify(void)
{
	/* TODO: Change thread-safe function(add handler to event-loop) */
	sensor_data_t *data;
	int len;

	if (m_sensor->get_data(&data, &len) < 0)
		return OP_ERROR;

	/* TODO: pointer would be better */
	sensor_info info = m_sensor->get_sensor_info();

	return m_sensor->notify(info.get_uri().c_str(), data, len);
}

external_sensor_handler::external_sensor_handler(const sensor_info &info,
		external_sensor *sensor)
: sensor_handler(info)
, m_sensor(sensor)
, m_notifier(NULL)
, m_policy(OP_DEFAULT)
{
	init();
}

external_sensor_handler::~external_sensor_handler()
{
	deinit();
}

bool external_sensor_handler::init(void)
{
	m_notifier = new(std::nothrow) external_sensor_notifier(this);
	retvm_if(!m_notifier, false, "Failed to allocate memory");

	m_sensor->set_notifier(m_notifier);
	return true;
}

void external_sensor_handler::deinit(void)
{
	delete m_notifier;
	m_notifier = NULL;
}

const sensor_info &external_sensor_handler::get_sensor_info(void)
{
	return m_info;
}

int external_sensor_handler::start(sensor_observer *ob)
{
	retv_if(!m_sensor, -EINVAL);

	int policy = m_sensor->start(ob);
	retv_if(policy <= OP_ERROR, policy);

	add_observer(ob);

	return OP_SUCCESS;
}

int external_sensor_handler::stop(sensor_observer *ob)
{
	retv_if(!m_sensor, -EINVAL);

	int policy = m_sensor->stop(ob);
	retv_if(policy <= OP_ERROR, policy);

	remove_observer(ob);

	return OP_SUCCESS;
}

int external_sensor_handler::get_min_interval(void)
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

int external_sensor_handler::set_interval(sensor_observer *ob, int32_t interval)
{
	retv_if(!m_sensor, -EINVAL);

	int _interval = interval;

	if ((m_policy == OP_DEFAULT && observer_count() == 0) || m_policy == OP_SUCCESS) {
		m_policy = m_sensor->set_interval(ob, interval);
		retv_if(m_policy <= OP_ERROR, m_policy);
	}

	m_interval_map[ob] = interval;

	if (m_policy == OP_DEFAULT && observer_count() > 0) {
		_interval = get_min_interval();
		return m_sensor->set_interval(ob, _interval);
	}

	return OP_SUCCESS;
}

int external_sensor_handler::get_min_batch_latency(void)
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

int external_sensor_handler::set_batch_latency(sensor_observer *ob, int32_t latency)
{
	retv_if(!m_sensor, -EINVAL);

	int _latency = latency;

	if ((m_policy == OP_DEFAULT && observer_count() == 0) || m_policy == OP_SUCCESS) {
		m_policy = m_sensor->set_batch_latency(ob, latency);
		retv_if(m_policy <= OP_ERROR, m_policy);
	}

	m_batch_latency_map[ob] = _latency;

	if (m_policy == OP_DEFAULT && observer_count() > 0) {
		_latency = get_min_batch_latency();
		return m_sensor->set_batch_latency(ob, latency);
	}
	return OP_SUCCESS;
}

int external_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, int32_t value)
{
	retv_if(!m_sensor, -EINVAL);

	if ((m_policy == OP_DEFAULT && observer_count() == 0) || m_policy == OP_SUCCESS) {
		m_policy = m_sensor->set_attribute(ob, attr, value);
		retv_if(m_policy <= OP_ERROR, m_policy);
	}

	if (m_policy == OP_DEFAULT) {
		/* default logic */
	}
	return OP_SUCCESS;
}

int external_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, const char *value, int len)
{
	retv_if(!m_sensor, -EINVAL);

	if ((m_policy == OP_DEFAULT && observer_count() == 0) || m_policy == OP_SUCCESS) {
		m_policy = m_sensor->set_attribute(ob, attr, value, len);
		retv_if(m_policy <= OP_ERROR, m_policy);
	}

	if (m_policy == OP_DEFAULT) {
		/* default logic */
	}
	return OP_SUCCESS;
}

int external_sensor_handler::get_data(sensor_data_t **data, int *len)
{
	return m_sensor->get_data(data, len);
}

int external_sensor_handler::flush(sensor_observer *ob)
{
	retv_if(!m_sensor, -EINVAL);

	m_sensor->flush(this);

	return OP_SUCCESS;
}
