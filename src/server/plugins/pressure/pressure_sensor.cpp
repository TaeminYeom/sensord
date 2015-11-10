/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <sensor_logs.h>
#include <sf_common.h>
#include <pressure_sensor.h>
#include <sensor_plugin_loader.h>

using std::bind1st;
using std::mem_fun;
using std::string;
using std::vector;

#define SENSOR_NAME "PRESSURE_SENSOR"
#define SENSOR_TYPE_PRESSURE		"PRESSURE"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ATTR_VALUE				"value"

pressure_sensor::pressure_sensor()
: m_sensor_hal(NULL)
, m_resolution(0.0f)
{
	m_name = string(SENSOR_NAME);

	register_supported_event(PRESSURE_RAW_DATA_EVENT);

	physical_sensor::set_poller(pressure_sensor::working, this);
}

pressure_sensor::~pressure_sensor()
{
	INFO("pressure_sensor is destroyed!");
}

bool pressure_sensor::init()
{
	m_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(SENSOR_HAL_TYPE_PRESSURE);

	if (!m_sensor_hal) {
		ERR("cannot load sensor_hal[%s]", sensor_base::get_name());
		return false;
	}

	sensor_properties_s properties;

	if (!m_sensor_hal->get_properties(properties)) {
		ERR("sensor->get_properties() is failed!\n");
		return false;
	}

	m_resolution = properties.resolution;

	string model_id = m_sensor_hal->get_model_id();

	INFO("%s is created!", sensor_base::get_name());

	return true;
}

void pressure_sensor::get_types(vector<sensor_type_t> &types)
{
	types.push_back(PRESSURE_SENSOR);
}

bool pressure_sensor::working(void *inst)
{
	pressure_sensor *sensor = (pressure_sensor*)inst;
	return sensor->process_event();
}

bool pressure_sensor::process_event(void)
{
	sensor_event_t event;

	if (!m_sensor_hal->is_data_ready())
		return true;

	m_sensor_hal->get_sensor_data(event.data);

	AUTOLOCK(m_client_info_mutex);

	if (get_client_cnt(PRESSURE_RAW_DATA_EVENT)) {
		event.sensor_id = get_id();
		event.event_type = PRESSURE_RAW_DATA_EVENT;
		push(event);
	}

	return true;
}

bool pressure_sensor::on_start(void)
{
	if (!m_sensor_hal->enable()) {
		ERR("m_sensor_hal start fail\n");
		return false;
	}

	return start_poll();
}

bool pressure_sensor::on_stop(void)
{
	if (!m_sensor_hal->disable()) {
		ERR("m_sensor_hal stop fail\n");
		return false;
	}

	return stop_poll();
}

bool pressure_sensor::get_properties(sensor_type_t sensor_type, sensor_properties_s &properties)
{
	return m_sensor_hal->get_properties(properties);
}

int pressure_sensor::get_sensor_data(unsigned int type, sensor_data_t &data)
{
	return m_sensor_hal->get_sensor_data(data);
}

bool pressure_sensor::set_interval(unsigned long interval)
{
	AUTOLOCK(m_mutex);

	INFO("Polling interval is set to %dms", interval);

	return m_sensor_hal->set_interval(interval);
}
