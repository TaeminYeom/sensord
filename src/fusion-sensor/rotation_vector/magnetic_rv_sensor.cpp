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

#include "magnetic_rv_sensor.h"

#include <sensor_log.h>
#include <sensor_types.h>
#include <fusion_util.h>

#define NAME_SENSOR  "http://tizen.org/sensor/general/geomagnetic_rotation_vector/tizen_default"
#define NAME_VENDOR  "tizen.org"

#define SRC_ID_ACC   0x1
#define SRC_STR_ACC  "http://tizen.org/sensor/general/accelerometer"

#define SRC_ID_MAG   0x3
#define SRC_STR_MAG  "http://tizen.org/sensor/general/magnetic"

static sensor_info2_t sensor_info = {
	id: 0x1,
	type: GEOMAGNETIC_RV_SENSOR,
	uri: NAME_SENSOR,
	vendor: NAME_VENDOR,
	min_range: -1,
	max_range: 1,
	resolution: 1,
	min_interval: 10,
	max_batch_count: 0,
	wakeup_supported: false,
	privilege:"",
};

static required_sensor_s required_sensors[] = {
	{SRC_ID_ACC,     SRC_STR_ACC},
	{SRC_ID_MAG,     SRC_STR_MAG},
};

magnetic_rv_sensor::magnetic_rv_sensor()
: m_x(-1)
, m_y(-1)
, m_z(-1)
, m_w(-1)
, m_time(0)
, m_interval(100)
, m_accuracy(SENSOR_ACCURACY_UNDEFINED)
{
}

magnetic_rv_sensor::~magnetic_rv_sensor()
{
}

int magnetic_rv_sensor::get_sensor_info(const sensor_info2_t **info)
{
	*info = &sensor_info;
	return OP_SUCCESS;
}

int magnetic_rv_sensor::get_required_sensors(const required_sensor_s **sensors)
{
	*sensors = required_sensors;
	return 2;
}

int magnetic_rv_sensor::update(uint32_t id, sensor_data_t *data, int len)
{
	unsigned long long timestamp;

	if (id == SRC_ID_ACC)
		m_fusion.push_accel(*data);
	else if (id == SRC_ID_MAG)
		m_fusion.push_mag(*data);

	if (!m_fusion.get_rv(timestamp, m_w, m_x, m_y, m_z))
		return OP_ERROR;

	if (timestamp == m_time)
		return OP_ERROR;

	m_time = timestamp;
	m_accuracy = data->accuracy;

	_D("[rotation_vector] : [%10f] [%10f] [%10f] [%10f]", m_x, m_y, m_z, m_w);
	return OP_SUCCESS;
}

int magnetic_rv_sensor::get_data(sensor_data_t **data, int *length)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	retvm_if(!sensor_data, -ENOMEM, "Failed to allocate memory");

	sensor_data->accuracy = m_accuracy;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 4;
	sensor_data->values[0] = m_w;
	sensor_data->values[1] = m_x;
	sensor_data->values[2] = m_y;
	sensor_data->values[3] = m_z;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return 0;
}
