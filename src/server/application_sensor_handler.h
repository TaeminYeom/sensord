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

#ifndef __APPLICATION_SENSOR_HANDLER_H__
#define __APPLICATION_SENSOR_HANDLER_H__

#include <channel.h>
#include <sensor_types.h>
#include <unordered_map>
#include <atomic>

#include "sensor_handler.h"
#include "sensor_observer.h"

namespace sensor {

class application_sensor_handler : public sensor_handler {
public:
	application_sensor_handler(const sensor_info &info, ipc::channel *ch);
	~application_sensor_handler();

	/* TODO: const */
	int publish(sensor_data_t *data, int len);

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
	sensor_info m_info;
	ipc::channel *m_ch;
	std::atomic<bool> m_started;
	int32_t m_prev_interval;

	int get_min_interval(void);

	std::vector<sensor_handler *> m_required_sensors;
	std::unordered_map<sensor_observer *, int> m_interval_map;
};

}

#endif /* __APPLICATION_SENSOR_HANDLER_H__ */
