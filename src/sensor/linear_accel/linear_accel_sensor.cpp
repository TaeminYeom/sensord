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

#include "linear_accel_sensor.h"

#include <sensor_log.h>
#include <sensor_types.h>
#include <fusion_util.h>

#define NAME_SENSOR "http://tizen.org/sensor/general/linear_acceleration/tizen_default"
#define NAME_VENDOR "tizen.org"

#define SRC_ID_ACC   0x1
#define SRC_STR_ACC  "http://tizen.org/sensor/general/accelerometer"

#define SRC_ID_GRAVITY  0x2
#define SRC_STR_GRAVITY "http://tizen.org/sensor/general/gravity"

#define GRAVITY 9.80665

static sensor_info2_t sensor_info = {
	id: 0x1,
	type: LINEAR_ACCEL_SENSOR,
	uri: NAME_SENSOR,
	vendor: NAME_VENDOR,
	min_range: -19.6,
	max_range: 19.6,
	resolution: 0.01,
	min_interval: 5,
	max_batch_count: 0,
	wakeup_supported: false,
	privilege:"",
};

static required_sensor_s required_sensors[] = {
	{SRC_ID_ACC,     SRC_STR_ACC},
	{SRC_ID_GRAVITY, SRC_STR_GRAVITY},
};

linear_accel_sensor::linear_accel_sensor()
: m_x(0)
, m_y(0)
, m_z(0)
, m_gx(0)
, m_gy(0)
, m_gz(0)
, m_accuracy(0)
, m_time(0)
{
}

linear_accel_sensor::~linear_accel_sensor()
{
}

int linear_accel_sensor::get_sensor_info(const sensor_info2_t **info)
{
	*info = &sensor_info;
	return OP_SUCCESS;
}

int linear_accel_sensor::get_required_sensors(const required_sensor_s **sensors)
{
	*sensors = required_sensors;
	return 2;
}

int linear_accel_sensor::update(uint32_t id, sensor_data_t *data, int len)
{
	if (id == SRC_ID_GRAVITY) {
		m_gx = data->values[0];
		m_gy = data->values[1];
		m_gz = data->values[2];
	} else if (id == SRC_ID_ACC) {
		m_accuracy = data->accuracy;
		m_time = data->timestamp;
		m_x = data->values[0] - m_gx;
		m_y = data->values[1] - m_gy;
		m_z = data->values[2] - m_gz;

		return OP_SUCCESS;
	}

	return OP_ERROR; /* skip */
}

int linear_accel_sensor::get_data(sensor_data_t **data, int *length)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));

	sensor_data->accuracy = SENSOR_ACCURACY_GOOD;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 3;
	sensor_data->values[0] = m_x;
	sensor_data->values[1] = m_y;
	sensor_data->values[2] = m_z;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return 0;
}
