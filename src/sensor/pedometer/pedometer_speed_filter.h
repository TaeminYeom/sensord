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

#ifndef __PEDOMETER_SPEED_FILTER_H__
#define __PEDOMETER_SPEED_FILTER_H__

#include "common.h"

/************************************************************************
 * stores pedometer speed filter state.
 */
class pedometer_speed_filter {
public:
	pedometer_speed_filter();
	~pedometer_speed_filter();

	void clear_speed(void);

	/************************************************************************
	 * sets new maximum step duration in ns.
	 * if there is no new speed during this time current speed is cleared.
	 * 0 disables this feature.
	 *
	 * @param step_max_duration
	 *            maximum step duration in ns.
	 *            0 to disable step duration checking.
	 */
	void set_step_max_duration(long long step_max_duration);

	/************************************************************************
	 * called when new step detection event occurs.
	 *
	 * @param timestamp
	 *            timestamp of step detection event in ns.
	 * @param steplength
	 *            length of detected step in m.
	 */
	void get_step(timestamp_t timestamp, double step_length);

	/************************************************************************
	 * reports new speed.
	 *
	 * @param timestamp
	 *            timestamp of speed event.
	 * @param speed
	 *            current speed in m/s.
	 */
	void new_speed(timestamp_t timestamp, double speed);

	/************************************************************************
	 * returns current speed.
	 *
	 * @param timestamp
	 *            timestamp for which speed should be calculated.
	 * @return speed for given timestamp in m/s.
	 */
	double get_speed(timestamp_t timestamp);

	/************************************************************************
	 * changes current speed.
	 *
	 * @param speed
	 *            current speed in m/s.
	 */
	void set_current_speed(double speed);

	/************************************************************************
	 * @return estimated current speed in m/s.
	 */
	double get_current_speed(void);

	/************************************************************************
	 */
	bool is_known_timestamp(void);

	/************************************************************************
	 * @return timestamp of last step detection event in ns.
	 */
	timestamp_t get_timestamp(void);

	/************************************************************************
	 * resets filter to initial state.
	 */
	void reset(void);

private:
	/** timestamp of last step detection event in ns. */
	timestamp_t m_last_timestamp;

	/** estimated current speed in m/s. */
	double m_current_speed;

	/** length of last step in m. */
	double m_last_step_length;

	/** duration of last step in s. */
	double m_last_step_duration;

	/** maximum step duration in ns. 0 to disable step duration checking. */
	long long m_step_max_duration;
};

#endif /* __PEDOMETER_SPEED_FILTER_H__ */
