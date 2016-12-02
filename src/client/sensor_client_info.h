/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#ifndef _SENSOR_CLIENT_INFO_H_
#define _SENSOR_CLIENT_INFO_H_

#include <glib.h>
#include <sys/types.h>
#include <unistd.h>
#include <csocket.h>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cmutex.h>
#include <sensor_handle_info.h>
#include <command_channel.h>

typedef std::vector<unsigned int> handle_vector;
typedef std::vector<sensor_id_t> sensor_id_vector;
typedef std::unordered_map<int, sensor_handle_info> sensor_handle_info_map;
typedef std::unordered_map<sensor_id_t, command_channel *> sensor_command_channel_map;

typedef struct sensor_rep {
	bool active;
	int pause_policy;
	unsigned int interval;
	unsigned int latency;
	event_type_vector event_types;
} sensor_rep;

class sensor_client_info {
public:
	static sensor_client_info& get_instance(void);
	int create_handle(sensor_id_t sensor_id);
	bool delete_handle(int handle);
	bool register_event(int handle, unsigned int event_type,
			unsigned int interval, unsigned int latency, void *cb, void* user_data);
	bool unregister_event(int handle, unsigned int event_type);

	bool register_accuracy_cb(int handle, sensor_accuracy_changed_cb_t cb, void* user_data);
	bool unregister_accuracy_cb(int handle);

	bool set_sensor_params(int handle, int sensor_state, int sensor_pause_policy);
	bool get_sensor_params(int handle, int &sensor_state, int &sensor_pause_policy);
	bool set_sensor_state(int handle, int sensor_state);

	bool get_passive_mode(int handle);
	bool set_passive_mode(int handle, bool passive);

	bool set_attribute(int handle, int attribute, int value);
	bool set_attribute(int handle, int attribute, const char *value, int len);

	bool set_sensor_pause_policy(int handle, int pause_policy);
	bool set_event_batch(int handle, unsigned int event_type, unsigned int interval, unsigned int latency);
	bool set_accuracy(int handle, int accuracy);
	bool set_bad_accuracy(int handle, int bad_accuracy);
	bool get_event_info(int handle, unsigned int event_type, unsigned int &interval, unsigned int &latency, void* &cb, void* &user_data);
	void get_listening_sensors(sensor_id_vector &sensors);
	void get_sensor_rep(sensor_id_t sensor, sensor_rep& rep);

	bool get_active_batch(sensor_id_t sensor, unsigned int &interval, unsigned int &latency);
	unsigned int get_active_pause_policy(sensor_id_t sensor_id);
	void get_active_event_types(sensor_id_t sensor_id, event_type_vector &active_event_types);

	bool get_sensor_id(int handle, sensor_id_t &sensor_id);
	bool get_sensor_state(int handle, int &state);

	bool has_client_id(void);
	int get_client_id(void);
	void set_client_id(int client_id);

	bool is_active(void);
	bool is_sensor_registered(sensor_id_t sensor_id);
	bool is_sensor_active(sensor_id_t sensor_id);
	bool is_event_active(int handle, unsigned int event_type, unsigned long long event_id);

	bool add_command_channel(sensor_id_t sensor_id, command_channel *cmd_channel);
	bool get_command_channel(sensor_id_t sensor_id, command_channel **cmd_channel);
	bool close_command_channel(void);
	bool close_command_channel(sensor_id_t sensor_id);

	void get_all_handles(handle_vector &handles);
	void get_sensor_handle_info(sensor_id_t sensor, sensor_handle_info_map &handles_info);
	void get_all_handle_info(sensor_handle_info_map &handles_info);

	void set_pause_policy(sensor_id_t sensor, int power_save_state);

	void clear(void);

	sensor_client_info();
	~sensor_client_info();

private:
	sensor_handle_info_map m_sensor_handle_infos;
	sensor_command_channel_map m_command_channels;

	int m_client_id;

	cmutex m_handle_info_lock;
};

#endif /* _SENSOR_CLIENT_INFO_H_ */
