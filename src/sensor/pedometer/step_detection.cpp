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

#include "step_detection.h"

#include <math.h>
#include <sensor_log.h>

/* Size of average filter. */
#define AV_FILTER_SIZE 7

#define AV_GFILTER_SIZE 500

#define FAST_PEAK_THRESHOLD 0.77

#define SLOW_PEAK_THRESHOLD 0.3

#define PEAK_THRESHOLD_FOR_MEAN 2.0

#define B_COEF 1.08

#define SLOPE_PARAM -0.75

step_detection::step_detection()
: m_average_filter(AV_FILTER_SIZE)
, m_average_gfilter(AV_GFILTER_SIZE)
, m_zero_crossing_up(true)
, m_zero_crossing_down(false)
, m_zc_filter()
, m_peak_threshold(FAST_PEAK_THRESHOLD)
, m_use_savitzky(false)
, m_last_step_timestamp(UNKNOWN_TIMESTAMP)
, m_zero_crossing_up_detected(false)
, m_zero_crossing_down_detected(false)
, m_minimum_acceleration(0)
, m_maximum_acceleration(0)
, m_is_slow_step_detected(false)
{
}

step_detection::~step_detection()
{
}

/************************************************************************
 */
void step_detection::reset(void)
{
	m_average_filter.reset();
	m_average_gfilter.reset();
	m_zero_crossing_up.reset();
	m_zero_crossing_down.reset();

	m_average_gfilter.filter(9.81);
	m_zc_filter.reset();

	m_zero_crossing_up_detected = false;
	m_zero_crossing_down_detected = false;
	m_minimum_acceleration = 0;
	m_maximum_acceleration = 0;
	m_is_slow_step_detected = false;
}

/************************************************************************
 */
void step_detection::set_peak_threshold(double threshold)
{
	m_peak_threshold = threshold;
}

/************************************************************************
 */
void step_detection::set_use_savitzky(bool use_savitzky)
{
	m_use_savitzky = use_savitzky;
}

/************************************************************************
 */
bool step_detection::is_slow_step(void)
{
	return m_is_slow_step_detected;
}

/************************************************************************
 */
static double cal_step_length(double time, double sqrt4peak_valley_diff)
{
	double step_length = 0;
	if (time <= 0 || time > 1.00) {
		step_length = 0.50 * sqrt4peak_valley_diff;
	} else if (time < 0.3) {
		step_length = 0;
	} else {
		step_length = B_COEF + SLOPE_PARAM * time;
	}

	if (step_length > 1.5)
		step_length = 0;

	return step_length;
}

/************************************************************************
 */
bool step_detection::add_acc_sensor_values_average(
		timestamp_t timestamp, double acc, step_event* step)
{
	double acceleration = m_average_filter.filter(acc - 9.8066f);

	bool n_zero_up = m_zero_crossing_up.detect_step(timestamp, acceleration);
	bool n_zero_down = m_zero_crossing_down.detect_step(timestamp, acceleration);

	double sqrt4peak_valley_diff = 0;
	bool is_step_detected = false;

	if (n_zero_up) {
		m_zero_crossing_up_detected = true;
		m_zero_crossing_down_detected = false;
	}
	if (n_zero_down) {
		m_zero_crossing_up_detected = false;
		m_zero_crossing_down_detected = true;
	}
	if (m_zero_crossing_up_detected) {
		if (m_maximum_acceleration < acceleration) {
			m_maximum_acceleration = acceleration;
		}
	}
	if (m_zero_crossing_down_detected) {
		if (m_minimum_acceleration > acceleration) {
			m_minimum_acceleration = acceleration;
		}
	}

	double peak_threshold;
	if (m_zero_crossing_up.m_time_sum / 1E9 < 1.2) {
		peak_threshold = m_peak_threshold;
		m_is_slow_step_detected = false;
	} else {
		peak_threshold = SLOW_PEAK_THRESHOLD;
		m_is_slow_step_detected = true;
	}

	if (n_zero_down) {
		if (m_maximum_acceleration > peak_threshold
				|| (m_maximum_acceleration - m_minimum_acceleration > PEAK_THRESHOLD_FOR_MEAN)) {
			sqrt4peak_valley_diff = pow(m_maximum_acceleration - m_minimum_acceleration, 0.25);
			m_minimum_acceleration = 0;
			m_maximum_acceleration = 0;
			is_step_detected = true;
		}
	}

	if (m_zero_crossing_up.m_time_sum / 1E9 < 0.3)
		is_step_detected = false;

	if (is_step_detected) {
		if (m_last_step_timestamp == UNKNOWN_TIMESTAMP)
			m_last_step_timestamp = timestamp;

		double time = (timestamp - m_last_step_timestamp) / 1E9;
		m_last_step_timestamp = timestamp;
		m_zero_crossing_up.m_time_sum = 0;
		step->m_timestamp = timestamp;
		step->m_step_length = cal_step_length(time, sqrt4peak_valley_diff);

		return true;
	}
	return false;
}

/************************************************************************
 */
bool step_detection::add_acc_sensor_values_savitzky(
		timestamp_t timestamp, double acc, step_event* step)
{
	double acceleration = m_zc_filter.filter(acc - m_average_gfilter.filter(acc));
	bool n_zero_up = m_zero_crossing_up.detect_step(timestamp, acceleration);
	bool n_zero_down = m_zero_crossing_down.detect_step(timestamp, acceleration);

	double sqrt4peak_valley_diff = 0;
	bool is_step_detected = false;

	if (n_zero_up) {
		m_zero_crossing_up_detected = true;
		m_zero_crossing_down_detected = false;
	}
	if (n_zero_down) {
		m_zero_crossing_up_detected = false;
		m_zero_crossing_down_detected = true;
	}
	if (m_zero_crossing_up_detected) {
		if (m_maximum_acceleration < acceleration) {
			m_maximum_acceleration = acceleration;
		}
	}
	if (m_zero_crossing_down_detected) {
		if (m_minimum_acceleration > acceleration) {
			m_minimum_acceleration = acceleration;
		}
	}

	bool zup = m_zero_crossing_up.m_time_sum / 1E9 > 1.2;
	if (n_zero_down) {
		is_step_detected = false;

		if ((m_maximum_acceleration > 0.6 &&
						m_minimum_acceleration < -0.852)
				|| (m_maximum_acceleration > 0.714
						&& m_minimum_acceleration < -0.455)) {
			is_step_detected = true;
			m_is_slow_step_detected = false;
		}
		if (m_maximum_acceleration - m_minimum_acceleration > 3.32) {
			is_step_detected = true;
			m_is_slow_step_detected = false;
		}
		if (zup && m_maximum_acceleration > 0.764 && m_minimum_acceleration < -0.0235) {// slow steps
			is_step_detected = true;
			m_is_slow_step_detected = true;
		}

		if (is_step_detected) {
			sqrt4peak_valley_diff = pow(m_maximum_acceleration - m_minimum_acceleration, 0.25);
			m_minimum_acceleration = 0.0;
			m_maximum_acceleration = 0.0;
		}
	}

	if (m_zero_crossing_up.m_time_sum / 1E9 < 0.3)
		is_step_detected = false;

	if (is_step_detected) {
		if (m_last_step_timestamp == UNKNOWN_TIMESTAMP)
			m_last_step_timestamp = timestamp;

		double time = (timestamp - m_last_step_timestamp) / 1E9;

		m_last_step_timestamp = timestamp;
		m_zero_crossing_up.m_time_sum = 0;
		step->m_timestamp = timestamp;
		step->m_step_length = cal_step_length(time, sqrt4peak_valley_diff);

		return true;
	}

	return false;
}

/************************************************************************
 */
bool step_detection::get_step(timestamp_t timestamp, double acc, step_event* step)
{
	return m_use_savitzky
			? add_acc_sensor_values_savitzky(timestamp, acc, step)
			: add_acc_sensor_values_average(timestamp, acc, step);
}
