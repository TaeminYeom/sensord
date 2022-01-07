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

#include "sensor_info.h"

#include <sensor_types.h>
#include <sensor_types_private.h>
#include <sensor_log.h>
#include <cfloat>
#include <algorithm>
#include <string>
#include <cmath>

#include "sensor_utils.h"

#define MIN_RANGE -FLT_MAX
#define MAX_RANGE FLT_MAX

using namespace sensor;

sensor_info::sensor_info()
: m_type(UNKNOWN_SENSOR)
, m_uri(SENSOR_UNKNOWN_NAME)
, m_model(SENSOR_UNKNOWN_NAME)
, m_vendor(SENSOR_UNKNOWN_NAME)
, m_min_range(0)
, m_max_range(0)
, m_resolution(0)
, m_min_interval(0)
, m_max_interval(0)
, m_max_batch_count(0)
, m_wakeup_supported(false)
, m_privilege("")
{
}

sensor_info::sensor_info(const sensor_info &info)
: m_type(info.m_type)
, m_uri(info.m_uri)
, m_model(info.m_model)
, m_vendor(info.m_vendor)
, m_min_range(info.m_min_range)
, m_max_range(info.m_max_range)
, m_resolution(info.m_resolution)
, m_min_interval(info.m_min_interval)
, m_max_interval(info.m_max_interval)
, m_max_batch_count(info.m_max_batch_count)
, m_wakeup_supported(info.m_wakeup_supported)
, m_privilege(info.m_privilege)
{
}

sensor_info::sensor_info(const sensor_info_t &info)
{
	/* TODO: HAL should change name from single name to URI */
	const char *type = sensor::utils::get_uri((sensor_type_t)info.type);
	std::string uri(type);
	uri.append("/").append(info.name);

	set_type((sensor_type_t)info.type);
	set_uri(uri.c_str());
	set_model(info.model_name);
	set_vendor(info.vendor);
	set_min_range(info.min_range);
	set_max_range(info.max_range);
	set_resolution(info.resolution);
	set_min_interval(info.min_interval);
	set_max_interval(info.max_interval);
	set_max_batch_count(info.max_batch_count);
	set_wakeup_supported(info.wakeup_supported);
	/* TODO: sensor_info_t should have privilege string */
	set_privilege("");
}

sensor_info::sensor_info(const sensor_info2_t &info)
{
	std::string uri(info.uri);
	std::size_t found = uri.find_last_of("/\\");

	set_type(info.type);
	set_uri(uri.c_str());
	set_model(uri.substr(found + 1, uri.length()).c_str());
	set_vendor(info.vendor);
	set_min_range(info.min_range);
	set_max_range(info.max_range);
	set_resolution(info.resolution);
	set_min_interval(info.min_interval);
	set_max_interval(info.max_interval);
	set_max_batch_count(info.max_batch_count);
	set_wakeup_supported(info.wakeup_supported);
	set_privilege(info.privilege);
}

sensor_type_t sensor_info::get_type(void)
{
	return m_type;
}

std::string &sensor_info::get_uri(void)
{
	return m_uri;
}

std::string &sensor_info::get_model(void)
{
	return m_model;
}

std::string &sensor_info::get_vendor(void)
{
	return m_vendor;
}

float sensor_info::get_min_range(void)
{
	return m_min_range;
}

float sensor_info::get_max_range(void)
{
	return m_max_range;
}

float sensor_info::get_resolution(void)
{
	return m_resolution;
}

int sensor_info::get_min_interval(void)
{
	return m_min_interval;
}

int sensor_info::get_max_interval(void)
{
	return m_max_interval;
}

int sensor_info::get_max_batch_count(void)
{
	return m_max_batch_count;
}

bool sensor_info::is_wakeup_supported(void)
{
	return m_wakeup_supported;
}

std::string &sensor_info::get_privilege(void)
{
	return m_privilege;
}

void sensor_info::set_type(sensor_type_t type)
{
	m_type = type;
}

void sensor_info::set_uri(const char *name)
{
	m_uri = name;
}

void sensor_info::set_model(const char *model)
{
	m_model = model;
}

void sensor_info::set_vendor(const char *vendor)
{
	m_vendor = vendor;
}

void sensor_info::set_min_range(float min_range)
{
	m_min_range = min_range;

	if (!std::isnormal(m_min_range))
		m_min_range = 0; /* set value to 0 when the value is NaN, infinity, zero or subnormal */
	if (m_min_range < MIN_RANGE)
		m_min_range = MIN_RANGE;
	if (m_min_range > MAX_RANGE)
		m_min_range = MAX_RANGE;
}

void sensor_info::set_max_range(float max_range)
{
	m_max_range = max_range;

	if (!std::isnormal(m_max_range))
		m_max_range = 0; /* set value to 0 when the value is NaN, infinity, zero or subnormal */
	if (m_max_range < MIN_RANGE)
		m_max_range = MIN_RANGE;
	if (m_max_range > MAX_RANGE)
		m_max_range = MAX_RANGE;
}

void sensor_info::set_resolution(float resolution)
{
	m_resolution = resolution;
}

void sensor_info::set_min_interval(int min_interval)
{
	m_min_interval = min_interval;
}

void sensor_info::set_max_interval(int max_interval)
{
	m_max_interval = max_interval;
}

void sensor_info::set_max_batch_count(int max_batch_count)
{
	m_max_batch_count = max_batch_count;
}

void sensor_info::set_wakeup_supported(bool supported)
{
	m_wakeup_supported = supported;
}

void sensor_info::set_privilege(const char *privilege)
{
	m_privilege = privilege;
}

void sensor_info::add_privilege(const char *privilege)
{
	if (!m_privilege.empty())
		m_privilege.append(PRIV_DELIMITER);
	m_privilege.append(privilege);
}

void sensor_info::serialize(raw_data_t &data)
{
	put(data, m_type);
	put(data, m_uri);
	put(data, m_model);
	put(data, m_vendor);
	put(data, m_min_range);
	put(data, m_max_range);
	put(data, m_resolution);
	put(data, m_min_interval);
	//put(data, m_max_interval); to do
	put(data, m_max_batch_count);
	put(data, m_wakeup_supported);
	put(data, m_privilege);
}

void sensor_info::deserialize(const char *data, int data_len)
{
	int type;

	raw_data_t raw_data(&data[0], &data[data_len]);
	auto it = raw_data.begin();
	it = get(it, type);
	m_type = (sensor_type_t)type;

	it = get(it, m_uri);
	it = get(it, m_model);
	it = get(it, m_vendor);
	it = get(it, m_min_range);
	it = get(it, m_max_range);
	it = get(it, m_resolution);
	it = get(it, m_min_interval);
	//it = get(it, m_max_interval); to do
	it = get(it, m_max_batch_count);
	it = get(it, m_wakeup_supported);
	it = get(it, m_privilege);
}

void sensor_info::show(void)
{
	_I("URI = %s", m_uri.c_str());
	_I("Model = %s", m_model.c_str());
	_I("Vendor = %s", m_vendor.c_str());
	_I("Min_range = %f", m_min_range);
	_I("Max_range = %f", m_max_range);
	_I("Resolution = %f", m_resolution);
	_I("Min_interval = %d", m_min_interval);
	_I("Max_batch_count = %d", m_max_batch_count);
	_I("Wakeup_supported = %d", m_wakeup_supported);
	_I("Privilege = %s", m_privilege.c_str());
}

void sensor_info::clear(void)
{
	m_type = UNKNOWN_SENSOR;
	m_uri.clear();
	m_model.clear();
	m_vendor.clear();
	m_min_range = 0.0f;
	m_max_range = 0.0f;
	m_resolution = 0.0f;
	m_min_interval = 0;
	m_max_interval = 0;
	m_max_batch_count = 0;
	m_wakeup_supported = false;
	m_privilege.clear();
}

void sensor_info::put(raw_data_t &data, int value)
{
	char buffer[sizeof(value)];

	int *temp = reinterpret_cast<int *>(buffer);
	*temp = value;

	copy(&buffer[0], &buffer[sizeof(buffer)], back_inserter(data));
}

void sensor_info::put(raw_data_t &data, unsigned int value)
{
	char buffer[sizeof(value)];

	unsigned int *temp = reinterpret_cast<unsigned int *>(buffer);
	*temp = value;

	copy(&buffer[0], &buffer[sizeof(buffer)], back_inserter(data));
}

void sensor_info::put(raw_data_t &data, int64_t value)
{
	char buffer[sizeof(value)];

	int64_t *temp = reinterpret_cast<int64_t *>(buffer);
	*temp = value;

	copy(&buffer[0], &buffer[sizeof(buffer)], back_inserter(data));
}

void sensor_info::put(raw_data_t &data, float value)
{
	char buffer[sizeof(value)];

	float *temp = reinterpret_cast<float *>(buffer);
	*temp = value;

	copy(&buffer[0], &buffer[sizeof(buffer)], back_inserter(data));
}

void sensor_info::put(raw_data_t &data, std::string &value)
{
	put(data, (int) value.size());

	copy(value.begin(), value.end(), back_inserter(data));
}

void sensor_info::put(raw_data_t &data, bool value)
{
	char buffer[sizeof(value)];

	bool *temp = (bool *) buffer;
	*temp = value;

	copy(&buffer[0], &buffer[sizeof(buffer)], back_inserter(data));
}

raw_data_iterator sensor_info::get(raw_data_iterator it, int &value)
{
	copy(it, it + sizeof(value), (char*) &value);

	return it + sizeof(value);
}

raw_data_iterator sensor_info::get(raw_data_iterator it, unsigned int &value)
{
	copy(it, it + sizeof(value), (char*) &value);

	return it + sizeof(value);
}

raw_data_iterator sensor_info::get(raw_data_iterator it, int64_t &value)
{
	copy(it, it + sizeof(value), (char*) &value);

	return it + sizeof(value);
}

raw_data_iterator sensor_info::get(raw_data_iterator it, float &value)
{
	copy(it, it + sizeof(value), (char*) &value);

	return it + sizeof(value);
}

raw_data_iterator sensor_info::get(raw_data_iterator it, std::string &value)
{
	int len;

	it = get(it, len);

	copy(it, it + len, back_inserter(value));

	return it + len;
}

raw_data_iterator sensor_info::get(raw_data_iterator it, bool &value)
{
	copy(it, it + sizeof(value), (char*) &value);

	return it + sizeof(value);
}
