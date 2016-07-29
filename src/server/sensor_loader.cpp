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

#include <dlfcn.h>
#include <dirent.h>
#include <sensor_common.h>
#include <sensor_loader.h>
#include <sensor_hal.h>
#include <sensor_base.h>
#include <sensor_log.h>
#include <physical_sensor.h>
#include <virtual_sensor.h>
#include <external_sensor.h>
#include <unordered_set>
#include <algorithm>
#include <memory>

#include <hrm_sensor.h>

#ifdef ENABLE_AUTO_ROTATION
#include <auto_rotation_sensor.h>
#endif
#ifdef ENABLE_GRAVITY
#include <gravity_sensor.h>
#endif
#ifdef ENABLE_LINEAR_ACCEL
#include <linear_accel_sensor.h>
#endif
#ifdef ENABLE_ORIENTATION
#include <orientation_sensor.h>
#endif
#ifdef ENABLE_ROTATION_VECTOR
#include <rv_sensor.h>
#include <magnetic_rv_sensor.h>
#include <gyro_rv_sensor.h>
#endif
#ifdef ENABLE_FACE_DOWN
#include <face_down_sensor.h>
#endif

using std::vector;
using std::string;

#define DEVICE_HAL_DIR_PATH "/usr/lib/sensor"

sensor_loader::sensor_loader()
{
}

sensor_loader::~sensor_loader()
{
	sensor_device_map_t::iterator it_device;
	std::vector<void *>::iterator it_handle;

	for (it_device = m_active_devices.begin(); it_device != m_active_devices.end();)
		it_device = m_active_devices.erase(it_device);

	for (it_handle = m_handles.begin(); it_handle != m_handles.end(); ++it_handle)
		dlclose(*it_handle);

	m_handles.clear();
}

sensor_loader& sensor_loader::get_instance(void)
{
	static sensor_loader inst;
	return inst;
}

bool sensor_loader::load(void)
{
	std::vector<string> device_hal_paths;
	std::vector<string> unique_device_hal_paths;

	get_paths_from_dir(string(DEVICE_HAL_DIR_PATH), device_hal_paths);

	std::unordered_set<string> s;
	auto unique = [&s](vector<string> &paths, const string &path) {
		if (s.insert(path).second)
			paths.push_back(path);
	};

	for_each(device_hal_paths.begin(), device_hal_paths.end(),
		[&](const string &path) {
			unique(unique_device_hal_paths, path);
		}
	);

	for_each(unique_device_hal_paths.begin(), unique_device_hal_paths.end(),
		[&](const string &path) {
			void *handle;
			load_sensor_devices(path, handle);
			m_handles.push_back(handle);
		}
	);

	create_sensors();
	show_sensor_info();

	return true;
}

bool sensor_loader::load_sensor_devices(const string &path, void* &handle)
{
	sensor_device_t *_devices = NULL;
	sensor_device *device = NULL;
	const sensor_info_t *infos;

	_I("load device: [%s]", path.c_str());

	void *_handle = dlopen(path.c_str(), RTLD_NOW);
	if (!_handle) {
		_E("Failed to dlopen(%s), dlerror : %s", path.c_str(), dlerror());
		return false;
	}

	dlerror();

	/* TODO: The out-param of the create function should be const */
	create_t create_devices = (create_t) dlsym(_handle, "create");
	if (!create_devices) {
		_E("Failed to find symbols in %s", path.c_str());
		dlclose(_handle);
		return false;
	}

	int device_size = create_devices(&_devices);
	if (!_devices) {
		_E("Failed to create devices, path is %s\n", path.c_str());
		dlclose(_handle);
		return false;
	}

	for (int i = 0; i < device_size; ++i) {
		device = static_cast<sensor_device *>(_devices[i]);
		std::shared_ptr<sensor_device> device_ptr(device);

		int info_size = device_ptr->get_sensors(&infos);
		for (int j = 0; j < info_size; ++j)
			m_devices[&infos[j]] = device_ptr;
	}

	handle = _handle;

	return true;
}

void sensor_loader::create_sensors(void)
{
	/* HRM sensors need SENSOR_PERMISSION_BIO */
	create_physical_sensors<hrm_sensor>(HRM_RAW_SENSOR);
	create_physical_sensors<hrm_sensor>(HRM_SENSOR);
	create_physical_sensors<hrm_sensor>(HRM_LED_GREEN_SENSOR);
	create_physical_sensors<hrm_sensor>(HRM_LED_IR_SENSOR);
	create_physical_sensors<hrm_sensor>(HRM_LED_RED_SENSOR);

	create_physical_sensors<physical_sensor>(UNKNOWN_SENSOR);

#ifdef ENABLE_AUTO_ROTATION
	create_virtual_sensors<auto_rotation_sensor>("Auto Rotation");
#endif
#ifdef ENABLE_ROTATION_VECTOR
	create_virtual_sensors<rv_sensor>("Rotation Vector");
	create_virtual_sensors<magnetic_rv_sensor>("Magnetic Rotation Vector");
	create_virtual_sensors<gyro_rv_sensor>("Gyroscope Rotation Vector");
#endif
#ifdef ENABLE_GRAVITY
	create_virtual_sensors<gravity_sensor>("Gravity");
#endif
#ifdef ENABLE_LINEAR_ACCEL
	create_virtual_sensors<linear_accel_sensor>("Linear Accel");
#endif
#ifdef ENABLE_ORIENTATION
	create_virtual_sensors<orientation_sensor>("Orientation");
#endif
#ifdef ENABLE_FACE_DOWN
	create_virtual_sensors<face_down_sensor>("Face Down");
#endif
}

template<typename _sensor>
void sensor_loader::create_physical_sensors(sensor_type_t type)
{
	int32_t index;
	const sensor_info_t *info;
	physical_sensor *sensor;
	sensor_device *device;

	sensor_device_map_t::iterator it;

	for (it = m_devices.begin(); it != m_devices.end(); ++it) {
		info = it->first;
		device = it->second.get();

		if (type != UNKNOWN_SENSOR) {
			if (type != (sensor_type_t)(info->type))
				continue;
		}

		sensor = dynamic_cast<physical_sensor *>(create_sensor<_sensor>());

		if (!sensor) {
			_E("Memory allocation failed[%s]", info->name);
			return;
		}

		sensor_type_t _type = (sensor_type_t)info->type;
		index = (int32_t)m_sensors.count(_type);

		sensor->set_id(((int64_t)_type << SENSOR_TYPE_SHIFT) | index);
		sensor->set_sensor_info(info);
		sensor->set_sensor_device(device);

		std::shared_ptr<sensor_base> sensor_ptr(sensor);
		m_sensors.insert(std::make_pair(_type, sensor_ptr));

		m_active_devices[it->first] = it->second;
		m_devices.erase(it->first);

		_I("created [%s] sensor", sensor->get_name());
	}
}

template <typename _sensor>
void sensor_loader::create_virtual_sensors(const char *name)
{
	int32_t index;
	sensor_type_t type;
	virtual_sensor *instance;

	instance = dynamic_cast<virtual_sensor *>(create_sensor<_sensor>());
	if (!instance) {
		_E("Memory allocation failed[%s]", name);
		return;
	}

	if (!instance->init()) {
		_E("Failed to init %s", name);
		delete instance;
		return;
	}

	std::shared_ptr<sensor_base> sensor(instance);
	type = sensor->get_type();
	index = (int32_t)(m_sensors.count(type));

	sensor->set_id((int64_t)type << SENSOR_TYPE_SHIFT | index);

	m_sensors.insert(std::make_pair(type, sensor));

	_I("created [%s] sensor", sensor->get_name());
}

template <typename _sensor>
void sensor_loader::create_external_sensors(const char *name)
{
	int32_t index;
	sensor_type_t type;
	external_sensor *instance;

	instance = dynamic_cast<external_sensor *>(create_sensor<_sensor>());
	if (!instance) {
		_E("Memory allocation failed[%s]", name);
		return;
	}

	std::shared_ptr<sensor_base> sensor(instance);
	type = sensor->get_type();
	index = (int32_t)(m_sensors.count(type));

	sensor->set_id((int64_t)type << SENSOR_TYPE_SHIFT | index);

	m_sensors.insert(std::make_pair(type, sensor));

	_I("created [%s] sensor", sensor->get_name());
}

template <typename _sensor>
sensor_base* sensor_loader::create_sensor(void)
{
	sensor_base *instance = NULL;

	try {
		instance = new _sensor;
	} catch (std::exception &e) {
		_E("Failed to create sensor, exception: %s", e.what());
		return NULL;
	} catch (int err) {
		_ERRNO(errno, _E, "Failed to create sensor");
		return NULL;
	}

	return instance;
}

void sensor_loader::show_sensor_info(void)
{
	_I("========== Loaded sensor information ==========\n");

	int index = 0;

	auto it = m_sensors.begin();

	while (it != m_sensors.end()) {
		sensor_base *sensor = it->second.get();

		sensor_info info;
		sensor->get_sensor_info(info);
		_I("No:%d [%s]\n", ++index, sensor->get_name());
		info.show();
		it++;
	}

	_I("===============================================\n");
}

bool sensor_loader::get_paths_from_dir(const string &dir_path, vector<string> &hal_paths)
{
	DIR *dir = NULL;
	struct dirent dir_entry;
	struct dirent *result;
	string name;
	int ret;

	dir = opendir(dir_path.c_str());

	if (!dir) {
		_E("Failed to open dir: %s", dir_path.c_str());
		return false;
	}

	while (true) {
		ret = readdir_r(dir, &dir_entry, &result);

		if (ret != 0)
			continue;

		if (result == NULL)
			break;

		name = string(dir_entry.d_name);

		if (name == "." || name == "..")
			continue;

		hal_paths.push_back(dir_path + "/" + name);
	}

	closedir(dir);
	return true;
}

sensor_base* sensor_loader::get_sensor(sensor_type_t type)
{
	auto it = m_sensors.find(type);

	if (it == m_sensors.end())
		return NULL;

	return it->second.get();
}

sensor_base* sensor_loader::get_sensor(sensor_id_t id)
{
	vector<sensor_base *> sensors;

	sensor_type_t type = static_cast<sensor_type_t>(id >> SENSOR_TYPE_SHIFT);
	unsigned int index = (id & SENSOR_INDEX_MASK);

	sensors = get_sensors(type);

	if (index >= sensors.size())
		return NULL;

	return sensors[index];
}

vector<sensor_type_t> sensor_loader::get_sensor_types(void)
{
	vector<sensor_type_t> sensor_types;

	auto it = m_sensors.begin();

	while (it != m_sensors.end()) {
		sensor_types.push_back((sensor_type_t)(it->first));
		it = m_sensors.upper_bound(it->first);
	}

	return sensor_types;
}

vector<sensor_base *> sensor_loader::get_sensors(sensor_type_t type)
{
	vector<sensor_base *> sensor_list;
	std::pair<sensor_map_t::iterator, sensor_map_t::iterator> ret;

	if ((int)(type) == (int)SENSOR_DEVICE_ALL)
		ret = std::make_pair(m_sensors.begin(), m_sensors.end());
	else
		ret = m_sensors.equal_range(type);

	for (auto it = ret.first; it != ret.second; ++it)
		sensor_list.push_back(it->second.get());

	return sensor_list;
}

vector<sensor_base *> sensor_loader::get_virtual_sensors(void)
{
	vector<sensor_base *> virtual_list;
	sensor_base* sensor;

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		sensor = it->second.get();

		if (!sensor || !sensor->is_virtual())
			continue;

		virtual_list.push_back(sensor);
	}

	return virtual_list;
}
