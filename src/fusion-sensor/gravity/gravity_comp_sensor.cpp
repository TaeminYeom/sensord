/*
 * sensord
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include "gravity_comp_sensor.h"

#include <sensor_log.h>
#include <sensor_types.h>
#include <cmath>

#define NAME_SENSOR "http://tizen.org/sensor/general/gravity/tizen_complementary"
#define NAME_VENDOR "tizen.org"

#define SRC_ID_ACC   0x1
#define SRC_STR_ACC  "http://tizen.org/sensor/general/accelerometer"

#define SRC_ID_GYRO  0x2
#define SRC_STR_GYRO "http://tizen.org/sensor/general/gyroscope"

#define GRAVITY 9.80665

#define PHASE_ACCEL_READY 0x01
#define PHASE_GYRO_READY 0x02
#define PHASE_FUSION_READY 0x03
#define US_PER_SEC 1000000
#define MS_PER_SEC 1000
#define INV_ANGLE -1000
#define TAU_LOW 0.4
#define TAU_MID 0.75
#define TAU_HIGH 0.99

#define DEG2RAD(x) ((x) * M_PI / 180.0)
#define NORM(x, y, z) sqrt((x)*(x) + (y)*(y) + (z)*(z))
#define ARCTAN(x, y) ((x) == 0 ? 0 : (y) != 0 ? atan2((x), (y)) : (x) > 0 ? M_PI/2.0 : -M_PI/2.0)

static sensor_info2_t sensor_info = {
	id: 0x1,
	type: GRAVITY_SENSOR,
	uri: NAME_SENSOR,
	vendor: NAME_VENDOR,
	min_range: -19.6,
	max_range: 19.6,
	resolution: 0.01,
	min_interval: 5,
	max_batch_count: 0,
	wakeup_supported: false,
	privilege:"",
};

static required_sensor_s required_sensors[] = {
	{SRC_ID_ACC,  SRC_STR_ACC},
	{SRC_ID_GYRO, SRC_STR_GYRO},
};

gravity_comp_sensor::gravity_comp_sensor()
: m_fusion_phase(0)
, m_x(-1)
, m_y(-1)
, m_z(-1)
, m_accuracy(-1)
, m_time(0)
, m_accel_mag(0)
, m_time_new(0)
{
	m_angle[0] = m_angle[1] = m_angle[2] = 0;
	m_velocity[0] = m_velocity[1] = m_velocity[2] = 0;
	m_angle_n[0] = m_angle_n[1] = m_angle_n[2] = 0;
}

gravity_comp_sensor::~gravity_comp_sensor()
{
}

int gravity_comp_sensor::get_sensor_info(const sensor_info2_t **info)
{
	*info = &sensor_info;
	return OP_SUCCESS;
}

int gravity_comp_sensor::get_required_sensors(const required_sensor_s **sensors)
{
	*sensors = required_sensors;
	return 2;
}

int gravity_comp_sensor::update(uint32_t id, sensor_data_t *data, int len)
{
	if (id == SRC_ID_ACC) {
		fusion_set_accel(data);
		m_fusion_phase |= PHASE_ACCEL_READY;
	} else if (id == SRC_ID_GYRO) {
		fusion_set_gyro(data);
		m_fusion_phase |= PHASE_GYRO_READY;
	}

	if (m_fusion_phase != PHASE_FUSION_READY)
		return OP_ERROR;

	m_fusion_phase = 0;

	fusion_update_angle();
	fusion_get_gravity();

	return OP_SUCCESS;
}

void gravity_comp_sensor::fusion_set_accel(sensor_data_t *data)
{
	double x = data->values[0];
	double y = data->values[1];
	double z = data->values[2];

	m_accel_mag = NORM(x, y, z);

	m_angle_n[0] = ARCTAN(z, y);
	m_angle_n[1] = ARCTAN(x, z);
	m_angle_n[2] = ARCTAN(y, x);

	m_accuracy = data->accuracy;
	m_time_new = data->timestamp;

	_D("AccIn: (%f, %f, %f)", x/m_accel_mag, y/m_accel_mag, z/m_accel_mag);
}

void gravity_comp_sensor::fusion_set_gyro(sensor_data_t *data)
{
	m_velocity[0] = -DEG2RAD(data->values[0]);
	m_velocity[1] = -DEG2RAD(data->values[1]);
	m_velocity[2] = -DEG2RAD(data->values[2]);

	m_time_new = data->timestamp;
}

void gravity_comp_sensor::fusion_update_angle(void)
{
	_D("AngleIn: (%f, %f, %f)", m_angle_n[0], m_angle_n[1], m_angle_n[2]);
	_D("AngAccl: (%f, %f, %f)", m_velocity[0], m_velocity[1], m_velocity[2]);
	_D("Angle  : (%f, %f, %f)", m_angle[0], m_angle[1], m_angle[2]);

	if (m_angle[0] == INV_ANGLE) {
		/* 1st iteration */
		m_angle[0] = m_angle_n[0];
		m_angle[1] = m_angle_n[1];
		m_angle[2] = m_angle_n[2];
	} else {
		complementary(m_time_new - m_time);
	}

	_D("Angle' : (%f, %f, %f)", m_angle[0], m_angle[1], m_angle[2]);
}

void gravity_comp_sensor::fusion_get_gravity(void)
{
	double x = 0, y = 0, z = 0;
	double norm;
	double vec[3][3];

	/* Rotating along y-axis then z-axis */
	vec[0][2] = cos(m_angle[1]);
	vec[0][0] = sin(m_angle[1]);
	vec[0][1] = vec[0][0] * tan(m_angle[2]);

	/* Rotating along z-axis then x-axis */
	vec[1][0] = cos(m_angle[2]);
	vec[1][1] = sin(m_angle[2]);
	vec[1][2] = vec[1][1] * tan(m_angle[0]);

	/* Rotating along x-axis then y-axis */
	vec[2][1] = cos(m_angle[0]);
	vec[2][2] = sin(m_angle[0]);
	vec[2][0] = vec[2][2] * tan(m_angle[1]);

	/* Normalize */
	for (int i = 0; i < 3; ++i) {
		norm = NORM(vec[i][0], vec[i][1], vec[i][2]);
		vec[i][0] /= norm;
		vec[i][1] /= norm;
		vec[i][2] /= norm;
		x += vec[i][0];
		y += vec[i][1];
		z += vec[i][2];
	}

	norm = NORM(x, y, z);

	m_x = x / norm * GRAVITY;
	m_y = y / norm * GRAVITY;
	m_z = z / norm * GRAVITY;
	m_time = m_time_new;
}

void gravity_comp_sensor::complementary(unsigned long long time_diff)
{
	double err = fabs(m_accel_mag - GRAVITY) / GRAVITY;
	double tau = (err < 0.1 ? TAU_LOW : err > 0.9 ? TAU_HIGH : TAU_MID);
	double delta_t = (double)time_diff/ US_PER_SEC;
	double alpha = tau / (tau + delta_t);

	_D("mag, err, tau, dt, alpha = %f, %f, %f, %f, %f", m_accel_mag, err, tau, delta_t, alpha);

	m_angle[0] = complementary(m_angle[0], m_angle_n[0], m_velocity[0], delta_t, alpha);
	m_angle[1] = complementary(m_angle[1], m_angle_n[1], m_velocity[1], delta_t, alpha);
	m_angle[2] = complementary(m_angle[2], m_angle_n[2], m_velocity[2], delta_t, alpha);
}

double gravity_comp_sensor::complementary(double angle, double angle_in, double vel, double delta_t, double alpha)
{
	double s, c;
	angle = angle + vel * delta_t;
	s = alpha * sin(angle) + (1 - alpha) * sin(angle_in);
	c = alpha * cos(angle) + (1 - alpha) * cos(angle_in);
	return ARCTAN(s, c);
}

int gravity_comp_sensor::get_data(sensor_data_t **data, int *len)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	retvm_if(!sensor_data, -ENOMEM, "Failed to allocate memory");

	sensor_data->accuracy = m_accuracy;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 3;
	sensor_data->values[0] = m_x;
	sensor_data->values[1] = m_y;
	sensor_data->values[2] = m_z;

	*data = sensor_data;
	*len = sizeof(sensor_data_t);

	return 0;
}
