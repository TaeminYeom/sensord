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

#ifndef __FACE_DOWN_SENSOR_H__
#define __FACE_DOWN_SENSOR_H__

#include <virtual_sensor.h>
#include <sensor_types.h>
#include <face_down_alg_impl.h>

class face_down_sensor : public virtual_sensor {
public:
	face_down_sensor();
	~face_down_sensor();

	/* initialize sensor */
	bool init(void);

	/* sensor info */
	sensor_type_t get_type(void);
	unsigned int get_event_type(void);
	const char *get_name(void);

	bool get_sensor_info(sensor_info & info);

	/* synthesize event */
	void synthesize(const sensor_event_t & event);

	bool add_interval(int client_id, unsigned int interval, bool is_processor);
	bool delete_interval(int client_id, bool is_processor);

	/* get data */
	int get_data(sensor_data_t ** data, int *length);
private:
	sensor_base * m_gravity_sensor;
	face_down_alg_impl *m_alg;

	unsigned long long m_time;
	bool m_state;
	unsigned int m_interval;

	bool set_interval(unsigned long interval);
	bool set_batch_latency(unsigned long latency);

	bool on_start(void);
	bool on_stop(void);
	face_down_alg_impl *get_alg(void);
};

#endif /* __FACE_DOWN_SENSOR_H__ */
