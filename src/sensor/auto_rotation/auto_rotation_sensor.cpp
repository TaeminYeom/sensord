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

#include "auto_rotation_sensor.h"

#include <sensor_log.h>
#include <sensor_types.h>

#include "auto_rotation_alg_emul.h"

#define NAME_SENSOR "http://tizen.org/sensor/general/auto_rotation/tizen_default"
#define NAME_VENDOR "tizen.org"

#define SRC_ID_ACC  0x1
#define SRC_STR_ACC "http://tizen.org/sensor/general/accelerometer"

static sensor_info2_t sensor_info = {
	id: 0x1,
	type: AUTO_ROTATION_SENSOR,
	uri: NAME_SENSOR,
	vendor: NAME_VENDOR,
	min_range: AUTO_ROTATION_DEGREE_UNKNOWN,
	max_range: AUTO_ROTATION_DEGREE_270,
	resolution: 1,
	min_interval: 60,
	max_batch_count: 0,
	wakeup_supported: false,
	privilege:"",
};

static required_sensor_s required_sensors[] = {
	{SRC_ID_ACC, SRC_STR_ACC}
};

auto_rotation_sensor::auto_rotation_sensor()
: m_rotation(0)
, m_rotation_time(0)
, m_alg(NULL)
{
	if (!init())
		throw OP_ERROR;
}

auto_rotation_sensor::~auto_rotation_sensor()
{
	deinit();
}

bool auto_rotation_sensor::init(void)
{
	m_alg = get_alg();
	retvm_if(!m_alg, false, "Not supported");
	retvm_if(!m_alg->open(), false, "Cannot open auto rotation algorithm");
	return true;
}

void auto_rotation_sensor::deinit(void)
{
	delete m_alg;
}

int auto_rotation_sensor::get_sensor_info(const sensor_info2_t **info)
{
	*info = &sensor_info;
	return OP_SUCCESS;
}

int auto_rotation_sensor::get_required_sensors(const required_sensor_s **sensors)
{
	*sensors = required_sensors;
	return 1;
}

int auto_rotation_sensor::update(uint32_t id, sensor_data_t *data, int len)
{
	int rotation;
	float acc[3];
	acc[0] = data->values[0];
	acc[1] = data->values[1];
	acc[2] = data->values[2];

	if (!m_alg->get_rotation(acc, data->timestamp, m_rotation, rotation))
		return OP_ERROR;

	_D("Rotation: %d, ACC[0]: %f, ACC[1]: %f, ACC[2]: %f",
		rotation, data->values[0], data->values[1], data->values[2]);

	m_rotation = rotation;
	m_rotation_time = data->timestamp;

	return OP_SUCCESS;
}

int auto_rotation_sensor::get_data(sensor_data_t **data, int *length)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	retvm_if(!sensor_data, -ENOMEM, "Failed to allocate memory");

	sensor_data->accuracy = SENSOR_ACCURACY_GOOD;
	sensor_data->timestamp = m_rotation_time;
	sensor_data->values[0] = m_rotation;
	sensor_data->value_count = 1;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return 1;
}

int auto_rotation_sensor::start(observer_h ob)
{
	m_rotation = AUTO_ROTATION_DEGREE_UNKNOWN;

	/* TODO: cache */

	m_alg->start();

	/* if OP_DEFAULT is returned,
	 * this function is not called anymore before stop() is called */
	return OP_DEFAULT;
}

int auto_rotation_sensor::stop(observer_h ob)
{
	m_alg->stop();

	/* if OP_DEFAULT is returned,
	 * this function is not called anymore before start() is called */
	return OP_DEFAULT;
}

int auto_rotation_sensor::set_interval(observer_h ob, int32_t &interval)
{
	/* Fix internal */
	interval = 60;
	return OP_SUCCESS;
}

auto_rotation_alg *auto_rotation_sensor::get_alg(void)
{
	auto_rotation_alg *alg = new(std::nothrow) auto_rotation_alg_emul();
	retvm_if(!alg, NULL, "Failed to allocate memory");

	return alg;
}
