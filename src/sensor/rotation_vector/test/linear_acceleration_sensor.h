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

#ifndef _LINEAR_ACCELERATION_SENSOR_H_
#define _LINEAR_ACCELERATION_SENSOR_H_

#include "gravity_sensor.h"

class linear_acceleration_sensor
{
public:
	gravity_sensor grav_sensor;

	sensor_data<float> get_linear_acceleration(const sensor_data<float> accel,
				const sensor_data<float> gyro, const sensor_data<float> magnetic);
};

#include "linear_acceleration_sensor.cpp"

#endif /* _LINEAR_ACCELERATION_SENSOR_H_ */
