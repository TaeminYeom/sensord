/*
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

#ifndef __PEDOMETER_SENSOR_H__
#define __PEDOMETER_SENSOR_H__

#include <fusion_sensor.h>
#include <sensor_types.h>

#include "pedometer.h"

class pedometer_sensor : public fusion_sensor {
public:
	pedometer_sensor();
	virtual ~pedometer_sensor();

	int get_sensor_info(const sensor_info2_t **info);
	int get_required_sensors(const required_sensor_s **sensors);

	int update(uint32_t id, sensor_data_t *data, int len);
	int get_data(sensor_data_t **data, int *len);

	int start(observer_h ob);
	int stop(observer_h ob);

private:
	long long m_step_count;
	float m_step_length;
	float m_step_total_length;
	float m_step_speed;
	unsigned long long m_time;

	pedometer m_pedometer;
};

#endif /* __PEDOMETER_SENSOR_H__ */
