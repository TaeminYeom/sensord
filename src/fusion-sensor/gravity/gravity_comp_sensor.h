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

#ifndef __GRAVITY_COMP_SENSOR_H__
#define __GRAVITY_COMP_SENSOR_H__

#include <fusion_sensor.h>
#include <sensor_types.h>

class gravity_comp_sensor : public fusion_sensor {
public:
	gravity_comp_sensor();
	virtual ~gravity_comp_sensor();

	int get_sensor_info(const sensor_info2_t **info);
	int get_required_sensors(const required_sensor_s **sensors);

	int update(uint32_t id, sensor_data_t *data, int len);
	int get_data(sensor_data_t **data, int *len);

private:
	int m_fusion_phase;

	float m_x;
	float m_y;
	float m_z;
	int m_accuracy;
	unsigned long long m_time;

	double m_angle[3];
	double m_angle_n[3];
	double m_accel_mag;
	double m_velocity[3];
	unsigned long long m_time_new;

	void fusion_set_accel(sensor_data_t *data);
	void fusion_set_gyro(sensor_data_t *data);
	void fusion_update_angle(void);
	void fusion_get_gravity(void);
	double complementary(double angle, double angle_in, double vel, double delta_t, double alpha);
	void complementary(unsigned long long time_diff);
};

#endif /* _GRAVITY_COMP_SENSOR_H_ */
