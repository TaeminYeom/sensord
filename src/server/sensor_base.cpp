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

#include <stdint.h>
#include <limits.h>
#include <sensor_hal_types.h>
#include <sensor_event_queue.h>
#include <sensor_base.h>
#include <sensor_common.h>

#include <algorithm>
#include <utility>
#include <functional>

using std::make_pair;
using std::vector;

sensor_base::sensor_base()
: m_last_data(NULL)
, m_id(SENSOR_ID_INVALID)
, m_permission(SENSOR_PERMISSION_STANDARD)
, m_started(false)
, m_client(0)
{
}

sensor_base::~sensor_base()
{
}

void sensor_base::set_id(sensor_id_t id)
{
	m_id = id;
}

sensor_id_t sensor_base::get_id(void)
{
	if (m_id == SENSOR_ID_INVALID)
		return UNKNOWN_SENSOR;

	return m_id;
}

sensor_type_t sensor_base::get_type(void)
{
	return UNKNOWN_SENSOR;
}

unsigned int sensor_base::get_event_type(void)
{
	return -1;
}

const char* sensor_base::get_name(void)
{
	return NULL;
}

bool sensor_base::get_sensor_info(sensor_info &info)
{
	return false;
}

bool sensor_base::is_virtual(void)
{
	return false;
}

int sensor_base::get_data(sensor_data_t **data, int *length)
{
	return OP_ERROR;
}

bool sensor_base::flush(void)
{
	return true;
}

int sensor_base::add_attribute(int client_id, int32_t attribute, int32_t value)
{
	return set_attribute(attribute, value);
}

int sensor_base::add_attribute(int client_id, int32_t attribute, char *value, int value_size)
{
	return set_attribute(attribute, value, value_size);
}

bool sensor_base::delete_attribute(int client_id)
{
	return true;
}

int sensor_base::set_attribute(int32_t attribute, int32_t value)
{
	return OP_SUCCESS;
}

int sensor_base::set_attribute(int32_t attribute, char *value, int value_size)
{
	return OP_SUCCESS;
}

bool sensor_base::start(void)
{
	AUTOLOCK(m_client_mutex);

	if (!pre_start())
		return false;

	++m_client;

	if (m_client == 1) {
		if (!on_start()) {
			_E("[%s] sensor failed to start", get_name());
			return false;
		}

		m_started = true;
	}

	_I("[%s] sensor started, #client = %d", get_name(), m_client);

	return true;
}

bool sensor_base::stop(void)
{
	AUTOLOCK(m_client_mutex);

	--m_client;

	if (m_client == 0) {
		if (!on_stop()) {
			_E("[%s] sensor faild to stop", get_name());
			return false;
		}

		m_started = false;

		free(m_last_data);
		m_last_data = NULL;
	}

	_I("[%s] sensor stopped, #client = %d", get_name(), m_client);

	return true;
}

bool sensor_base::is_started(void)
{
	AUTOLOCK(m_client_mutex);

	return m_started;
}

bool sensor_base::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	unsigned int prev_min, cur_min;

	AUTOLOCK(m_sensor_info_list_mutex);

	prev_min = m_sensor_info_list.get_min_interval();

	if (!m_sensor_info_list.add_interval(client_id, interval, is_processor))
		return false;

	cur_min = m_sensor_info_list.get_min_interval();

	if (cur_min != prev_min) {
		_I("Min interval for sensor[%#llx] is changed from %dms to %dms"
			" by%sclient[%d] adding interval",
			get_id(), prev_min, cur_min,
			is_processor ? " processor " : " ", client_id);

		set_interval(cur_min);
	}

	return true;
}

bool sensor_base::delete_interval(int client_id, bool is_processor)
{
	unsigned int prev_min, cur_min;
	AUTOLOCK(m_sensor_info_list_mutex);

	prev_min = m_sensor_info_list.get_min_interval();

	if (!m_sensor_info_list.delete_interval(client_id, is_processor))
		return false;

	cur_min = m_sensor_info_list.get_min_interval();

	if (!cur_min) {
		_I("No interval for sensor[%#llx] by%sclient[%d] deleting interval, "
			 "so set to default %dms",
			 get_id(), is_processor ? " processor " : " ",
			 client_id, POLL_1HZ_MS);

		set_interval(POLL_1HZ_MS);
	} else if (cur_min != prev_min) {
		_I("Min interval for sensor[%#llx] is changed from %dms to %dms"
			" by%sclient[%d] deleting interval",
			get_id(), prev_min, cur_min,
			is_processor ? " processor " : " ", client_id);

		set_interval(cur_min);
	}

	return true;
}

unsigned int sensor_base::get_interval(int client_id, bool is_processor)
{
	AUTOLOCK(m_sensor_info_list_mutex);

	return m_sensor_info_list.get_interval(client_id, is_processor);
}

bool sensor_base::add_batch(int client_id, unsigned int latency)
{
	unsigned int prev_max, cur_max;

	AUTOLOCK(m_sensor_info_list_mutex);

	prev_max = m_sensor_info_list.get_max_batch();

	if (!m_sensor_info_list.add_batch(client_id, latency))
		return false;

	cur_max = m_sensor_info_list.get_max_batch();

	if (cur_max != prev_max) {
		_I("Max latency for sensor[%#llx] is changed from %dms to %dms by client[%d] adding latency",
			get_id(), prev_max, cur_max, client_id);
		set_batch_latency(cur_max);
	}

	return true;
}

bool sensor_base::delete_batch(int client_id)
{
	unsigned int prev_max, cur_max;
	AUTOLOCK(m_sensor_info_list_mutex);

	prev_max = m_sensor_info_list.get_max_batch();

	if (!m_sensor_info_list.delete_batch(client_id))
		return false;

	cur_max = m_sensor_info_list.get_max_batch();

	if (!cur_max) {
		_I("No latency for sensor[%#llx] by client[%d] deleting latency, so set to default count",
			 get_id(), client_id);

		set_batch_latency(UINT_MAX);
	} else if (cur_max != prev_max) {
		_I("Max latency for sensor[%#llx] is changed from %dms to %dms by client[%d] deleting latency",
			get_id(), prev_max, cur_max, client_id);

		set_batch_latency(cur_max);
	}

	return true;
}

unsigned int sensor_base::get_batch(int client_id)
{
	AUTOLOCK(m_sensor_info_list_mutex);

	return m_sensor_info_list.get_batch(client_id);
}

int sensor_base::get_permission(void)
{
	return m_permission;
}

void sensor_base::set_permission(int permission)
{
	m_permission = permission;
}

bool sensor_base::push(sensor_event_t *event)
{
	if (!event || !(event->data))
		return false;

	set_cache(event->data);

	AUTOLOCK(m_client_mutex);

	if (m_client <= 0)
		return false;

	sensor_event_queue::get_instance().push(event);
	return true;
}

void sensor_base::set_cache(sensor_data_t *data)
{
	AUTOLOCK(m_data_cache_mutex);

	/* Caching the last known data for sync-read support */
	if (m_last_data == NULL) {
		m_last_data = (sensor_data_t*)malloc(sizeof(sensor_data_t));
		retm_if(m_last_data == NULL, "Memory allocation failed");
	}

	memcpy(m_last_data, data, sizeof(sensor_data_t));
}

int sensor_base::get_cache(sensor_data_t **data)
{
	retv_if(m_last_data == NULL, -ENODATA);

	*data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	retvm_if(*data == NULL, -ENOMEM, "Memory allocation failed");

	AUTOLOCK(m_data_cache_mutex);

	memcpy(*data, m_last_data, sizeof(sensor_data_t));
	return 0;
}

bool sensor_base::set_interval(unsigned long interval)
{
	return true;
}

bool sensor_base::set_batch_latency(unsigned long latency)
{
	return true;
}

bool sensor_base::pre_start(void)
{
	return true;
}

bool sensor_base::on_start(void)
{
	return true;
}

bool sensor_base::on_stop(void)
{
	return true;
}

unsigned long long sensor_base::get_timestamp(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000;
}

unsigned long long sensor_base::get_timestamp(timeval *t)
{
	if (!t) {
		_E("t is NULL");
		return 0;
	}

	return ((unsigned long long)(t->tv_sec)*1000000LL +t->tv_usec);
}
