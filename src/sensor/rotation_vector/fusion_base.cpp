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
#include <sensor_loader.h>
#include <sensor_base.h>
#include <cmath>
#include "fusion_base.h"

#define ACCEL_COMPENSATION -1
#define GYRO_COMPENSATION 1
#define MAG_COMPENSATION -1

fusion_base::fusion_base()
: m_enable_accel(false)
, m_enable_gyro(false)
, m_enable_magnetic(false)
{
	_I("fusion_base is created!");
}

fusion_base::~fusion_base()
{
	_I("fusion_sensor is destroyed!");
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
	pre_process_data(m_accel, data.values, ACCEL_COMPENSATION, ACCEL_SCALE);
	m_accel.m_time_stamp = data.timestamp;
	m_enable_accel = true;
	if (get_orientation())
		store_orientation();
}

void fusion_base::push_gyro(sensor_data_t &data)
{
	//_I("[fusion_sensor] : Pushing mag");
	pre_process_data(m_gyro, data.values, GYRO_COMPENSATION, GYRO_SCALE);
	m_gyro.m_time_stamp = data.timestamp;
	m_enable_gyro = true;
	if (get_orientation())
		store_orientation();
}

void fusion_base::push_mag(sensor_data_t &data)
{
	//_I("[fusion_sensor] : Pushing gyro");
	pre_process_data(m_magnetic, data.values, MAG_COMPENSATION, GEOMAGNETIC_SCALE);
	m_magnetic.m_time_stamp = data.timestamp;
	m_enable_magnetic = true;
	if (get_orientation())
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
	m_x = m_orientation_filter.m_quaternion.m_quat.m_vec[0];
	m_y = m_orientation_filter.m_quaternion.m_quat.m_vec[1];
	m_z = m_orientation_filter.m_quaternion.m_quat.m_vec[2];
	m_w = m_orientation_filter.m_quaternion.m_quat.m_vec[3];
	clear();
}
