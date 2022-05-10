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

#ifndef __SENSOR_LISTENER_H__
#define __SENSOR_LISTENER_H__

#include <ipc_client.h>
#include <channel.h>
#include <channel_handler.h>
#include <event_loop.h>
#include <sensor_info.h>
#include <sensor_types.h>
#include <cmutex.h>
#include <map>
#include <atomic>
#include <vector>

namespace sensor {

class sensor_listener {
public:
	sensor_listener(sensor_t sensor);
	sensor_listener(sensor_t sensor, ipc::event_loop *loop);
	virtual ~sensor_listener();

	int get_id(void);
	sensor_t get_sensor(void);

	ipc::channel_handler *get_event_handler(void);
	ipc::channel_handler *get_accuracy_handler(void);
	ipc::channel_handler *get_attribute_int_changed_handler(void);
	ipc::channel_handler *get_attribute_str_changed_handler(void);

	/* TODO: modify the listener so that it can register multiple handlers(1:n) */
	void set_event_handler(ipc::channel_handler *handler);
	void set_accuracy_handler(ipc::channel_handler *handler);
	void set_attribute_int_changed_handler(ipc::channel_handler *handler);
	void set_attribute_str_changed_handler(ipc::channel_handler *handler);

	void unset_event_handler(void);
	void unset_accuracy_handler(void);
	void unset_attribute_int_changed_handler(void);
	void unset_attribute_str_changed_handler(void);

	int start(void);
	int stop(void);

	int get_interval(void);
	int get_max_batch_latency(void);
	int get_pause_policy(void);
	int get_passive_mode(void);

	int set_interval(unsigned int interval);
	int set_max_batch_latency(unsigned int max_batch_latency);
	int set_passive_mode(bool passive);
	int set_attribute(int attribute, int value);
	int get_attribute(int attribute, int* value);
	void update_attribute(int attribute, int value);
	int set_attribute(int attribute, const char *value, int len);
	int get_attribute(int attribute, char **value, int *len);
	void update_attribute(int attribute, const char *value, int len);
	int get_sensor_data(sensor_data_t *data);
	int get_sensor_data_list(sensor_data_t **data, int *count);
	int flush(void);

	void restore(void);

private:
	bool init(void);
	void deinit(void);

	bool connect(void);
	void disconnect(void);
	bool is_connected(void);

	int m_id;
	sensor_info *m_sensor;

	ipc::ipc_client *m_client;
	ipc::channel *m_cmd_channel;
	ipc::channel *m_evt_channel;
	ipc::channel_handler *m_handler;
	ipc::channel_handler *m_evt_handler;
	ipc::channel_handler *m_acc_handler;
	ipc::channel_handler *m_attr_int_changed_handler;
	ipc::channel_handler *m_attr_str_changed_handler;

	ipc::event_loop *m_loop { nullptr };
	std::atomic<bool> m_connected;
	std::atomic<bool> m_started;
	std::map<int, int> m_attributes_int;
	std::map<int, std::vector<char>> m_attributes_str;

	cmutex lock;
};

}

#endif /* __SENSOR_LISTENER_H__ */
