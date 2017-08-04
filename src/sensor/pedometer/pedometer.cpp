/*
 *  Copyright (c) 2016-2017 Samsung Electronics Co., Ltd.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "pedometer.h"
#include "step_event.h"

#include <math.h>

pedometer::pedometer()
: m_step_detection()
, m_total_length(0)
, m_step_count(0)
, m_pedometer_filter()
, m_some_speed(false)
{
}

pedometer::~pedometer()
{
}

void pedometer::set_savitzky_filter(bool enable)
{
	m_step_detection.set_use_savitzky(enable);
}

void pedometer::reset(void)
{
	m_total_length = 0;
	m_step_count = 0;
	m_step_detection.reset();
	m_pedometer_filter.reset();
	m_some_speed = false;
}

bool pedometer::get_pedometer(timestamp_t timestamp, double acc, pedometer_info *info)
{
	bool result = false;
	step_event event;

	if (m_step_detection.get_step(timestamp, acc, &event)) {
		if (event.m_timestamp != UNKNOWN_TIMESTAMP) {
			m_step_count++;
			m_total_length += event.m_step_length;
			m_pedometer_filter.get_step(timestamp, event.m_step_length);
			double speed = m_pedometer_filter.get_speed(timestamp);
			info->timestamp = timestamp;
			info->is_step_detected = true;
			info->step_count = m_step_count;
			info->step_length = event.m_step_length;
			info->total_step_length = m_total_length;
			info->step_speed = speed;
			result = true;
			m_some_speed = speed != 0;
		}
	}
	if (m_some_speed) {
		double speed = m_pedometer_filter.get_speed(timestamp);
		if (speed == 0) {
			m_some_speed = false;
			info->timestamp = timestamp;
			info->is_step_detected = true;
			info->step_count = m_step_count;
			info->step_length = 0;
			info->total_step_length = m_total_length;
			info->step_speed = 0;
			result = true;
		}
	}
	return result;
}
