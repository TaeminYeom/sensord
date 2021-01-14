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

#ifndef __SENSOR_LOADER_H__
#define __SENSOR_LOADER_H__

#include <physical_sensor.h>
#include <fusion_sensor.h>
#include <external_sensor.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace sensor {

typedef std::vector<std::shared_ptr<sensor_device>> device_sensor_registry_t;
typedef std::vector<std::shared_ptr<physical_sensor>> physical_sensor_registry_t;
typedef std::vector<std::shared_ptr<fusion_sensor>> fusion_sensor_registry_t;
typedef std::vector<std::shared_ptr<external_sensor>> external_sensor_registry_t;

class sensor_loader {
public:
	sensor_loader();
	virtual ~sensor_loader();

	void load_hal(device_sensor_registry_t &devices);
	void load_hal_legacy(const std::string &path, device_sensor_registry_t &devices);
	void load_physical_sensor(const std::string &path, physical_sensor_registry_t &sensors);
	void load_fusion_sensor(const std::string &path, fusion_sensor_registry_t &sensors);
	void load_external_sensor(const std::string &path, external_sensor_registry_t &sensors);

	void unload(void);

private:
	template<typename T>
	bool load(const std::string &path, std::vector<std::shared_ptr<T>> &sensors);

	bool get_module_paths(const std::string &dir_path, std::vector<std::string> &paths);

	std::map<std::string, void *> m_modules;
};

}

#endif	/* __SENSOR_LOADER_H__ */
