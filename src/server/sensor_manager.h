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

#include <string>
#include <vector>
#include <unordered_map>

#include <channel.h>
#include <event_loop.h>

#include "sensor_handler.h"
#include "sensor_observer.h"
#include "sensor_loader.h"

#include "physical_sensor_handler.h"
#include "fusion_sensor_handler.h"
#include "external_sensor_handler.h"

namespace sensor {

class sensor_manager {
public:
	sensor_manager(ipc::event_loop *loop);
	~sensor_manager();

	bool init(void);
	bool deinit(void);

	bool is_supported(const std::string uri);

	bool register_sensor(sensor_handler *sensor);
	void deregister_sensor(const std::string uri);

	void register_channel(ipc::channel *ch);
	void deregister_channel(ipc::channel *ch);

	sensor_handler *get_sensor_by_type(const std::string uri);
	sensor_handler *get_sensor(const std::string uri);
	std::vector<sensor_handler *> get_sensors(void);

	size_t serialize(int sock_fd, char **bytes);

private:
	typedef std::map<std::string, sensor_handler *> sensor_map_t;

	void create_physical_sensors(
			device_sensor_registry_t &devices,
			physical_sensor_registry_t &psensors);
	void create_fusion_sensors(fusion_sensor_registry_t &vsensors);
	void create_external_sensors(external_sensor_registry_t &vsensors);

	void init_sensors(void);
	void register_handler(physical_sensor_handler *sensor);

	int serialize(sensor_info *info, char **bytes);

	void send(ipc::message &msg);
	void send_added_msg(sensor_info *info);
	void send_removed_msg(const std::string &uri);

	void show(void);

	ipc::event_loop *m_loop;
	sensor_loader m_loader;
	sensor_map_t m_sensors;

	std::vector<ipc::channel *> m_channels;
};

}

#endif /* __SENSOR_MANAGER_H__ */
