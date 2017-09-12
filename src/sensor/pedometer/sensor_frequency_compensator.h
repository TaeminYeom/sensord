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

#ifndef __SENSOR_FREQUENCY_COMPENSATOR_H_
#define __SENSOR_FREQUENCY_COMPENSATOR_H_

#include "common.h"

#include <memory>

/************************************************************************
 * Stores frequency compensator state.
 */
class sensor_frequency_compensator {
public:
	sensor_frequency_compensator(double desired_rate);
	~sensor_frequency_compensator();

	/************************************************************************
	 * Resets frequency compensator to initial state.
	 */
	void reset();

	/************************************************************************
	 * Filters input data.
	 *
	 * @param value
	 *              Data to filter.
	 * @result Filtered data.
	 */
	void add(timestamp_t t, double *value);

	/************************************************************************
	 */
	bool has_next();

	/************************************************************************
	 */
	void get_next(double *value);

private:
	long long m_desired_frequency;

	timestamp_t m_t1;

	double m_v1[3];

	timestamp_t m_t2;

	double m_v2[3];

	timestamp_t m_timestamp;
};

#endif /* __SENSOR_FREQUENCY_COMPENSATOR_H_ */
