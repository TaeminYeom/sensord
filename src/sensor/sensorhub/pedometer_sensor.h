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

#ifndef _PEDOMETER_SENSOR_H_
#define _PEDOMETER_SENSOR_H_

#include <physical_sensor.h>

class pedometer_sensor : public physical_sensor {
public:
	pedometer_sensor();
	virtual ~pedometer_sensor();

	bool on_event(const sensor_data_t *data, int data_len, int remains);

private:
	unsigned long long m_steps;
	unsigned long long m_walk_steps;
	unsigned long long m_run_steps;
	unsigned long long m_walk_up_steps;
	unsigned long long m_walk_down_steps;
	unsigned long long m_run_up_steps;
	unsigned long long m_run_down_steps;
	double m_distance;
	double m_calories;

	void accumulate(sensor_pedometer_data_t *data);
};

#endif /* _PEDOMETER_SENSOR_H_ */

