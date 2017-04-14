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

#ifndef __SENSOR_LISTENER_PROXY_H__
#define __SENSOR_LISTENER_PROXY_H__

#include <channel.h>
#include <message.h>

#include "sensor_handler.h"
#include "sensor_observer.h"

namespace sensor {

class sensor_listener_proxy : public sensor_observer {
public:
	sensor_listener_proxy(uint32_t id, sensor_handler *sensor, ipc::channel *ch);
	~sensor_listener_proxy();

	uint32_t get_id(void);

	/* sensor observer */
	int update(const char *uri, ipc::message *msg);

	int start(void);
	int stop(void);

	int set_interval(unsigned int interval);
	int set_max_batch_latency(unsigned int max_batch_latency);
	int set_passive_mode(bool passive);
	int set_attribute(int attribute, int value);
	int set_attribute(int attribute, const char *value, int len);
	int flush(void);
	int get_data(sensor_data_t **data, int *len);
	std::string get_required_privileges(void);

private:
	uint32_t m_id;

	sensor_handler *m_sensor;
	ipc::channel *m_ch;

	bool m_passive;
	int m_pause_policy;
	int m_axis_orientation;
};

}

#endif /* __SENSOR_LISTENER_PROXY_H__ */
