/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include "magnetic_orientation_sensor.h"

#include <sensor_log.h>
#include <sensor_types.h>
#include <fusion_util.h>

#define NAME_SENSOR "http://tizen.org/sensor/general/geomagnetic_orientation/tizen_default"
#define NAME_VENDOR "tizen.org"

#define SRC_ID_RV   0x1
#define SRC_STR_RV  "http://tizen.org/sensor/general/geomagnetic_rotation_vector"

static sensor_info2_t sensor_info = {
	id: 0x1,
	type: GEOMAGNETIC_ORIENTATION_SENSOR,
	uri: NAME_SENSOR,
	vendor: NAME_VENDOR,
	min_range: -180,
	max_range: 360,
	resolution: 0.01,
	min_interval: 10,
	max_batch_count: 0,
	wakeup_supported: false,
	privilege:"",
};

static required_sensor_s required_sensors[] = {
	{SRC_ID_RV,     SRC_STR_RV},
};


magnetic_orientation_sensor::magnetic_orientation_sensor()
: m_azimuth(-1)
, m_pitch(-1)
, m_roll(-1)
, m_accuracy(-1)
, m_time(0)
, m_interval(0)
{
}

magnetic_orientation_sensor::~magnetic_orientation_sensor()
{
}

int magnetic_orientation_sensor::get_sensor_info(const sensor_info2_t **info)
{
	*info = &sensor_info;
	return OP_SUCCESS;
}

int magnetic_orientation_sensor::get_required_sensors(const required_sensor_s **sensors)
{
	*sensors = required_sensors;
	return 1;
}

int magnetic_orientation_sensor::update(uint32_t id, sensor_data_t *data, int len)
{
	int error;
	float azimuth, pitch, roll;

	error = quat_to_orientation(data->values, azimuth, pitch, roll);
	retv_if(error, OP_ERROR);

	m_azimuth = azimuth;
	m_pitch = pitch;
	m_roll = roll;
	m_time = data->timestamp;
	m_accuracy = data->accuracy;

	//_D("[magnetic_orientation] : [%10f] [%10f] [%10f]", m_azimuth, m_pitch, m_roll);
	return OP_SUCCESS;
}

int magnetic_orientation_sensor::get_data(sensor_data_t **data, int *length)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	retvm_if(!sensor_data, -ENOMEM, "Failed to allocate memory");

	sensor_data->accuracy = m_accuracy;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 3;
	sensor_data->values[0] = m_azimuth;
	sensor_data->values[1] = m_pitch;
	sensor_data->values[2] = m_roll;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return 0;
}
