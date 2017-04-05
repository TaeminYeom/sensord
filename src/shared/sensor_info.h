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

#ifndef __SENSOR_INFO_H__
#define __SENSOR_INFO_H__

#include <stdint.h>
#include <string>
#include <vector>
#include <sensor_hal.h>
#include <sensor_types.h>

namespace sensor {

typedef std::vector<char> raw_data_t;
typedef raw_data_t::iterator raw_data_iterator;

/* TODO: builder */
class sensor_info {
public:
	sensor_info();
	sensor_info(const sensor_info &info);
	sensor_info(const sensor_info_t &info);

	/* TODO: it would be better to get_type() returns type(URI) */
	sensor_type_t get_type(void);
	std::string &get_type_uri(void);
	std::string &get_uri(void);
	std::string &get_model(void);
	std::string & get_vendor(void);
	float get_min_range(void);
	float get_max_range(void);
	float get_resolution(void);
	int get_min_interval(void);
	int get_max_batch_count(void);
	bool is_wakeup_supported(void);
	sensor_permission_t get_permission(void);

	/* TODO: it would be better to get_type() returns type(URI) */
	void set_type(sensor_type_t type);
	void set_type_uri(const char *type_uri);
	void set_uri(const char *name);
	void set_model(const char *name);
	void set_vendor(const char *vendor);
	void set_min_range(float min_range);
	void set_max_range(float max_range);
	void set_resolution(float resolution);
	void set_min_interval(int min_interval);
	void set_max_batch_count(int max_batch_count);
	void set_wakeup_supported(bool supported);
	void set_permission(sensor_permission_t permission);

	void clear(void);

	void serialize(raw_data_t &data);
	void deserialize(const char *data, int data_len);
	void show(void);

private:
	sensor_type_t m_type;
	std::string m_type_uri;
	std::string m_uri;
	std::string m_model;
	std::string m_vendor;
	float m_min_range;
	float m_max_range;
	float m_resolution;
	int m_min_interval;
	int m_max_batch_count;
	bool m_wakeup_supported;
	sensor_permission_t m_permission;

	/* TODO: use template */
	void put(raw_data_t &data, int value);
	void put(raw_data_t &data, unsigned int value);
	void put(raw_data_t &data, int64_t value);
	void put(raw_data_t &data, float value);
	void put(raw_data_t &data, std::string &value);
	void put(raw_data_t &data, bool value);

	/* TODO: use template */
	raw_data_iterator get(raw_data_iterator it, int &value);
	raw_data_iterator get(raw_data_iterator it, unsigned int &value);
	raw_data_iterator get(raw_data_iterator it, int64_t &value);
	raw_data_iterator get(raw_data_iterator it, float &value);
	raw_data_iterator get(raw_data_iterator it, std::string &value);
	raw_data_iterator get(raw_data_iterator it, bool &value);
};

}

#endif /* __SENSOR_INFO_H__ */
