/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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
#include <time.h>
#include <sensor_common.h>
#include <client_common.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sensor_types.h>
#include <string>
#include <map>

#define LOG_PER_COUNT_EVERY_EVENT 1
#define LOG_PER_COUNT_MAX 25

static std::map<sensor_id_t, unsigned int> sensor_log_count;

const char* get_sensor_name(sensor_id_t id)
{
	sensor_type_t type = (sensor_type_t) (id >> SENSOR_TYPE_SHIFT);

	return util_sensor_type_t::get_string(type);
}

const char* get_event_name(unsigned int event_type)
{
	sensor_type_t type = (sensor_type_t) (event_type >> SENSOR_EVENT_SHIFT);

	return util_sensor_type_t::get_string(type);
}

unsigned int get_calibration_event_type(unsigned int event_type)
{
	sensor_type_t type = (sensor_type_t)(event_type >> SENSOR_EVENT_SHIFT);

	switch (type) {
	case GEOMAGNETIC_SENSOR:
	case ROTATION_VECTOR_SENSOR:
	case RV_RAW_SENSOR:
	case ORIENTATION_SENSOR:
		return CALIBRATION_EVENT(type);
	default:
		return 0;
	}
}

unsigned int get_log_per_count(sensor_id_t id)
{
	sensor_type_t type = (sensor_type_t)(id >> SENSOR_TYPE_SHIFT);

	switch (type) {
	/* on_changed_event type sensors */
	case PROXIMITY_SENSOR:
	case GESTURE_WRIST_UP_SENSOR:
	case GESTURE_WRIST_DOWN_SENSOR:
	case GESTURE_MOVEMENT_SENSOR:
	case WEAR_STATUS_SENSOR:
		return LOG_PER_COUNT_EVERY_EVENT;
	default:
		break;
	}
	return LOG_PER_COUNT_MAX;
}

void print_event_occurrence_log(sensor_handle_info &info)
{
	unsigned int count;
	unsigned int log_per_count;

	auto it_count = sensor_log_count.find(info.m_sensor_id);
	if (it_count == sensor_log_count.end())
		sensor_log_count[info.m_sensor_id] = 0;

	count = ++sensor_log_count[info.m_sensor_id];
	log_per_count = get_log_per_count(info.m_sensor_id);

	if ((count != 1) && (count % log_per_count != 0))
		return;

	_D("%s receives %s[%d][state: %d, pause policy: %d, count: %d]", get_client_name(),
			get_sensor_name(info.m_sensor_id), info.m_handle,
			info.m_sensor_state, info.m_pause_policy, count);
}

/*
 *	To prevent user mistakenly freeing sensor_info using sensor_t
 */
static const int SENSOR_TO_SENSOR_INFO = 4;
static const int SENSOR_INFO_TO_SENSOR = -SENSOR_TO_SENSOR_INFO;

sensor_info *sensor_to_sensor_info(sensor_t sensor)
{
	if (!sensor)
		return NULL;

	sensor_info* info = (sensor_info *)((char *)sensor + SENSOR_TO_SENSOR_INFO);

	return info;
}

sensor_t sensor_info_to_sensor(const sensor_info *info)
{
	if (!info)
		return NULL;

	sensor_t sensor = (sensor_t)((char *)info + SENSOR_INFO_TO_SENSOR);

	return sensor;
}
