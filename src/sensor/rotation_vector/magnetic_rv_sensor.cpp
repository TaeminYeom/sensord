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

#include <sensor_log.h>
#include <sensor_types.h>

#include <sensor_common.h>
#include <virtual_sensor.h>
#include <magnetic_rv_sensor.h>
#include <sensor_loader.h>
#include <fusion_util.h>

#define SENSOR_NAME "SENSOR_MAGNETIC_ROTATION_VECTOR"


magnetic_rv_sensor::magnetic_rv_sensor()
: m_accel_sensor(NULL)
, m_mag_sensor(NULL)
, m_x(-1)
, m_y(-1)
, m_z(-1)
, m_w(-1)
, m_time(0)
, m_accuracy(SENSOR_ACCURACY_UNDEFINED)
{
}

magnetic_rv_sensor::~magnetic_rv_sensor()
{
	_I("%s is destroyed!", SENSOR_NAME);
}

bool magnetic_rv_sensor::init(void)
{
	m_accel_sensor = sensor_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);
	m_mag_sensor = sensor_loader::get_instance().get_sensor(GEOMAGNETIC_SENSOR);

	if (!m_accel_sensor || !m_mag_sensor) {
		_E("cannot load sensors[%s]", SENSOR_NAME);
		return false;
	}

	_I("%s is created!", SENSOR_NAME);
	return true;
}

sensor_type_t magnetic_rv_sensor::get_type(void)
{
	return GEOMAGNETIC_RV_SENSOR;
}

unsigned int magnetic_rv_sensor::get_event_type(void)
{
	return CONVERT_TYPE_EVENT(GEOMAGNETIC_RV_SENSOR);
}

const char* magnetic_rv_sensor::get_name(void)
{
	return SENSOR_NAME;
}

bool magnetic_rv_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC);
	info.set_name(get_name());
	info.set_vendor("Samsung Electronics");
	info.set_min_range(0);
	info.set_max_range(1);
	info.set_resolution(1);
	info.set_min_interval(1);
	info.set_fifo_count(0);
	info.set_max_batch_count(0);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(false);

	return true;
}

void magnetic_rv_sensor::synthesize(const sensor_event_t& event)
{
	sensor_event_t *rotation_vector_event;

	if (event.event_type != ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME &&
		event.event_type != GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME)
		return;

	if (event.event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME)
		m_fusion.push_accel(*(event.data));
	else if (event.event_type == GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME)
		m_fusion.push_mag(*(event.data));

	if (m_accuracy == SENSOR_ACCURACY_UNDEFINED)
		m_accuracy = event.data->accuracy;
	else if (m_accuracy > event.data->accuracy)
		m_accuracy = event.data->accuracy;

	unsigned long long timestamp;
	if (!m_fusion.get_rv(timestamp, m_w, m_x, m_y, m_z))
		return;

	if (timestamp == m_time)
		return;
	m_time = timestamp;

	rotation_vector_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
	if (!rotation_vector_event) {
		_E("Failed to allocate memory");
		return;
	}
	rotation_vector_event->data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	if (!rotation_vector_event->data) {
		_E("Failed to allocate memory");
		free(rotation_vector_event);
		return;
	}

	rotation_vector_event->sensor_id = get_id();
	rotation_vector_event->event_type = CONVERT_TYPE_EVENT(GEOMAGNETIC_RV_SENSOR);
	rotation_vector_event->data_length = sizeof(sensor_data_t);
	rotation_vector_event->data->accuracy = m_accuracy;
	rotation_vector_event->data->timestamp = m_time;
	rotation_vector_event->data->value_count = 4;
	rotation_vector_event->data->values[0] = m_w;
	rotation_vector_event->data->values[1] = m_x;
	rotation_vector_event->data->values[2] = m_y;
	rotation_vector_event->data->values[3] = m_z;
	push(rotation_vector_event);
	m_accuracy = SENSOR_ACCURACY_UNDEFINED;

	_D("[rotation_vector] : [%10f] [%10f] [%10f] [%10f]", m_x, m_y, m_z, m_w);
}

int magnetic_rv_sensor::get_data(sensor_data_t **data, int *length)
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
	*length = sizeof(sensor_data_t);

	return 0;
}

bool magnetic_rv_sensor::set_interval(unsigned long interval)
{
	m_interval = interval;
	return true;
}

bool magnetic_rv_sensor::set_batch_latency(unsigned long latency)
{
	return false;
}

bool magnetic_rv_sensor::on_start(void)
{
	m_accel_sensor->start();
	m_mag_sensor->start();
	m_time = 0;
	m_accuracy = SENSOR_ACCURACY_UNDEFINED;
	return activate();
}

bool magnetic_rv_sensor::on_stop(void)
{
	m_accel_sensor->stop();
	m_mag_sensor->stop();
	m_time = 0;
	m_accuracy = SENSOR_ACCURACY_UNDEFINED;
	return deactivate();
}

bool magnetic_rv_sensor::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	m_accel_sensor->add_interval(client_id, interval, true);
	m_mag_sensor->add_interval(client_id, interval, true);

	return sensor_base::add_interval(client_id, interval, is_processor);
}

bool magnetic_rv_sensor::delete_interval(int client_id, bool is_processor)
{
	m_accel_sensor->delete_interval(client_id, true);
	m_mag_sensor->delete_interval(client_id, true);

	return sensor_base::delete_interval(client_id, is_processor);
}
