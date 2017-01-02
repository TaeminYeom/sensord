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

#include <sensor_common.h>
#include <sensor_log.h>
#include "pedometer_sensor.h"

enum value_index {
	IDX_STEPS = 0,
	IDX_WALK_STEPS,
	IDX_RUN_STEPS,
	IDX_DISTANCE,
	IDX_CALORIES,
	IDX_SPEED,
	IDX_FREQUENCY,
	IDX_STATE,
	IDX_WALK_UP,
	IDX_WALK_DOWN,
	IDX_RUN_UP,
	IDX_RUN_DOWN,
	IDX_STATE_EX,
};

pedometer_sensor::pedometer_sensor()
: m_steps(0)
, m_walk_steps(0)
, m_run_steps(0)
, m_walk_up_steps(0)
, m_walk_down_steps(0)
, m_run_up_steps(0)
, m_run_down_steps(0)
, m_distance(0)
, m_calories(0)
{
	set_permission(SENSOR_PERMISSION_BIO);

	_I("pedometer_sensor is created : %#x", this);
}

pedometer_sensor::~pedometer_sensor()
{
}

bool pedometer_sensor::on_event(const sensor_data_t *data, int data_len, int remains)
{
	if (data_len == sizeof(sensorhub_data_t))
		return false;

	accumulate((sensor_pedometer_data_t*)data);
	return true;
}

void pedometer_sensor::accumulate(sensor_pedometer_data_t *data)
{
	m_steps += data->values[IDX_STEPS];
	m_walk_steps += data->values[IDX_WALK_STEPS];
	m_run_steps += data->values[IDX_RUN_STEPS];
	m_distance += data->values[IDX_DISTANCE];
	m_calories += data->values[IDX_CALORIES];

	m_walk_up_steps += data->values[IDX_WALK_UP];
	m_walk_down_steps += data->values[IDX_WALK_DOWN];
	m_run_up_steps += data->values[IDX_RUN_UP];
	m_run_down_steps += data->values[IDX_RUN_DOWN];

	data->values[IDX_STEPS] = m_steps;
	data->values[IDX_WALK_STEPS] = m_walk_steps;
	data->values[IDX_RUN_STEPS] = m_run_steps;
	data->values[IDX_DISTANCE] = m_distance;
	data->values[IDX_CALORIES] = m_calories;

	data->values[IDX_WALK_UP] = m_walk_up_steps;
	data->values[IDX_WALK_DOWN] = m_walk_down_steps;
	data->values[IDX_RUN_UP] = m_run_up_steps;
	data->values[IDX_RUN_DOWN] = m_run_down_steps;
}
