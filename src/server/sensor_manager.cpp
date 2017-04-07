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

#include "sensor_manager.h"

#include <unistd.h>
#include <sensor_log.h>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "sensor_event_handler.h"
#include "permission_checker.h"
#include "sensor_loader.h"
#include "physical_sensor_handler.h"
#include "fusion_sensor_handler.h"
#include "external_sensor_handler.h"
#include "fusion_sensor_handler.h"

using namespace sensor;

#define DEVICE_HAL_DIR_PATH LIBDIR "/sensor/hal"
#define PHYSICAL_SENSOR_DIR_PATH LIBDIR "/sensor/physical"
#define VIRTUAL_SENSOR_DIR_PATH LIBDIR "/sensor/fusion"
#define EXTERNAL_SENSOR_DIR_PATH LIBDIR "/sensor/external"

static device_sensor_registry_t devices;
static physical_sensor_registry_t physical_sensors;
static fusion_sensor_registry_t fusion_sensors;
static external_sensor_registry_t external_sensors;

sensor_manager::sensor_manager(ipc::event_loop *loop)
: m_loop(loop)
{
}

sensor_manager::~sensor_manager()
{
}

bool sensor_manager::init(void)
{
	m_loader.load_hal(DEVICE_HAL_DIR_PATH, devices);
	m_loader.load_physical_sensor(PHYSICAL_SENSOR_DIR_PATH, physical_sensors);
	m_loader.load_fusion_sensor(VIRTUAL_SENSOR_DIR_PATH, fusion_sensors);
	m_loader.load_external_sensor(EXTERNAL_SENSOR_DIR_PATH, external_sensors);

	retvm_if(devices.empty(), false, "There is no sensors");

	/* TODO: support dynamic sensor */
	create_physical_sensors(devices, physical_sensors);
	create_fusion_sensors(fusion_sensors);
	create_external_sensors(external_sensors);

	init_sensors();

	show();

	return true;
}

bool sensor_manager::deinit(void)
{
	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it)
		delete it->second;
	m_sensors.clear();

	external_sensors.clear();
	fusion_sensors.clear();
	physical_sensors.clear();
	devices.clear();

	m_loader.unload();

	return true;
}

bool sensor_manager::is_supported(std::string uri)
{
	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		sensor_info info = it->second->get_sensor_info();

		if (info.get_uri() == uri)
			return true;
	}

	return false;
}

bool sensor_manager::register_sensor(sensor_handler *sensor)
{
	retvm_if(!sensor, false, "Invalid sensor");

	sensor_info info = sensor->get_sensor_info();

	auto it = m_sensors.find(info.get_uri());
	retvm_if(it != m_sensors.end(), false, "There is already a sensor with the same name");

	m_sensors[info.get_uri()] = sensor;

	_I("Registered[%s]", info.get_uri().c_str());

	return true;
}

void sensor_manager::deregister_sensor(const std::string uri)
{
	auto it = m_sensors.find(uri);
	ret_if(it == m_sensors.end());

	delete it->second;
	m_sensors.erase(it);

	_I("Deregistered[%s]", uri.c_str());
}

sensor_handler *sensor_manager::get_sensor_by_type(const std::string uri)
{
	auto it = m_sensors.begin();
	for (; it != m_sensors.end(); ++it) {
		std::size_t found = it->first.rfind(uri);
		if (found != std::string::npos)
			return it->second;
	}

	return NULL;
}

sensor_handler *sensor_manager::get_sensor(const std::string uri)
{
	auto it = m_sensors.find(uri);
	retv_if(it == m_sensors.end(), NULL);

	return m_sensors[uri];
}

std::vector<sensor_handler *> sensor_manager::get_sensors(void)
{
	std::vector<sensor_handler *> sensors;

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it)
		sensors.push_back(it->second);

	return sensors;
}

void sensor_manager::create_physical_sensors(device_sensor_registry_t &devices,
		physical_sensor_registry_t &psensors)
{
	const sensor_info_t *info;
	physical_sensor_handler *psensor;

	for (auto it = devices.begin(); it != devices.end(); ++it) {
		int count = (*it)->get_sensors(&info);

		for (int i = 0; i < count; ++i) {
			/* TODO: psensors */
			psensor = new(std::nothrow) physical_sensor_handler(
					info[i], it->get(), info[i].id, NULL);
			retm_if(!psensor, "Failed to allocate memory");

			sensor_info sinfo = psensor->get_sensor_info();
			m_sensors[sinfo.get_uri()] = psensor;
		}
	}

}

void sensor_manager::create_fusion_sensors(fusion_sensor_registry_t &fsensors)
{
	const sensor_info2_t *info;
	fusion_sensor_handler *fsensor;
	std::vector<std::string> req_names;

	for (auto it = fsensors.begin(); it != fsensors.end(); ++it) {
		int count = (*it)->get_sensors(&info);

		for (int i = 0; i < count; ++i) {
			bool support = true;
			req_names.clear();

			fsensor = new(std::nothrow) fusion_sensor_handler(info[i], it->get());
			retm_if(!fsensor, "Failed to allocate memory");

			(*it)->get_required_sensors(req_names);
			for (unsigned int j = 0; j < req_names.size(); ++j) {
				sensor_handler *sensor;
				sensor = get_sensor_by_type(req_names[j]);

				if (sensor == NULL) {
					support = false;
					break;
				}

				fsensor->add_required_sensor(sensor);
			}

			if (!support) {
				delete fsensor;
				/* TODO: remove plugin */
				continue;
			}

			sensor_info sinfo = fsensor->get_sensor_info();
			m_sensors[sinfo.get_uri()] = fsensor;
		}
	}
}

void sensor_manager::create_external_sensors(external_sensor_registry_t &esensors)
{
	const sensor_info2_t *info;
	external_sensor_handler *esensor;

	for (auto it = esensors.begin(); it != esensors.end(); ++it) {
		int count = (*it)->get_sensors(&info);

		for (int i = 0; i < count; ++i) {
			esensor = new(std::nothrow) external_sensor_handler(info[i], it->get());
			retm_if(!esensor, "Failed to allocate memory");

			sensor_info sinfo = esensor->get_sensor_info();
			m_sensors[sinfo.get_uri()] = esensor;
		}
	}
}

static void put_int_to_vec(std::vector<char> &data, int value)
{
	char buf[sizeof(value)];

	int *temp = (int *)buf;
	*temp = value;

	std::copy(&buf[0], &buf[sizeof(buf)], back_inserter(data));
}

/* packet format :
 * [count:4] {[size:4] [info:n] [size:4] [info:n] ...}
 */
size_t sensor_manager::serialize(int sock_fd, char **bytes)
{
	static permission_checker checker;

	sensor_info info;
	std::vector<char> raw_list;

	put_int_to_vec(raw_list, m_sensors.size());

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		info = it->second->get_sensor_info();

		raw_data_t *raw = new(std::nothrow) raw_data_t();
		retvm_if(!raw, -ENOMEM, "Failed to allocated memory");

		info.serialize(*raw);

		/* copy size */
		put_int_to_vec(raw_list, raw->size());

		/* copy info */
		std::copy(raw->begin(), raw->end(), std::back_inserter(raw_list));

		delete raw;
	}

	*bytes = new(std::nothrow) char[raw_list.size()];
	retvm_if(!*bytes, -ENOMEM, "Failed to allocate memory");

	std::copy(raw_list.begin(), raw_list.end(), *bytes);

	return raw_list.size();
}

void sensor_manager::init_sensors(void)
{
	physical_sensor_handler *sensor;

	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		sensor = dynamic_cast<physical_sensor_handler *>(it->second);
		if (sensor == NULL)
			continue;

		/* it doesn't need to deregister handlers, they are consumed in event_loop */
		register_handler(sensor);
	}
}

void sensor_manager::register_handler(physical_sensor_handler *sensor)
{
	ret_if(sensor->get_poll_fd() < 0);

	sensor_event_handler *handler = new(std::nothrow) sensor_event_handler(sensor);
	retm_if(!handler, "Failed to allocate memory");

	m_loop->add_event(sensor->get_poll_fd(),
			ipc::EVENT_IN | ipc::EVENT_HUP | ipc::EVENT_NVAL, handler);
}

void sensor_manager::show(void)
{
	int index = 0;

	_I("========== Loaded sensor information ==========\n");
	for (auto it = m_sensors.begin(); it != m_sensors.end(); ++it) {
		sensor_info info = it->second->get_sensor_info();

		_I("Sensor #%d[%s]: ", ++index, it->first.c_str());
		info.show();
	}
	_I("===============================================\n");
}
