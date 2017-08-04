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

#ifndef __ZERO_CROSSING_STEP_DETECTION_H__
#define __ZERO_CROSSING_STEP_DETECTION_H__

#include "common.h"

/************************************************************************
 * zero crossing detection engine state.
 */
class zero_crossing_step_detection {
public:
	zero_crossing_step_detection(bool up);
	~zero_crossing_step_detection();

	/************************************************************************
	 */
	bool detect_step(timestamp_t timestamp, double acceleration);

	/************************************************************************
	 */
	void reset(void);

	timestamp_t m_time_sum;

private:
	/**
	 * for <code>true</code> detects zero up crossing, for <code>false</code> detects zero down crossing.
	 */
	bool m_up;

	/** acceleration in previous detect step. */
	double m_last_acceleration;

	/** timestamp of last acc event. unknown time if no one. */
	timestamp_t m_last_timestamp;

	/** timestamp of last detected zero crossing. unknown time if not yet detected. */
	timestamp_t m_last_zero_crossing_time;
};

#endif /* __ZERO_CROSSING_STEP_DETECTION_H__ */
