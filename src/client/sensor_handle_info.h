/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#ifndef _SENSOR_HANDLE_INFO_H_
#define _SENSOR_HANDLE_INFO_H_

#include <sensor_internal.h>
#include <reg_event_info.h>
#include <sensor_log.h>
#include <string.h>
#include <unordered_map>
#include <string>
#include <map>

class attribute_info {
public:
	attribute_info();
	~attribute_info();

	char *get(void);
	bool set(const char *value, unsigned int len);

	unsigned int size(void);

private:
	char *m_attr;
	unsigned int m_len;
};

typedef std::unordered_map<unsigned int, reg_event_info> event_info_map;
typedef std::map<int, int> sensor_attribute_int_map;
typedef std::map<int, attribute_info *> sensor_attribute_str_map;

class sensor_handle_info {
public:
	sensor_handle_info();
	~sensor_handle_info();

	bool add_reg_event_info(unsigned int event_type, unsigned int interval, unsigned int latency, void *cb, void *user_data);
	bool delete_reg_event_info(unsigned int event_type);

	bool change_reg_event_batch(unsigned int event_type, unsigned int interval, unsigned int latency);

	reg_event_info* get_reg_event_info(const unsigned int event_type);
	void get_reg_event_types(event_type_vector &event_types);
	void get_batch(unsigned int &interval, unsigned int &latency);
	unsigned int get_reg_event_count(void);

	void clear(void);
	void clear_all_events(void);
	static unsigned long long renew_event_id(void);

	bool get_passive_mode(void);
	void set_passive_mode(bool passive);
	bool is_started(void);

	int m_handle;
	sensor_id_t m_sensor_id;
	int m_sensor_state;
	int m_pause_policy;
	int m_bad_accuracy;
	int m_accuracy;
	sensor_accuracy_changed_cb_t m_accuracy_cb;
	void *m_accuracy_user_data;
	bool m_passive;
	sensor_attribute_int_map attributes_int;
	sensor_attribute_str_map attributes_str;

private:
	event_info_map m_reg_event_infos;
	static unsigned long long m_event_id;
};

#endif /* _SENSOR_HANDLE_INFO_H_ */
