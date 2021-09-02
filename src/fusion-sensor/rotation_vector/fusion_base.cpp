/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sensor_log.h>
#include <cmath>
#include "fusion_base.h"

const float RAD2DEG = 57.29577951;
const float US2S = 1000000.0f;

fusion_base::fusion_base()
: m_enable_accel(false)
, m_enable_gyro(false)
, m_enable_magnetic(false)
, m_x(0)
, m_y(0)
, m_z(0)
, m_w(0)
, m_timestamp(0)
, m_timestamp_accel(0)
, m_timestamp_gyro(0)
, m_timestamp_mag(0)
{
}

fusion_base::~fusion_base()
{
}

void fusion_base::clear(void)
{
	m_enable_accel = false;
	m_enable_gyro = false;
	m_enable_magnetic = false;
}

void fusion_base::push_accel(sensor_data_t &data)
{
	//_I("[fusion_sensor] : Pushing accel");
	android::vec3_t v(data.values);

	float dT = (data.timestamp - m_timestamp_accel) / US2S;
	m_timestamp_accel = data.timestamp;
	m_timestamp = data.timestamp;

	m_enable_accel = true;
	m_orientation_filter.handleAcc(v, dT);
	store_orientation();
}

void fusion_base::push_gyro(sensor_data_t &data)
{
	//_I("[fusion_sensor] : Pushing gyro");
	android::vec3_t v(data.values);
	v[0] /= RAD2DEG;
	v[1] /= RAD2DEG;
	v[2] /= RAD2DEG;

	float dT = (data.timestamp - m_timestamp_gyro) / US2S;
	m_timestamp_gyro = data.timestamp;
	m_timestamp = data.timestamp;

	m_enable_gyro = true;
	m_orientation_filter.handleGyro(v, dT);
	store_orientation();
}

void fusion_base::push_mag(sensor_data_t &data)
{
	//_I("[fusion_sensor] : Pushing mag");
	android::vec3_t v(data.values);

	m_timestamp_mag = data.timestamp;
	m_timestamp = data.timestamp;

	m_enable_magnetic = true;
	m_orientation_filter.handleMag(v);
	store_orientation();
}

bool fusion_base::get_rv(unsigned long long &timestamp, float &x, float &y, float &z, float &w)
{
	if (m_timestamp == 0)
		return false;
	timestamp = m_timestamp;
	x = m_x;
	y = m_y;
	z = m_z;
	w = m_w;
	return true;
}

void fusion_base::store_orientation(void)
{
	android::quat_t q = m_orientation_filter.getAttitude();
	m_x = q[0];
	m_y = q[1];
	m_z = q[2];
	m_w = q[3];

	if (std::isnan(m_x) || std::isnan(m_y) || std::isnan(m_z) || std::isnan(m_w)) {
		m_timestamp = m_timestamp_accel = m_timestamp_gyro = m_timestamp_mag = 0;
		m_orientation_filter = android::orientation_filter();
	}
	clear();
}
