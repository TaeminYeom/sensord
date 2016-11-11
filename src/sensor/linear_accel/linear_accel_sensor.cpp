/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <linear_accel_sensor.h>
#include <sensor_loader.h>
#include <fusion_util.h>

#define SENSOR_NAME "SENSOR_LINEAR_ACCELERATION"

#define GRAVITY 9.80665

linear_accel_sensor::linear_accel_sensor()
: m_accel_sensor(NULL)
, m_gravity_sensor(NULL)
, m_x(0)
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
	_I("linear_accel_sensor is destroyed!\n");
}

bool linear_accel_sensor::init(void)
{
	m_accel_sensor = sensor_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);

	if (!m_accel_sensor) {
		_W("cannot load accelerometer sensor_hal[%s]", get_name());
		return false;
	}

	m_gravity_sensor = sensor_loader::get_instance().get_sensor(GRAVITY_SENSOR);

	if (!m_gravity_sensor) {
		_W("cannot load gravity sensor_hal[%s]", get_name());
		return false;
	}

	_I("%s is created!\n", get_name());

	return true;
}

sensor_type_t linear_accel_sensor::get_type(void)
{
	return LINEAR_ACCEL_SENSOR;
}

unsigned int linear_accel_sensor::get_event_type(void)
{
	return LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME;
}

const char* linear_accel_sensor::get_name(void)
{
	return SENSOR_NAME;
}

bool linear_accel_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC); // FIXME
	info.set_name("Linear Accelerometer Sensor");
	info.set_vendor("Samsung Electronics");
	info.set_min_range(-19.6);
	info.set_max_range(19.6);
	info.set_resolution(0.01);
	info.set_min_interval(1);
	info.set_fifo_count(0);
	info.set_max_batch_count(0);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(false);

	return true;
}

void linear_accel_sensor::synthesize(const sensor_event_t& event)
{
	if (event.event_type == GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME) {
		m_gx = event.data->values[0];
		m_gy = event.data->values[1];
		m_gz = event.data->values[2];
		return;
	}

	if (event.event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME) {
		m_time = event.data->timestamp;
		m_x = event.data->values[0] - m_gx;
		m_y = event.data->values[1] - m_gy;
		m_z = event.data->values[2] - m_gz;

		sensor_event_t *linear_accel_event;
		sensor_data_t *linear_accel_data;
		int data_length;
		int remains;

		linear_accel_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
		if (!linear_accel_event) {
			_E("Failed to allocate memory");
			return;
		}

		remains = get_data(&linear_accel_data, &data_length);

		if (remains < 0) {
			free(linear_accel_event);
			return;
		}

		linear_accel_event->sensor_id = get_id();
		linear_accel_event->event_type = LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME;
		linear_accel_event->data_length = data_length;
		linear_accel_event->data = linear_accel_data;

		push(linear_accel_event);
	}
}

int linear_accel_sensor::get_data(sensor_data_t **data, int *length)
{
	/* if It is batch sensor, remains can be 2+ */
	int remains = 1;

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

	return --remains;
}

bool linear_accel_sensor::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	if (m_accel_sensor)
		m_accel_sensor->add_interval(client_id, interval, true);

	if (m_gravity_sensor)
		m_gravity_sensor->add_interval(client_id, interval, true);

	return sensor_base::add_interval(client_id, interval, is_processor);
}

bool linear_accel_sensor::delete_interval(int client_id, bool is_processor)
{
	if (m_accel_sensor)
		m_accel_sensor->delete_interval(client_id, true);

	if (m_gravity_sensor)
		m_gravity_sensor->delete_interval(client_id, true);

	return sensor_base::delete_interval(client_id, is_processor);
}

bool linear_accel_sensor::set_interval(unsigned long interval)
{
	m_interval = interval;
	return true;
}

bool linear_accel_sensor::set_batch_latency(unsigned long latency)
{
	return false;
}

bool linear_accel_sensor::set_wakeup(int wakeup)
{
	return false;
}

bool linear_accel_sensor::on_start(void)
{
	if (m_accel_sensor)
		m_accel_sensor->start();

	if (m_gravity_sensor)
		m_gravity_sensor->start();

	m_time = 0;
	return activate();
}

bool linear_accel_sensor::on_stop(void)
{
	if (m_accel_sensor)
		m_accel_sensor->stop();

	if (m_gravity_sensor)
		m_gravity_sensor->stop();

	m_time = 0;
	return deactivate();
}
