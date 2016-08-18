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

#ifdef _LINEAR_ACCELERATION_SENSOR_H_

sensor_data<float> linear_acceleration_sensor::get_linear_acceleration(const sensor_data<float> accel,
				const sensor_data<float> gyro, const sensor_data<float> magnetic)
{
	sensor_data<float> gravity_data;
	float la_x, la_y, la_z;

	gravity_data = grav_sensor.get_gravity(accel, gyro, magnetic);

	la_x = accel.m_data.m_vec[0] - gravity_data.m_data.m_vec[1];
	la_y = accel.m_data.m_vec[1] - gravity_data.m_data.m_vec[0];
	la_z = accel.m_data.m_vec[2] - gravity_data.m_data.m_vec[2];

	sensor_data<float> lin_accel_data(la_x, la_y, la_z);

	return lin_accel_data;
}

#endif
