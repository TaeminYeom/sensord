/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <dlfcn.h>

#include <sensor_log.h>
#include <sensor_types.h>

#include <sensor_common.h>
#include <virtual_sensor.h>
#include <gravity_sensor.h>
#include <sensor_loader.h>
#include <fusion_util.h>

#define SENSOR_NAME "SENSOR_GRAVITY"

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

gravity_sensor::gravity_sensor()
: m_fusion(NULL)
, m_accel_sensor(NULL)
, m_gyro_sensor(NULL)
, m_fusion_phase(0)
, m_x(-1)
, m_y(-1)
, m_z(-1)
, m_accuracy(-1)
, m_time(0)
{
}

gravity_sensor::~gravity_sensor()
{
	_I("gravity_sensor is destroyed!\n");
}

bool gravity_sensor::init(void)
{
	/* Acc (+ Gyro) fusion */
	m_accel_sensor = sensor_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);

	if (!m_accel_sensor) {
		_W("cannot load accelerometer sensor_hal[%s]", get_name());
		return false;
	}

	m_gyro_sensor = sensor_loader::get_instance().get_sensor(GYROSCOPE_SENSOR);

	_I("%s (%s) is created!\n", get_name(), m_gyro_sensor ? "Acc+Gyro Fusion" : "LowPass Acc");
	return true;
}

sensor_type_t gravity_sensor::get_type(void)
{
	return GRAVITY_SENSOR;
}

unsigned int gravity_sensor::get_event_type(void)
{
	return GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
}

const char* gravity_sensor::get_name(void)
{
	return SENSOR_NAME;
}

bool gravity_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC); // FIXME
	info.set_name("Gravity Sensor");
	info.set_vendor("Samsung Electronics");
	info.set_min_range(-19.6);
	info.set_max_range(19.6);
	info.set_resolution(0.01);
	info.set_min_interval(1);
	info.set_fifo_count(0);
	info.set_max_batch_count(0);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(false);

	return true;
}

void gravity_sensor::synthesize(const sensor_event_t& event)
{
	/* If the rotation vector sensor is available */
	if (m_fusion) {
		synthesize_rv(event);
		return;
	}

	/* If both Acc & Gyro are available */
	if (m_gyro_sensor) {
		synthesize_fusion(event);
		return;
	}

	/* If only Acc is available */
	synthesize_lowpass(event);
}

void gravity_sensor::synthesize_rv(const sensor_event_t& event)
{
	if (!m_fusion->is_data_ready())
		return;

	sensor_event_t *gravity_event;
	float gravity[3];
	float x, y, z, w, heading_accuracy;
	int accuracy;

	if (!m_fusion->get_rotation_vector(x, y, z, w, heading_accuracy, accuracy)) {
		_W("Failed to get rotation vector");
		return;
	}

	unsigned long long timestamp = m_fusion->get_data_timestamp();

	if (!check_sampling_time(timestamp))
		return;

	float rotation[4] = {x, y, z, w};

	if (!rotation_to_gravity(rotation, gravity)) {
		_D("Invalid rotation_vector: [%10f] [%10f] [%10f] [%10f]", x, y, z, w);
		return;
	}

	gravity_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
	if (!gravity_event) {
		_E("Failed to allocate memory");
		return;
	}
	gravity_event->data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	if (!gravity_event->data) {
		_E("Failed to allocate memory");
		free(gravity_event);
		return;
	}

	gravity_event->sensor_id = get_id();
	gravity_event->event_type = GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
	gravity_event->data_length = sizeof(sensor_data_t);
	gravity_event->data->accuracy = accuracy;
	gravity_event->data->timestamp = m_fusion->get_data_timestamp();
	gravity_event->data->value_count = 3;
	gravity_event->data->values[0] = gravity[0];
	gravity_event->data->values[1] = gravity[1];
	gravity_event->data->values[2] = gravity[2];
	push(gravity_event);

	m_time = event.data->timestamp;
	m_x = gravity[0];
	m_y = gravity[1];
	m_z = gravity[2];
	m_accuracy = accuracy;
}

void gravity_sensor::synthesize_lowpass(const sensor_event_t& event)
{
	if (event.event_type != ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME)
		return;

	sensor_event_t *gravity_event;
	float x, y, z, norm, alpha, tau, err;

	norm = NORM(event.data->values[0], event.data->values[1], event.data->values[2]);
	x = event.data->values[0] / norm * GRAVITY;
	y = event.data->values[1] / norm * GRAVITY;
	z = event.data->values[2] / norm * GRAVITY;

	if (m_time > 0) {
		err = fabs(norm - GRAVITY) / GRAVITY;
		tau = (err < 0.1 ? TAU_LOW : err > 0.9 ? TAU_HIGH : TAU_MID);
		alpha = tau / (tau + (float)(event.data->timestamp - m_time) / US_PER_SEC);
		x = alpha * m_x + (1 - alpha) * x;
		y = alpha * m_y + (1 - alpha) * y;
		z = alpha * m_z + (1 - alpha) * z;
		norm = NORM(x, y, z);
		x = x / norm * GRAVITY;
		y = y / norm * GRAVITY;
		z = z / norm * GRAVITY;
	}

	gravity_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
	if (!gravity_event) {
		_E("Failed to allocate memory");
		return;
	}
	gravity_event->data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	if (!gravity_event->data) {
		_E("Failed to allocate memory");
		free(gravity_event);
		return;
	}

	gravity_event->sensor_id = get_id();
	gravity_event->event_type = GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
	gravity_event->data_length = sizeof(sensor_data_t);
	gravity_event->data->accuracy = event.data->accuracy;
	gravity_event->data->timestamp = event.data->timestamp;
	gravity_event->data->value_count = 3;
	gravity_event->data->values[0] = x;
	gravity_event->data->values[1] = y;
	gravity_event->data->values[2] = z;
	push(gravity_event);

	m_time = event.data->timestamp;
	m_x = x;
	m_y = y;
	m_z = z;
	m_accuracy = event.data->accuracy;
}

void gravity_sensor::synthesize_fusion(const sensor_event_t& event)
{
	sensor_event_t *gravity_event;

	if (event.event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME) {
		fusion_set_accel(event);
		m_fusion_phase |= PHASE_ACCEL_READY;
	} else if (event.event_type == GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME) {
		fusion_set_gyro(event);
		m_fusion_phase |= PHASE_GYRO_READY;
	}

	if (m_fusion_phase != PHASE_FUSION_READY)
		return;

	m_fusion_phase = 0;

	fusion_update_angle();
	fusion_get_gravity();

	gravity_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
	if (!gravity_event) {
		_E("Failed to allocate memory");
		return;
	}
	gravity_event->data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	if (!gravity_event->data) {
		_E("Failed to allocate memory");
		free(gravity_event);
		return;
	}

	gravity_event->sensor_id = get_id();
	gravity_event->event_type = GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
	gravity_event->data_length = sizeof(sensor_data_t);
	gravity_event->data->accuracy = m_accuracy;
	gravity_event->data->timestamp = m_time;
	gravity_event->data->value_count = 3;
	gravity_event->data->values[0] = m_x;
	gravity_event->data->values[1] = m_y;
	gravity_event->data->values[2] = m_z;
	push(gravity_event);
}

void gravity_sensor::fusion_set_accel(const sensor_event_t& event)
{
	double x = event.data->values[0];
	double y = event.data->values[1];
	double z = event.data->values[2];

	m_accel_mag = NORM(x, y, z);

	m_angle_n[0] = ARCTAN(z, y);
	m_angle_n[1] = ARCTAN(x, z);
	m_angle_n[2] = ARCTAN(y, x);

	m_accuracy = event.data->accuracy;
	m_time_new = event.data->timestamp;

	_D("AccIn: (%f, %f, %f)", x/m_accel_mag, y/m_accel_mag, z/m_accel_mag);
}

void gravity_sensor::fusion_set_gyro(const sensor_event_t& event)
{
	m_velocity[0] = -DEG2RAD(event.data->values[0]);
	m_velocity[1] = -DEG2RAD(event.data->values[1]);
	m_velocity[2] = -DEG2RAD(event.data->values[2]);

	m_time_new = event.data->timestamp;
}

void gravity_sensor::fusion_update_angle(void)
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

void gravity_sensor::fusion_get_gravity(void)
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

void gravity_sensor::complementary(unsigned long long time_diff)
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

double gravity_sensor::complementary(double angle, double angle_in, double vel, double delta_t, double alpha)
{
	double s, c;
	angle = angle + vel * delta_t;
	s = alpha * sin(angle) + (1 - alpha) * sin(angle_in);
	c = alpha * cos(angle) + (1 - alpha) * cos(angle_in);
	return ARCTAN(s, c);
}

int gravity_sensor::get_data(sensor_data_t **data, int *length)
{
	/* if It is batch sensor, remains can be 2+ */
	int remains = 1;

	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));

	sensor_data->accuracy = m_accuracy;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 3;
	sensor_data->values[0] = m_x;
	sensor_data->values[1] = m_y;
	sensor_data->values[2] = m_z;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return --remains;
}

bool gravity_sensor::set_batch_latency(unsigned long latency)
{
	return false;
}

bool gravity_sensor::set_wakeup(int wakeup)
{
	return false;
}

bool gravity_sensor::on_start(void)
{
	if (m_fusion)
		m_fusion->start();

	if (m_accel_sensor)
		m_accel_sensor->start();

	if (m_gyro_sensor)
		m_gyro_sensor->start();

	m_time = 0;
	m_fusion_phase = 0;
	m_angle[0] = INV_ANGLE;

	return activate();
}

bool gravity_sensor::on_stop(void)
{
	if (m_fusion)
		m_fusion->stop();

	if (m_accel_sensor)
		m_accel_sensor->stop();

	if (m_gyro_sensor)
		m_gyro_sensor->stop();

	m_time = 0;

	return deactivate();
}

bool gravity_sensor::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	if (m_fusion)
		m_fusion->set_interval(FUSION_EVENT_AGM, client_id, interval);

	if (m_accel_sensor)
		m_accel_sensor->add_interval(client_id, interval, true);

	if (m_gyro_sensor)
		m_gyro_sensor->add_interval(client_id, interval, true);

	return sensor_base::add_interval(client_id, interval, is_processor);
}

bool gravity_sensor::delete_interval(int client_id, bool is_processor)
{
	if (m_fusion)
		m_fusion->unset_interval(FUSION_EVENT_AGM, client_id);

	if (m_accel_sensor)
		m_accel_sensor->delete_interval(client_id, true);

	if (m_gyro_sensor)
		m_gyro_sensor->delete_interval(client_id, true);

	return sensor_base::delete_interval(client_id, is_processor);
}

bool gravity_sensor::set_interval(unsigned long interval)
{
	m_interval = interval;
	return true;
}

bool gravity_sensor::rotation_to_gravity(const float *rotation, float *gravity)
{
	float R[9];

	if (quat_to_matrix(rotation, R) < 0)
		return false;

	gravity[0] = R[6] * GRAVITY;
	gravity[1] = R[7] * GRAVITY;
	gravity[2] = R[8] * GRAVITY;

	return true;
}

bool gravity_sensor::check_sampling_time(unsigned long long timestamp)
{
	const float MIN_DELIVERY_DIFF_FACTOR = 0.75f;
	const int MS_TO_US = 1000;
	long long diff_time;

	diff_time = timestamp - m_time;

	if (m_time && (diff_time < m_interval * MS_TO_US * MIN_DELIVERY_DIFF_FACTOR))
		return false;

	return true;
}
