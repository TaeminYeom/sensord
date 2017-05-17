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

#include "zero_crossing_step_detection.h"

static bool detect_zero_crossing(double last_accel, double new_accel, bool up)
{
	if (up)
		return (last_accel < 0 && new_accel >= 0);

	return (last_accel > 0 && new_accel <= 0);
}

zero_crossing_step_detection::zero_crossing_step_detection(bool up)
: m_time_sum(0)
, m_up(up)
, m_last_acceleration(0)
, m_last_timestamp(UNKNOWN_TIMESTAMP)
, m_last_zero_crossing_time(UNKNOWN_TIMESTAMP)
{
}

zero_crossing_step_detection::~zero_crossing_step_detection()
{
}

bool zero_crossing_step_detection::detect_step(timestamp_t timestamp, double acceleration)
{
	bool step_detected = false;

	if (m_last_timestamp != UNKNOWN_TIMESTAMP) {
		// zero crossing detected
		if (detect_zero_crossing(m_last_acceleration, acceleration, m_up)) {
			m_time_sum += timestamp - m_last_timestamp;
			m_last_timestamp = timestamp;
			m_last_zero_crossing_time = timestamp;
			step_detected = true;
		}
	} else {
		m_last_timestamp = timestamp;
	}
	m_last_acceleration = acceleration;
	return step_detected;
}

void zero_crossing_step_detection::reset(void)
{
	m_last_acceleration = 0;
	m_time_sum = 0;
	m_last_timestamp = UNKNOWN_TIMESTAMP;
	m_last_zero_crossing_time = UNKNOWN_TIMESTAMP;
}
