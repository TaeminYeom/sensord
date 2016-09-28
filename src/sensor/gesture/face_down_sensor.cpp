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
#include <face_down_sensor.h>
#include <sensor_loader.h>
#include <fusion_util.h>
#include <face_down_alg_impl.h>

#define SENSOR_NAME "SENSOR_FACE_DOWN"

#define SENSOR_FREQUENCY 50

face_down_sensor::face_down_sensor()
: m_gravity_sensor(NULL)
, m_time(0)
, m_state(false)
{
}

face_down_sensor::~face_down_sensor()
{
	_I("%s is destroyed!", SENSOR_NAME);
}

bool face_down_sensor::init(void)
{
	m_gravity_sensor = sensor_loader::get_instance().get_sensor(GRAVITY_SENSOR);

	if (!m_gravity_sensor) {
		_E("cannot load gravity sensor sensor[%s]", SENSOR_NAME);
		return false;
	}

	m_alg = get_alg();
	if (!m_alg)
		return false;

	_I("%s is created!", SENSOR_NAME);
	return true;
}

sensor_type_t face_down_sensor::get_type(void)
{
	return GESTURE_FACE_DOWN_SENSOR;
}

unsigned int face_down_sensor::get_event_type(void)
{
	return CONVERT_TYPE_EVENT(GESTURE_FACE_DOWN_SENSOR);
}

const char *face_down_sensor::get_name(void)
{
	return SENSOR_NAME;
}

bool face_down_sensor::get_sensor_info(sensor_info & info)
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

void face_down_sensor::synthesize(const sensor_event_t & event)
{
	if (event.event_type != GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME)
		return;

	m_time = event.data->timestamp;
	m_alg->push_event(event);
	m_state = m_alg->get_face_down();
	if (!m_state)
		return;

	sensor_event_t *face_down_event;
	sensor_data_t *face_down_data;
	int data_length;

	face_down_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
	if (!face_down_event) {
		_E("Failed to allocate memory");
		return;
	}

	get_data(&face_down_data, &data_length);
	face_down_event->sensor_id = get_id();
	face_down_event->event_type = FACE_DOWN_RAW_DATA_EVENT;
	face_down_event->data_length = data_length;
	face_down_event->data = face_down_data;

	push(face_down_event);

	_I("[face_down_sensor] : True");
}

int face_down_sensor::get_data(sensor_data_t ** data, int *length)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));

	sensor_data->accuracy = SENSOR_ACCURACY_GOOD;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 1;
	sensor_data->values[0] = m_state;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return 0;
}

bool face_down_sensor::set_interval(unsigned long interval)
{
	m_interval = interval;
	return true;
}

bool face_down_sensor::set_batch_latency(unsigned long latency)
{
	return false;
}

bool face_down_sensor::on_start(void)
{
	if (m_gravity_sensor)
		m_gravity_sensor->start();

	m_time = 0;
	m_state = false;
	return activate();
}

bool face_down_sensor::on_stop(void)
{
	if (m_gravity_sensor)
		m_gravity_sensor->stop();

	m_time = 0;
	m_state = false;

	return deactivate();
}

bool face_down_sensor::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	m_gravity_sensor->add_interval(client_id, interval, true);
	return sensor_base::add_interval(client_id, interval, is_processor);
}

bool face_down_sensor::delete_interval(int client_id, bool is_processor)
{
	m_gravity_sensor->delete_interval(client_id, true);
	return sensor_base::delete_interval(client_id, is_processor);
}

face_down_alg_impl *face_down_sensor::get_alg(void)
{
	face_down_alg_impl *alg = new(std::nothrow) face_down_alg_impl();
	retvm_if(!alg, NULL, "Failed to allocate memory");

	return alg;
}
