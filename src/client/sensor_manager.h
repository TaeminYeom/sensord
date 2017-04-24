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

#ifndef __SENSOR_MANAGER_H__
#define __SENSOR_MANAGER_H__

#include <channel.h>
#include <sensor_info.h>
#include <ipc_client.h>
#include <event_loop.h>
#include <vector>
#include <atomic>

#include "sensor_internal.h"
#include "sensor_provider.h"
#include "sensor_manager_handler.h"

namespace sensor {

class sensor_manager {
public:
	sensor_manager();
	virtual ~sensor_manager();

	bool connect(void);
	void disconnect(void);
	void restore(void);

	int get_sensor(const char *uri, sensor_t *sensor);
	int get_sensors(const char *uri, sensor_t **list, int *count);

	bool is_supported(sensor_t sensor);
	bool is_supported(const char *uri);

	/* sensor provider */
	int add_sensor(sensor_info &info);
	int add_sensor(sensor_provider *provider);
	int remove_sensor(const char *uri);
	int remove_sensor(sensor_provider *provider);

	void add_sensor_added_cb(sensord_added_cb cb, void *user_data);
	void remove_sensor_added_cb(sensord_added_cb cb);

	void add_sensor_removed_cb(sensord_removed_cb cb, void *user_data);
	void remove_sensor_removed_cb(sensord_removed_cb cb);

private:
	typedef std::vector<sensor_info> sensor_list_t;

	bool init(void);
	void deinit(void);

	bool connect_channel(void);
	bool is_connected(void);

	void decode_sensors(const char *buf, std::vector<sensor_info> &infos);
	bool get_sensors_internal(void);

	bool has_privilege(std::string &uri);
	sensor_info *get_info(const char *uri);
	std::vector<sensor_info *> get_infos(const char *uri);

	ipc::ipc_client *m_client;
	ipc::channel *m_channel;
	ipc::event_loop m_loop;
	std::atomic<bool> m_connected;
	sensor_manager_handler *m_handler;

	sensor_list_t m_sensors;
};

}

#endif /* __SENSOR_MANAGER_H__ */
