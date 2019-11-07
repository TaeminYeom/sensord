/*
 * sensord
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#ifndef _TILT_SENSOR_H_
#define _TILT_SENSOR_H_

#include <sensor_internal.h>
#include <virtual_sensor.h>
#include <orientation_filter.h>

class tilt_sensor : public virtual_sensor {
public:
	tilt_sensor();
	virtual ~tilt_sensor();

	bool init(void);
	virtual void get_types(std::vector<sensor_type_t> &types);

	void synthesize(const sensor_event_t &event, std::vector<sensor_event_t> &outs);

	bool add_interval(int client_id, unsigned int interval);
	bool delete_interval(int client_id);

	int get_sensor_data(const unsigned int event_type, sensor_data_t &data);
	virtual bool get_properties(sensor_type_t sensor_type, sensor_properties_s &properties);
private:
	sensor_base *m_accel_sensor;
	sensor_base *m_fusion_sensor;

	sensor_data<float> m_accel;

	cmutex m_value_mutex;

	unsigned long long m_time;
	unsigned int m_interval;

	std::string m_vendor;
	std::string m_raw_data_unit;
	int m_default_sampling_time;
	int m_pitch_rotation_compensation;
	int m_roll_rotation_compensation;

	bool on_start(void);
	bool on_stop(void);
};

#endif /*_TILT_SENSOR_H_*/
