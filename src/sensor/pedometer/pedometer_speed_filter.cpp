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

#include "pedometer_speed_filter.h"

/** default for maximum step duration in [ns]. currently 2s. */
static const long long STEP_MAX_DURATION = 2000000000L;

pedometer_speed_filter::pedometer_speed_filter()
: m_last_timestamp(UNKNOWN_TIMESTAMP)
, m_current_speed(0)
, m_last_step_length(0)
, m_last_step_duration(0)
, m_step_max_duration(STEP_MAX_DURATION)
{
}

pedometer_speed_filter::~pedometer_speed_filter()
{
}

/************************************************************************
 * clears current speed, last step length and last step duration.
 */
void pedometer_speed_filter::clear_speed(void)
{
	m_current_speed = 0;
	m_last_step_length = 0;
	m_last_step_duration = 0;
}

/************************************************************************
 * sets new maximum step duration in [ns].
 * if there is no new speed during this time current speed is cleared.
 * 0 disables this feature.
 *
 * @param step_max_duration
 *            maximum step duration in [ns].
 *            0 to disable step duration checking.
 */
void pedometer_speed_filter::set_step_max_duration(long long step_max_duration)
{
	m_step_max_duration = step_max_duration;
}

/************************************************************************
 * called when new step detection event occurs.
 *
 * @param timestamp
 *            timestamp of step detection event in [ns].
 * @param steplength
 *            length of detected step in [m].
 */
void pedometer_speed_filter::new_step(timestamp_t timestamp, double step_length)
{
	if (m_last_timestamp == UNKNOWN_TIMESTAMP || timestamp == UNKNOWN_TIMESTAMP) {
		clear_speed();
	} else if (m_step_max_duration != 0 && timestamp - m_last_timestamp > m_step_max_duration) {
		clear_speed();
	} else if (m_last_timestamp < timestamp) {
		double step_duration = (timestamp - m_last_timestamp) / 1e9;
		m_current_speed =
			(step_length + m_last_step_length) / (step_duration + m_last_step_duration);
		m_last_step_length = step_length;
		m_last_step_duration = step_duration;
	} else {
		return;
	}
	m_last_timestamp = timestamp;
}

/************************************************************************
 * reports new speed.
 *
 * @param timestamp
 *            timestamp of speed event in [ns].
 * @param speed
 *            current speed in [m/s].
 */
void pedometer_speed_filter::new_speed(timestamp_t timestamp, double speed)
{
	if (m_last_timestamp == UNKNOWN_TIMESTAMP || timestamp == UNKNOWN_TIMESTAMP
			|| timestamp > m_last_timestamp) {
		m_last_timestamp = timestamp;
		m_current_speed = speed;
		m_last_step_length = 0;
		m_last_step_duration = 0;
	}
}

/************************************************************************
 * returns current speed.
 *
 * @param timestamp
 *            timestamp for which speed should be calculated.
 * @return speed for given timestamp in [m/s].
 */
double pedometer_speed_filter::get_speed(timestamp_t timestamp)
{
	if (m_step_max_duration != 0 && (m_last_timestamp == UNKNOWN_TIMESTAMP ||
			timestamp == UNKNOWN_TIMESTAMP ||
			timestamp - m_last_timestamp > m_step_max_duration)) {
		clear_speed();
	}
	return m_current_speed;
}

/************************************************************************
 * changes current speed.
 *
 * @param speed
 *            current speed in [m/s].
 */
void pedometer_speed_filter::set_current_speed(double speed)
{
	m_current_speed = speed;
}

/************************************************************************
 * @return estimated current speed in [m/s].
 */
double pedometer_speed_filter::get_current_speed(void)
{
	return m_current_speed;
}

/************************************************************************
 */
bool pedometer_speed_filter::is_known_timestamp(void)
{
	return m_last_timestamp != UNKNOWN_TIMESTAMP;
}

/************************************************************************
 */
timestamp_t pedometer_speed_filter::get_timestamp(void)
{
	return m_last_timestamp;
}

/************************************************************************
 * resets filter to initial state.
 */
void pedometer_speed_filter::reset(void)
{
	m_last_timestamp = UNKNOWN_TIMESTAMP;
	clear_speed();
}
