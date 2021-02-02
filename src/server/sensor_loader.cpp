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

#include "sensor_loader.h"

#include <dirent.h>
#include <dlfcn.h>
#include <fusion_sensor.h>
#include <hal/hal-sensor.h>
#include <physical_sensor.h>
#include <sensor_log.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>

using namespace sensor;

sensor_loader::sensor_loader() {
  if (hal_sensor_get_backend() != 0) {
    _E("Failed to load hal sensor backend");
  }
}

sensor_loader::~sensor_loader() {
  if (hal_sensor_put_backend() != 0) {
    _E("Failed to clear hal sensor backend");
  }
}

void sensor_loader::load_hal(device_sensor_registry_t &devices) {
  void **results = nullptr;

  int size = hal_sensor_create(&results);
  if (size <= 0 || !results) {
    _E("Failed to get sensor from hal sensor backend");
    return;
  }

  for (int i = 0; i < size; ++i) {
    devices.emplace_back(static_cast<sensor_device *>(results[i]));
  }
  _I("Success to load sensor from hal sensor backend");
}


void sensor_loader::load_hal_legacy(const std::string &path, device_sensor_registry_t &devices)
{
	load<sensor_device>(path, devices);
}

void sensor_loader::load_physical_sensor(const std::string &path, physical_sensor_registry_t &sensors)
{
	load<physical_sensor>(path, sensors);
}

void sensor_loader::load_fusion_sensor(const std::string &path, fusion_sensor_registry_t &sensors)
{
	load<fusion_sensor>(path, sensors);
}

void sensor_loader::load_external_sensor(const std::string &path, external_sensor_registry_t &sensors)
{
	load<external_sensor>(path, sensors);
}

void sensor_loader::unload(void)
{
	for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
		dlclose(it->second);
}

template<typename T>
bool sensor_loader::load(const std::string &dir_path, std::vector<std::shared_ptr<T>> &sensors)
{
	bool ret;
	void *handle;
	std::vector<std::string> module_paths;
	void **results;

	ret = get_module_paths(dir_path, module_paths);
	retv_if(!ret, false);

	for (auto &path : module_paths) {
		_I("Load sensor devices from %s", path.c_str());
		handle = dlopen(path.c_str(), RTLD_NOW);
		retvm_if(!handle, false, "Failed to dlopen from %s because %s", path.c_str(), dlerror());

		/* TODO: out-param of the create function should be const */
		create_t create = reinterpret_cast<create_t>(dlsym(handle, "create"));
		if (!create) {
			_E("Failed to find symbols from %s", path.c_str());
			dlclose(handle);
			return false;
		}

		int size = create(&results);
		if (size <= 0 || !results) {
			_E("Failed to create sensors from %s", path.c_str());
			dlclose(handle);
			return false;
		}

		for (int i = 0; i < size; ++i)
			sensors.emplace_back(static_cast<T *>(results[i]));

		m_modules[path.c_str()] = handle;
		_I("Success to load sensor devices from %s", path.c_str());
	}

	return true;
}

bool sensor_loader::get_module_paths(const std::string &dir_path, std::vector<std::string> &paths)
{
	DIR *dir = NULL;
	struct dirent *entry;
	struct stat buf;
	std::string filename;

	dir = opendir(dir_path.c_str());
	retvm_if(!dir, false, "Failed to open directory[%s]", dir_path.c_str());

	while (true) {
		entry = readdir(dir);
		if (!entry) break;

		filename = std::string(entry->d_name);

		if (filename == "." || filename == "..")
			continue;

		std::string full_path = dir_path + "/" + filename;

		if (lstat(full_path.c_str(), &buf) != 0)
			break;

		if (S_ISDIR(buf.st_mode))
			continue;

		paths.push_back(full_path);
	}
	closedir(dir);

	return true;
}
