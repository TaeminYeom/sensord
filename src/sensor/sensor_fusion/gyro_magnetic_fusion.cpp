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

#include "gyro_magnetic_fusion.h"

gyro_magnetic_fusion::gyro_magnetic_fusion()
{
}

gyro_magnetic_fusion::~gyro_magnetic_fusion()
{
}

bool gyro_magnetic_fusion::get_orientation(void)
{
	//_I("[fusion_sensor] : enable values are %d %d %d", m_enable_accel, m_enable_gyro, m_enable_magnetic);
	if (!m_enable_accel || !m_enable_gyro || !m_enable_magnetic)
		return false;

	m_orientation_filter.get_device_orientation(&m_accel, &m_gyro, &m_magnetic);
	m_timestamp = fmax(m_accel.m_time_stamp, m_gyro.m_time_stamp);
	m_timestamp = fmax(m_timestamp, m_magnetic.m_time_stamp);
	return true;
}
