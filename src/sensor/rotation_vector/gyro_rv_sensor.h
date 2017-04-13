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

#ifndef __GYRO_RV_SENSOR_H__
#define __GYRO_RV_SENSOR_H__

#include <fusion_sensor.h>
#include <sensor_types.h>
#include "gyro_fusion.h"

class gyro_rv_sensor : public fusion_sensor {
public:
	gyro_rv_sensor();
	virtual ~gyro_rv_sensor();

	int get_sensor_info(const sensor_info2_t **info);
	int get_required_sensors(const required_sensor_s **sensors);

	int update(uint32_t id, sensor_data_t *data, int len);
	int get_data(sensor_data_t **data, int *len);

private:
	gyro_fusion m_fusion;

	float m_x;
	float m_y;
	float m_z;
	float m_w;
	unsigned long long m_time;
	unsigned long m_interval;
	int m_accuracy;
};

#endif /* __GYRO_SENSOR_H__ */
