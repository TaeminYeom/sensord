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

#ifndef __FUSION_SENSOR_HANDLER_H__
#define __FUSION_SENSOR_HANDLER_H__

#include <message.h>
#include <sensor_types.h>
#include <fusion_sensor.h>
#include <unordered_map>

#include "sensor_handler.h"
#include "sensor_observer.h"

namespace sensor {

class fusion_sensor_handler : public sensor_handler, public sensor_observer {
public:
	fusion_sensor_handler(const sensor_info &info,
			fusion_sensor *sensor);
	~fusion_sensor_handler();

	void add_required_sensor(sensor_handler *sensor);

	/* subscriber */
	int update(const char *uri, ipc::message *msg);

	/* sensor interface */
	const sensor_info &get_sensor_info(void);

	int start(sensor_observer *ob);
	int stop(sensor_observer *ob);

	int set_interval(sensor_observer *ob, int32_t interval);
	int set_batch_latency(sensor_observer *ob, int32_t latency);
	int set_attribute(sensor_observer *ob, int32_t attr, int32_t value);
	int set_attribute(sensor_observer *ob, int32_t attr, const char *value, int len);
	int flush(sensor_observer *ob);
	int get_data(sensor_data_t **data, int *len);

private:
	int start_internal(void);
	int stop_internal(void);
	int set_interval_internal(int32_t interval);
	int set_batch_latency_internal(int32_t latency);
	int set_attribute_internal(int32_t attr, int32_t value);
	int set_attribute_internal(int32_t attr, const char *value, int len);

	int get_min_interval(void);
	int get_min_batch_latency(void);

	sensor_info m_info;
	fusion_sensor *m_sensor;
	std::vector<sensor_handler *> m_required_sensors;

	std::unordered_map<sensor_observer *, int> m_interval_map;
	std::unordered_map<sensor_observer *, int> m_batch_latency_map;
	int m_policy;
};

}

#endif /* __FUSION_SENSOR_HANDLER_H__ */
