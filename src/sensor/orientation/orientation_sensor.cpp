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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <dlfcn.h>

#include <sensor_log.h>
#include <sensor_types.h>

#include <sensor_common.h>
#include <virtual_sensor.h>
#include <orientation_sensor.h>
#include <sensor_loader.h>
#include <fusion_util.h>

#define SENSOR_NAME "SENSOR_ORIENTATION"

orientation_sensor::orientation_sensor()
: m_rotation_vector_sensor(NULL)
, m_azimuth(-1)
, m_pitch(-1)
, m_roll(-1)
, m_accuracy(-1)
, m_time(0)
{
}

orientation_sensor::~orientation_sensor()
{
	_I("%s is destroyed!", SENSOR_NAME);
}

bool orientation_sensor::init(void)
{
	m_rotation_vector_sensor = sensor_loader::get_instance().get_sensor(ROTATION_VECTOR_SENSOR);

	if (!m_rotation_vector_sensor) {
		_W("cannot load sensor[%s]", SENSOR_NAME);
		return false;
	}
	_I("%s is created!", SENSOR_NAME);
	return true;
}

sensor_type_t orientation_sensor::get_type(void)
{
	return ORIENTATION_SENSOR;
}

unsigned int orientation_sensor::get_event_type(void)
{
	return ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME;
}

const char* orientation_sensor::get_name(void)
{
	return SENSOR_NAME;
}

bool orientation_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC);
	info.set_name(get_name());
	info.set_vendor("Samsung Electronics");
	info.set_min_range(-180);
	info.set_max_range(360);
	info.set_resolution(0.01);
	info.set_min_interval(1);
	info.set_fifo_count(0);
	info.set_max_batch_count(0);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(false);

	return true;
}

void orientation_sensor::synthesize(const sensor_event_t& event)
{
	int error;
	sensor_event_t *orientation_event;
	float azimuth, pitch, roll;

	if (CONVERT_ID_TYPE(event.sensor_id) != ROTATION_VECTOR_SENSOR)
		return;

	error = quat_to_orientation(event.data->values, azimuth, pitch, roll);
	ret_if(error);

	orientation_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
	if (!orientation_event) {
		_E("Failed to allocate memory");
		return;
	}
	orientation_event->data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	if (!orientation_event->data) {
		_E("Failed to allocate memory");
		free(orientation_event);
		return;
	}

	orientation_event->sensor_id = get_id();
	orientation_event->event_type = CONVERT_TYPE_EVENT(ORIENTATION_SENSOR);
	orientation_event->data_length = sizeof(sensor_data_t);
	orientation_event->data->accuracy = event.data->accuracy;
	orientation_event->data->timestamp = event.data->timestamp;
	orientation_event->data->value_count = 3;
	orientation_event->data->values[0] = azimuth;
	orientation_event->data->values[1] = pitch;
	orientation_event->data->values[2] = roll;
	push(orientation_event);

	m_azimuth = azimuth;
	m_pitch = pitch;
	m_roll = roll;
	m_time = event.data->timestamp;
	m_accuracy = event.data->accuracy;

	_D("[orientation] : [%10f] [%10f] [%10f]", m_azimuth, m_pitch, m_roll);
}

int orientation_sensor::get_data(sensor_data_t **data, int *length)
{
	/* if It is batch sensor, remains can be 2+ */
	int remains = 1;

	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));

	sensor_data->accuracy = m_accuracy;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 3;
	sensor_data->values[0] = m_azimuth;
	sensor_data->values[1] = m_pitch;
	sensor_data->values[2] = m_roll;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return --remains;
}

bool orientation_sensor::set_interval(unsigned long interval)
{
	m_interval = interval;
	return true;
}

bool orientation_sensor::set_batch_latency(unsigned long latency)
{
	return false;
}

bool orientation_sensor::on_start(void)
{
	if (m_rotation_vector_sensor)
		m_rotation_vector_sensor->start();

	m_time = 0;

	return activate();
}

bool orientation_sensor::on_stop(void)
{
	if (m_rotation_vector_sensor)
		m_rotation_vector_sensor->stop();

	m_time = 0;

	return deactivate();
}

bool orientation_sensor::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	if (m_rotation_vector_sensor)
		m_rotation_vector_sensor->add_interval(client_id, interval, true);

	return sensor_base::add_interval(client_id, interval, is_processor);
}

bool orientation_sensor::delete_interval(int client_id, bool is_processor)
{
	if (m_rotation_vector_sensor)
		m_rotation_vector_sensor->delete_interval(client_id, true);

	return sensor_base::delete_interval(client_id, is_processor);
}
