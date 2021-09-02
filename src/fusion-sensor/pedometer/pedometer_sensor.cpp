/*
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

#include "pedometer_sensor.h"

#include <sensor_log.h>
#include <sensor_types.h>
#include <cmath>

#define NAME_SENSOR "http://samsung.com/sensor/healthinfo/pedometer/samsung_pedometer"
#define NAME_VENDOR "samsung.com"

#define SRC_ID_ACC   0x1
#define SRC_STR_ACC  "http://tizen.org/sensor/general/accelerometer"

#define US_TO_NS(x) (x * 1000)

/* Sensor information */
static sensor_info2_t sensor_info = {
	id: 0x1,
	type: HUMAN_PEDOMETER_SENSOR,
	uri: NAME_SENSOR,
	vendor: NAME_VENDOR,
	min_range: 0,
	max_range: 1,
	resolution: 1,
	min_interval: 0,
	max_batch_count: 0,
	wakeup_supported: false,
	privilege: "http://tizen.org/privilege/healthinfo",
};

/* Required sensor list */
static required_sensor_s required_sensors[] = {
	{SRC_ID_ACC, SRC_STR_ACC}
};

pedometer_sensor::pedometer_sensor()
: m_step_count(-1)
, m_step_length(-1)
, m_step_total_length(-1)
, m_step_speed(-1)
, m_time(0)
{
}

pedometer_sensor::~pedometer_sensor()
{
}

int pedometer_sensor::get_sensor_info(const sensor_info2_t **info)
{
	*info = &sensor_info;
	return OP_SUCCESS;
}

int pedometer_sensor::get_required_sensors(const required_sensor_s **sensors)
{
	*sensors = required_sensors;

	/* You should return the number of required sensor */
	return 1;
}

int pedometer_sensor::update(uint32_t id, sensor_data_t *data, int len)
{
	pedometer_info info;
	double acc[] = {data->values[0], data->values[1], data->values[2]};

	if (!m_pedometer.new_acceleration(&info, US_TO_NS(data->timestamp), acc))
		return OP_ERROR;

	m_step_count = info.step_count;
	m_step_length = info.step_length;
	m_step_total_length = info.total_step_length;
	m_step_speed = info.step_speed;
	m_time = data->timestamp;

	_D("[%lld] %lld %f %f %f", data->timestamp,
			info.step_count, info.step_length, info.total_step_length, info.step_speed);

	return OP_SUCCESS;
}

int pedometer_sensor::get_data(sensor_data_t **data, int *len)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	retvm_if(!sensor_data, -ENOMEM, "Failed to allocate memory");

	sensor_data->accuracy = SENSOR_ACCURACY_GOOD;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 8;
	sensor_data->values[0] = (float)m_step_count;
	sensor_data->values[1] = (float)m_step_count;
	sensor_data->values[2] = 0;
	sensor_data->values[3] = m_step_total_length;
	sensor_data->values[4] = 0;
	sensor_data->values[5] = m_step_speed;
	sensor_data->values[6] = 0;
	sensor_data->values[7] = 0;

	*data = sensor_data;
	*len = sizeof(sensor_data_t);

	return 0;
}

int pedometer_sensor::start(observer_h ob)
{
	m_pedometer.reset();
	return OP_DEFAULT;
}

int pedometer_sensor::stop(observer_h ob)
{
	return OP_DEFAULT;
}
