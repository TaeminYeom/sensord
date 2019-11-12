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

#ifndef __STEP_DETECTION_H__
#define __STEP_DETECTION_H__

#include "average_filter.h"
#include "zero_crossing_step_detection.h"
#include "savitzky_golay_filter15.h"
#include "step_event.h"
#include "timestamp.h"

/************************************************************************
 * step detection engine state.
 */
class step_detection {
public:

	step_detection();

	~step_detection();

	/************************************************************************
	 */
	void set_peak_threshold(double threshold);

	/************************************************************************
	 */
	void set_use_savitzky(bool use_savitzky);

	/************************************************************************
	 */
	bool new_acceleration(timestamp_t timestamp, double acc, step_event* step);

	/************************************************************************
	 * resets step_detection object to initial state.
	 */
	void reset(void);

private:
	average_filter m_average_filter;
	average_filter m_average_gfilter;
	zero_crossing_step_detection m_zero_crossing_up;
	zero_crossing_step_detection m_zero_crossing_down;

	savitzky_golay_filter15 m_zc_filter;
	double m_peak_threshold;
	bool m_use_savitzky;
	timestamp_t m_last_step_timestamp;
	bool m_zero_crossing_up_detected;
	bool m_zero_crossing_down_detected;
	double m_minimum_acceleration;
	double m_maximum_acceleration;
	bool m_is_slow_step_detected;

	/************************************************************************
	 */
	bool add_acc_sensor_values_average(
			timestamp_t timestamp, double acc, step_event* step);

	/************************************************************************
	 */
	bool add_acc_sensor_values_savitzky(
			timestamp_t timestamp, double acc, step_event* step);

	/************************************************************************
	 */
	bool is_slow_step(void);
};

#endif /* __STEP_DETECTION_H__ */
