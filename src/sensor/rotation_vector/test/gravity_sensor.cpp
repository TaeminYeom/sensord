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

#ifdef _GRAVITY_SENSOR_H_

#define GRAVITY		9.80665

sensor_data<float> gravity_sensor::get_gravity(const sensor_data<float> accel,
				const sensor_data<float> gyro, const sensor_data<float> magnetic)
{
	euler_angles<float> orientation;
	sensor_data<float> gravity;

	orientation = orien_sensor.get_orientation(accel, gyro, magnetic);

	gravity.m_data.m_vec[0] = GRAVITY * sin(orientation.m_ang.m_vec[0]);
	gravity.m_data.m_vec[1] = GRAVITY * sin(orientation.m_ang.m_vec[1]);
	gravity.m_data.m_vec[2] = GRAVITY * cos(orientation.m_ang.m_vec[0]) *
									cos(orientation.m_ang.m_vec[1]);

	return gravity;
}

#endif
