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

#include "pick_up_sensor.h"

#include <sensor_log.h>
#include <sensor_types.h>

#define NAME_SENSOR "http://tizen.org/sensor/general/gesture_pick_up/tizen_default"
#define NAME_VENDOR "tizen.org"

#define SRC_ID_MOVE 0x1
#define SRC_URI_MOVE "http://tizen.org/sensor/general/gesture_movement/SENSOR_GESTURE_MOVEMENT"

static sensor_info2_t sensor_info = {
	id: 0x1,
	type: GESTURE_PICK_UP_SENSOR,
	uri: NAME_SENSOR,
	vendor: NAME_VENDOR,
	min_range: 0,
	max_range: 1,
	resolution: 1,
	min_interval: 0,
	max_batch_count: 0,
	wakeup_supported: false,
	privilege:"",
};

static required_sensor_s required_sensors[] = {
	{SRC_ID_MOVE, SRC_URI_MOVE}
};

pick_up_sensor::pick_up_sensor()
: m_state(0)
, m_timestamp(0)
{
}

pick_up_sensor::~pick_up_sensor()
{
}

int pick_up_sensor::get_sensor_info(const sensor_info2_t **info)
{
	*info = &sensor_info;
	return OP_SUCCESS;
}

int pick_up_sensor::get_required_sensors(const required_sensor_s **sensors)
{
	*sensors = required_sensors;
	return 1;
}

int pick_up_sensor::update(uint32_t id, sensor_data_t *data, int len)
{
	int state = data->values[0];

	_D("State : %d", state);

	m_state = state;
	m_timestamp = data->timestamp;

	return OP_SUCCESS;
}

int pick_up_sensor::get_data(sensor_data_t **data, int *length)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	retvm_if(!sensor_data, -ENOMEM, "Failed to allocate memory");

	sensor_data->accuracy = SENSOR_ACCURACY_GOOD;
	sensor_data->timestamp = m_timestamp;
	sensor_data->values[0] = m_state;
	sensor_data->value_count = 1;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return 0;
}
