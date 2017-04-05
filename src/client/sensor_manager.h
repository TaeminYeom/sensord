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

namespace sensor {

class sensor_manager {
public:
	sensor_manager();
	virtual ~sensor_manager();

	bool connect(void);
	void disconnect(void);

	int get_sensor(sensor_type_t type, sensor_t *sensor);
	int get_sensors(sensor_type_t type, sensor_t **list, int *count);
	int get_sensor(const char *uri, sensor_t *sensor);
	int get_sensors(const char *uri, sensor_t **list, int *count);

	bool is_supported(sensor_t sensor);
	bool is_supported(const char *uri);

	void restore(void);

	/* TODO: register sensor_provider by using manager */
	/* int register_sensor(sensor_provider *provider); */
	/* int unregister_sensor(const char *uri) */

private:
	typedef std::vector<sensor_info> sensor_infos_t;

	bool init(void);
	void deinit(void);

	bool connect_channel(void);
	bool is_connected(void);

	void decode_sensors(const char *buf, std::vector<sensor_info> &infos);
	bool get_sensors_internal(void);

	sensor_info *get_info(const char *uri);
	std::vector<sensor_info *> get_infos(const char *uri);

	ipc::ipc_client *m_client;
	ipc::channel_handler *m_handler;
	ipc::channel *m_channel;
	ipc::event_loop m_loop;
	std::atomic<bool> m_connected;

	sensor_infos_t m_infos;
};

}

#endif /* __SENSOR_MANAGER_H__ */
