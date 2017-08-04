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

#ifndef __PEDOMETER_H__
#define __PEDOMETER_H__

#include "common.h"
#include "step_detection.h"
#include "pedometer_info.h"
#include "pedometer_speed_filter.h"

/************************************************************************
 * stores pedometer engine state.
 */
class pedometer {
public:
	pedometer();
	~pedometer();

	/************************************************************************
	 * enables/disables savitzky filter.
	 */
	void set_savitzky_filter(bool enable);

	/************************************************************************
	 * resets {@link pedometer} object to initial state.
	 */
	void reset(void);

	/************************************************************************
	 * called on new acceleration event.
	 *
	 * @param info
	 *            result of pedometer algorithm. valid only it method returns true.
	 * @param timestamp
	 *            timestamp of acceleration event in ns.
	 * @param acc
	 *            vertical component of global acceleration.
	 *
	 * @result
	 *            true if new step event was detected.
	 */
	bool get_pedometer(timestamp_t timestamp, double acc, pedometer_info *info);

private:
	/** detects step and estimates step length. */
	step_detection m_step_detection;

	/** sum of lengths all steps from start. */
	double m_total_length;

	/** number of steps from start. */
	long long m_step_count;

	/** estimates current speed from step length. */
	pedometer_speed_filter m_pedometer_filter;

	/** some non zero speed was detected. */
	bool m_some_speed;
};

#endif /* __PEDOMETER_H__ */
