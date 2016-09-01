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
#include <physical_sensor.h>
#include <sensor_event_queue.h>

#define UNKNOWN_NAME "UNKNOWN_SENSOR"

cmutex physical_sensor::m_mutex;

physical_sensor::physical_sensor()
: m_sensor_device(NULL)
{
}

physical_sensor::~physical_sensor()
{
	_I("physical sensor is destroyed");
}

void physical_sensor::set_sensor_info(const sensor_info_t *info)
{
	m_info = info;
}

void physical_sensor::set_sensor_device(sensor_device *device)
{
	m_sensor_device = device;
}

sensor_type_t physical_sensor::get_type(void)
{
	return static_cast<sensor_type_t>(m_info->type);
}

unsigned int physical_sensor::get_event_type(void)
{
	return m_info->event_type;
}

const char* physical_sensor::get_name(void)
{
	retv_if(!m_info, UNKNOWN_NAME);
	retv_if(!m_info->name, UNKNOWN_NAME);

	return m_info->name;
}

uint32_t physical_sensor::get_hal_id(void)
{
	return m_info->id;
}

int physical_sensor::get_poll_fd(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return OP_ERROR;

	return m_sensor_device->get_poll_fd();
}

bool physical_sensor::on_event(const sensor_data_t *data, int data_len, int remains)
{
	return true;
}

bool physical_sensor::read_fd(std::vector<uint32_t> &ids)
{
	AUTOLOCK(m_mutex);
	int size;
	uint32_t *_ids;

	if (!m_sensor_device)
		return false;

	size = m_sensor_device->read_fd(&_ids);

	for (int i = 0; i < size; ++i)
		ids.push_back(_ids[i]);

	return true;
}

int physical_sensor::get_data(sensor_data_t **data, int *length)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return OP_ERROR;

	int remains = 0;
	remains = m_sensor_device->get_data(m_info->id, data, length);

	if (*length < 0) {
		_E("Failed to get sensor event");
		return OP_ERROR;
	}

	return remains;
}

bool physical_sensor::flush(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	return m_sensor_device->flush(m_info->id);
}

bool physical_sensor::set_interval(unsigned long interval)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	_I("Polling interval is set to %dms", interval);

	return m_sensor_device->set_interval(m_info->id, interval);
}

bool physical_sensor::set_batch_latency(unsigned long latency)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	_I("Batch latency is set to %dms", latency);

	return m_sensor_device->set_batch_latency(m_info->id, latency);
}

int physical_sensor::set_attribute(int32_t attribute, int32_t value)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return OP_ERROR;

	if (!m_sensor_device->set_attribute_int(m_info->id, attribute, value))
		return OP_ERROR;

	return OP_SUCCESS;
}

int physical_sensor::set_attribute(int32_t attribute, char *value, int len)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return OP_ERROR;

	if (!m_sensor_device->set_attribute_str(m_info->id, attribute, value, len))
		return OP_ERROR;

	return OP_SUCCESS;
}

bool physical_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	return m_sensor_device->enable(m_info->id);
}

bool physical_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	return m_sensor_device->disable(m_info->id);
}

bool physical_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC); // FIXME
	info.set_name(m_info->model_name);
	info.set_vendor(m_info->vendor);
	info.set_min_range(m_info->min_range);
	info.set_max_range(m_info->max_range);
	info.set_resolution(m_info->resolution);
	info.set_min_interval(m_info->min_interval);
	info.set_fifo_count(0);
	info.set_max_batch_count(m_info->max_batch_count);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(m_info->wakeup_supported);

	return true;
}
