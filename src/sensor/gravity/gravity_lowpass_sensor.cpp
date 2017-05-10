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

#include "gravity_lowpass_sensor.h"

#include <sensor_log.h>
#include <sensor_types.h>
#include <fusion_util.h>
#include <cmath>

#define NAME_SENSOR "http://tizen.org/sensor/general/gravity/tizen_lowpass"
#define NAME_VENDOR "tizen.org"

#define SRC_ID_ACC   0x1
#define SRC_STR_ACC  "http://tizen.org/sensor/general/accelerometer"

#define GRAVITY 9.80665
#define US_PER_SEC 1000000
#define TAU_LOW 0.4
#define TAU_MID 0.75
#define TAU_HIGH 0.99
#define NORM(x, y, z) sqrt((x)*(x) + (y)*(y) + (z)*(z))

static sensor_info2_t sensor_info = {
	id: 0x1,
	type: GRAVITY_SENSOR,
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
	{SRC_ID_ACC, SRC_STR_ACC}
};

gravity_lowpass_sensor::gravity_lowpass_sensor()
: m_x(-1)
, m_y(-1)
, m_z(-1)
, m_accuracy(-1)
, m_time(0)
{
	_I("Gravity Sensor is created!");
}

gravity_lowpass_sensor::~gravity_lowpass_sensor()
{
}

int gravity_lowpass_sensor::get_sensor_info(const sensor_info2_t **info)
{
	*info = &sensor_info;
	return OP_SUCCESS;
}

int gravity_lowpass_sensor::get_required_sensors(const required_sensor_s **sensors)
{
	*sensors = required_sensors;
	return 1;
}

int gravity_lowpass_sensor::update(uint32_t id, sensor_data_t *data, int len)
{
	float x, y, z, norm, alpha, tau, err;

	norm = NORM(data->values[0], data->values[1], data->values[2]);
	x = data->values[0] / norm * GRAVITY;
	y = data->values[1] / norm * GRAVITY;
	z = data->values[2] / norm * GRAVITY;

	if (m_time > 0) {
		err = fabs(norm - GRAVITY) / GRAVITY;
		tau = (err < 0.1 ? TAU_LOW : err > 0.9 ? TAU_HIGH : TAU_MID);
		alpha = tau / (tau + (float)(data->timestamp - m_time) / US_PER_SEC);
		x = alpha * m_x + (1 - alpha) * x;
		y = alpha * m_y + (1 - alpha) * y;
		z = alpha * m_z + (1 - alpha) * z;
		norm = NORM(x, y, z);
		x = x / norm * GRAVITY;
		y = y / norm * GRAVITY;
		z = z / norm * GRAVITY;
	}

	m_time = data->timestamp;
	m_accuracy = data->accuracy;
	m_x = x;
	m_y = y;
	m_z = z;

	return OP_SUCCESS;
}

int gravity_lowpass_sensor::get_data(sensor_data_t **data, int *len)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));

	sensor_data->accuracy = m_accuracy;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 3;
	sensor_data->values[0] = m_x;
	sensor_data->values[1] = m_y;
	sensor_data->values[2] = m_z;

	*data = sensor_data;
	*len = sizeof(sensor_data_t);

	return 0;
}
