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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sensor_log.h>
#include <sensor_hal.h>
#include <physical_sensor.h>
#include <fusion_sensor.h>
#include <memory>

using namespace sensor;

sensor_loader::sensor_loader()
{
}

sensor_loader::~sensor_loader()
{
}

void sensor_loader::load_hal(const std::string &path, device_sensor_registry_t &devices)
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
	}

	return true;
}

bool sensor_loader::get_module_paths(const std::string &dir_path, std::vector<std::string> &paths)
{
	int ret;
	DIR *dir = NULL;
	struct dirent entry;
	struct dirent *result;
	struct stat buf;
	std::string filename;

	dir = opendir(dir_path.c_str());
	retvm_if(!dir, false, "Failed to open directory[%s]", dir_path.c_str());

	while (true) {
		ret = readdir_r(dir, &entry, &result);

		if (ret != 0)
			continue;
		if (!result)
			break;

		filename = std::string(entry.d_name);

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
