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

#ifndef __MAGNETIC_RV_SENSOR_H__
#define __MAGNETIC_RV_SENSOR_H__

#include <virtual_sensor.h>
#include <sensor_types.h>
#include <magnetic_fusion.h>

class magnetic_rv_sensor : public virtual_sensor {
public:
	magnetic_rv_sensor();
	virtual ~magnetic_rv_sensor();

	/* initialize sensor */
	bool init(void);

	/* sensor info */
	virtual sensor_type_t get_type(void);
	virtual unsigned int get_event_type(void);
	virtual const char* get_name(void);

	virtual bool get_sensor_info(sensor_info &info);

	/* synthesize event */
	virtual void synthesize(const sensor_event_t& event);

	bool add_interval(int client_id, unsigned int interval, bool is_processor);
	bool delete_interval(int client_id, bool is_processor);

	/* get data */
	virtual int get_data(sensor_data_t **data, int *length);
private:
	sensor_base *m_accel_sensor;
	sensor_base *m_mag_sensor;
	magnetic_fusion m_fusion;

	float m_x;
	float m_y;
	float m_z;
	float m_w;
	unsigned long long m_time;
	unsigned long m_interval;
	int m_accuracy;

	virtual bool set_interval(unsigned long interval);
	virtual bool set_batch_latency(unsigned long latency);

	virtual bool on_start(void);
	virtual bool on_stop(void);
};

#endif /* __MAGNETIC_SENSOR_H__ */
