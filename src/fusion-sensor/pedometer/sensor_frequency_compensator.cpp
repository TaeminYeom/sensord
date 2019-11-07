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

#include "sensor_frequency_compensator.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

/************************************************************************
 */
sensor_frequency_compensator::sensor_frequency_compensator(double desired_rate)
: m_desired_rate(desired_rate)
, m_t1(UNKNOWN_TIMESTAMP)
, m_v1{0.0, 0.0, 0.0}
, m_t2(UNKNOWN_TIMESTAMP)
, m_v2{0.0, 0.0, 0.0}
, m_timestamp(UNKNOWN_TIMESTAMP)
{
}

/************************************************************************
 */
sensor_frequency_compensator::~sensor_frequency_compensator()
{
}

/************************************************************************
 */
void sensor_frequency_compensator::reset() {
	m_t1 = UNKNOWN_TIMESTAMP;
	m_t2 = UNKNOWN_TIMESTAMP;
	m_timestamp = UNKNOWN_TIMESTAMP;
}

/************************************************************************
 */
void sensor_frequency_compensator::add(timestamp_t t, double *v) {
	if (m_timestamp == UNKNOWN_TIMESTAMP) {
		m_timestamp = t;
	}
	m_t1 = m_t2;
	memcpy(m_v1, m_v2, sizeof(m_v1));
	m_t2 = t;
	memcpy(m_v2, v, sizeof(m_v2));
}

/************************************************************************
 */
bool sensor_frequency_compensator::has_next() {
	if (m_t1 == UNKNOWN_TIMESTAMP || m_t2 == UNKNOWN_TIMESTAMP) {
		return false;
	}
	return m_timestamp + m_desired_rate < m_t2;
}

/************************************************************************
 */
void sensor_frequency_compensator::get_next(double *v) {
	timestamp_t t3 = m_timestamp;
	if (t3 < m_t1) {
		t3 = m_t1;
	}
	m_timestamp += m_desired_rate;
	double t = ((double) (t3 - m_t1)) / (m_t2 - m_t1);
	for (int i = 0; i < 3; i++) {
		v[i] = (1.0 - t) * m_v1[i] + t * m_v2[i];
	}
}
